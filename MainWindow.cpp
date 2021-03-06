//
// Copyright (C) 2015 Red Hat, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Authors: Daniel Kopecek <dkopecek@redhat.com>

#include "MainWindow.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-default"
#include <ui_MainWindow.h>
#include "DeviceDialog.h"
#include "DBusBridge.h"

#include "usbguard/Logger.hpp"

#include <QString>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QMessageBox>
#include <QApplication>
#include <QResource>
#include <QMenu>
#include <QAction>
#include <QDateTime>
#include <QTime>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QTreeView>
#include <QShortcut>
#include <QWindowStateChangeEvent>
#pragma GCC diagnostic pop

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  _settings("USBGuard", "usbguard-applet-qt"),
  _device_model(this),
  _bridge(this)
{
  /*
   * Seed the pseudo-random generator. We use it for
   * randomizing the position of the DeviceDialog window.
   */
  QTime time_rnd_seed = QTime::currentTime();
  qsrand((uint)time_rnd_seed.msec());
  ui->setupUi(this);
  ui->device_view->setModel(&_device_model);
  ui->device_view->setItemDelegateForColumn(2, &_target_delegate);
  ui->device_view->resizeColumnToContents(1);
  ui->device_view->setItemsExpandable(false);
  QObject::connect(ui->device_view->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
    this, SLOT(editDeviceListRow(QModelIndex)));
  QObject::connect(ui->device_view, SIGNAL(clicked(QModelIndex)),
    this, SLOT(editDeviceListRow(QModelIndex)));
  QObject::connect(ui->apply_button, SIGNAL(pressed()),
    this, SLOT(commitDeviceListChanges()));
  QObject::connect(ui->reset_button, SIGNAL(pressed()),
    this, SLOT(resetDeviceList()));
  setWindowTitle("USBGuard");
  setWindowIcon(QIcon(":/usbguard-icon.svg"));
  setWindowState(Qt::WindowMinimized);
  setupSystemTray();
  qRegisterMetaType<usbguard::DeviceManager::EventType>("usbguard::DeviceManager::EventType");
  qRegisterMetaType<usbguard::Rule::Target>("usbguard::Rule::Target");
  QObject::connect(&_bridge, SIGNAL(devicePresenceChanged(uint, usbguard::DeviceManager::EventType, usbguard::Rule::Target, const QString &)),
    this, SLOT(handleDevicePresenceChange(uint, usbguard::DeviceManager::EventType, usbguard::Rule::Target, const QString&)));
  QObject::connect(&_bridge, SIGNAL(devicePolicyChanged(uint, usbguard::Rule::Target, usbguard::Rule::Target, const QString&, uint)),
    this, SLOT(handleDevicePolicyChange(uint, usbguard::Rule::Target, usbguard::Rule::Target, const QString&, uint)));
  QObject::connect(&_bridge, SIGNAL(serviceAvailable()),
    this, SLOT(handleDBusConnect()));
  QObject::connect(&_bridge, SIGNAL(serviceUnavailable()),
    this, SLOT(handleDBusDisconnect()));
  /*
   * loadSettings has to be called before setupSettingsWatcher! Otherwise it
   * will trigger the slots connected by the setupSettingsWatcher method.
   */
  loadSettings();
  setupSettingsWatcher();
  ui->statusBar->showMessage(tr("Inactive. No D-Bus connection."));
  new QShortcut(QKeySequence(Qt::Key_Escape, Qt::Key_Escape), this, SLOT(showMinimized()));

  QTimer::singleShot(1000, this, SLOT(dbusTryConnect()));
}

void MainWindow::setupSystemTray()
{
  systray = new QSystemTrayIcon(QIcon(":/usbguard-icon-inactive.svg"), this);
  systray->setToolTip("USBGuard");
  auto menu = new QMenu();
  auto quit_action = new QAction(tr("Quit"), systray);
  menu->addAction(quit_action);
  systray->setContextMenu(menu);
  QObject::connect(quit_action, SIGNAL(triggered()), qApp, SLOT(quit()));
  QObject::connect(systray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
    this, SLOT(switchVisibilityState(QSystemTrayIcon::ActivationReason)));
  QObject::connect(&_flash_timer, SIGNAL(timeout()),
    this, SLOT(flashStep()));
  systray->show();
}

void MainWindow::setupSettingsWatcher()
{
  for (QCheckBox* checkbox : ui->settings_tab->findChildren<QCheckBox*>()) {
    QObject::connect(checkbox, SIGNAL(toggled(bool)), this, SLOT(saveSettings()));
  }

  for (QComboBox* combobox : ui->settings_tab->findChildren<QComboBox*>()) {
    QObject::connect(combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(saveSettings()));
  }

  for (QSpinBox* spinbox : ui->settings_tab->findChildren<QSpinBox*>()) {
    QObject::connect(spinbox, SIGNAL(valueChanged(int)), this, SLOT(saveSettings()));
  }
}

void MainWindow::switchVisibilityState(QSystemTrayIcon::ActivationReason reason)
{
  USBGUARD_LOG(Trace) << "reason=" << reason;

  if (reason == QSystemTrayIcon::Context) {
    systray->contextMenu()->show();
  }
  else {
    if (!isVisible() || (windowState() & Qt::WindowMinimized)) {
      USBGUARD_LOG(Trace) << "Showing main window";
      showNormal();
      stopFlashing();
    }
    else {
      USBGUARD_LOG(Trace) << "Minimizing main window";
      showMinimized();
    }
  }
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::showDeviceDialog(quint32 id, const usbguard::Rule& device_rule)
{
  auto dialog = new DeviceDialog(id);
  dialog->setRejectVisible(ui->show_reject_button_checkbox->isChecked());
  dialog->setDefaultDecisionTimeout(ui->decision_timeout->value());
  dialog->setMaskSerialNumber(ui->mask_serial_checkbox->isChecked());
  dialog->setDecisionIsPermanent(ui->decision_permanent_checkbox->isChecked());
  usbguard::Rule::Target default_target = usbguard::Rule::Target::Block;

  switch (ui->default_decision_combobox->currentIndex()) {
  case 0:
    default_target = usbguard::Rule::Target::Allow;
    break;

  case 1:
    default_target = usbguard::Rule::Target::Block;
    break;

  case 2:
    default_target = usbguard::Rule::Target::Reject;
    break;

  default:
    default_target = usbguard::Rule::Target::Block;
  }

  dialog->setDefaultDecision(default_target);
  dialog->setName(QString::fromStdString(device_rule.getName()));
  dialog->setSerial(QString::fromStdString(device_rule.getSerial()));
  dialog->setDeviceID(QString::fromStdString(device_rule.getDeviceID().getVendorID()),
    QString::fromStdString(device_rule.getDeviceID().getProductID()));
  dialog->setInterfaceTypes(device_rule.attributeWithInterface().values());
  dialog->setModal(false);
  dialog->setRandomizePosition(ui->randomize_position_checkbox->isChecked());
  QObject::connect(dialog, SIGNAL(allowed(quint32, bool)),
    this, SLOT(allowDevice(quint32, bool)));
  QObject::connect(dialog, SIGNAL(rejected(quint32, bool)),
    this, SLOT(rejectDevice(quint32, bool)));
  QObject::connect(dialog, SIGNAL(blocked(quint32, bool)),
    this, SLOT(blockDevice(quint32, bool)));
  dialog->show();
  dialog->raise();
  dialog->activateWindow();
}

void MainWindow::showMessage(const QString& message, bool alert, bool statusbar)
{
  const QString mtemplate(alert ? "[%1] <b>%2</b>" : "[%1] %2");
  const QString datetime = QDateTime::currentDateTime().toString();
  const QString mmessage = QString(mtemplate).arg(datetime).arg(message);
  ui->messages_text->append(mmessage);

  if (statusbar) {
    const QString stemplate("[%1] %2");
    const QString smessage = QString(stemplate).arg(datetime).arg(message);
    ui->statusBar->showMessage(smessage);
  }
}

void MainWindow::handleDevicePresenceChange(uint id,
  usbguard::DeviceManager::EventType event,
  usbguard::Rule::Target target,
  const QString& device_rule_string)
{
  (void)target;
  auto device_rule = usbguard::Rule::fromString(device_rule_string.toStdString());

  switch (event) {
  case usbguard::DeviceManager::EventType::Insert:
    handleDeviceInsert(id, device_rule);
    break;

  case usbguard::DeviceManager::EventType::Remove:
    handleDeviceRemove(id, device_rule);
    break;

  case usbguard::DeviceManager::EventType::Present:
  case usbguard::DeviceManager::EventType::Update:
  default:
    /* NOOP */
    break;
  }

  notifyDevicePresenceChanged(event, device_rule);
}

void MainWindow::handleDevicePolicyChange(uint id,
  usbguard::Rule::Target target_old,
  usbguard::Rule::Target target_new,
  const QString& device_rule_string,
  uint rule_id)
{
  (void)target_old;
  auto device_rule = usbguard::Rule::fromString(device_rule_string.toStdString());
  _device_model.updateDeviceTarget(id, target_new);
  ui->device_view->expandAll();
  notifyDevicePolicyChanged(device_rule, rule_id);

  if (target_new == usbguard::Rule::Target::Block &&
    rule_id == usbguard::Rule::ImplicitID) {
    showDeviceDialog(id, device_rule);
  }
}

void MainWindow::notifyDevicePresenceChanged(usbguard::DeviceManager::EventType event,
  const usbguard::Rule& device_rule)
{
  QString title;
  bool show_notification = true;
  QSystemTrayIcon::MessageIcon notification_icon = \
    QSystemTrayIcon::Information;

  switch (event) {
  case usbguard::DeviceManager::EventType::Insert:
    title = tr("USB Device Inserted");
    show_notification = ui->notify_inserted->isChecked();
    break;

  case usbguard::DeviceManager::EventType::Update:
    title = tr("USB Device Updated");
    break;

  case usbguard::DeviceManager::EventType::Remove:
    title = tr("USB Device Removed");
    show_notification = ui->notify_removed->isChecked();
    break;

  case usbguard::DeviceManager::EventType::Present:
    title = tr("USB Device Present");
    show_notification = ui->notify_present->isChecked();
    break;

  default:
    /* NOOP */
    return;
  }

  notify(title, notification_icon, device_rule, show_notification);
}

void MainWindow::notifyDevicePolicyChanged(const usbguard::Rule& device_rule, quint32 rule_id)
{
  (void)rule_id;
  QString title;
  bool show_notification = true;
  QSystemTrayIcon::MessageIcon notification_icon = \
    QSystemTrayIcon::Information;

  switch (device_rule.getTarget()) {
  case usbguard::Rule::Target::Allow:
    title = tr("USB Device Allowed");
    show_notification = ui->notify_allowed->isChecked();
    break;

  case usbguard::Rule::Target::Block:
    title = tr("USB Device Blocked");
    show_notification = ui->notify_blocked->isChecked();
    notification_icon = QSystemTrayIcon::Warning;
    break;

  case usbguard::Rule::Target::Reject:
    title = tr("USB Device Rejected");
    show_notification = ui->notify_rejected->isChecked();
    notification_icon = QSystemTrayIcon::Warning;

    if (windowState() & Qt::WindowMinimized) {
      startFlashing();
    }

    break;

  case usbguard::Rule::Target::Invalid:
  case usbguard::Rule::Target::Empty:
  case usbguard::Rule::Target::Match:
  case usbguard::Rule::Target::Device:
  case usbguard::Rule::Target::Unknown:
  default:
    /* NOOP */
    return;
  }

  notify(title, notification_icon, device_rule, show_notification);
}

void MainWindow::notify(const QString& title, QSystemTrayIcon::MessageIcon icon, const usbguard::Rule& device_rule,
  bool show_notification)
{
  const QString usb_id = QString::fromStdString(device_rule.getDeviceID().toString());
  const QString name = QString::fromStdString(device_rule.getName());
  const QString port = QString::fromStdString(device_rule.getViaPort());
  const QString message_body = QString("%1: USB ID=%2; Name=%3; Port=%4")
    .arg(title).arg(usb_id).arg(name).arg(port);
  showMessage(message_body);

  if (show_notification) {
    const QString notification_body = \
      QString("USB ID: %1\n"
        "Name: %2\n"
        "Port: %3\n")
      .arg(usb_id).arg(name).arg(port);
    showNotification(icon, title, notification_body);
  }
}

void MainWindow::showNotification(QSystemTrayIcon::MessageIcon icon, const QString& title, const QString& message)
{
  systray->showMessage(title, message, icon);
}

void MainWindow::notifyDBusConnected()
{
  const QString title = tr("D-Bus Connection Established");

  if (ui->notify_dbus->isChecked()) {
    showNotification(QSystemTrayIcon::Information, title, "");
  }

  showMessage(title, /*alert=*/false, /*statusbar=*/true);
}

void MainWindow::notifyDBusDisconnected()
{
  const QString title = tr("D-Bus Connection Lost");

  if (ui->notify_dbus->isChecked()) {
    showNotification(QSystemTrayIcon::Warning, title, "");
  }

  showMessage(title, /*alert=*/true, /*statusbar=*/true);
}

void MainWindow::startFlashing()
{
  _flash_state = false;
  _flash_timer.setInterval(500);
  _flash_timer.start();
}

void MainWindow::stopFlashing()
{
  _flash_state = false;
  _flash_timer.stop();

  if (_bridge.isConnected()) {
    systray->setIcon(QIcon(":/usbguard-icon.svg"));
  }
  else {
    systray->setIcon(QIcon(":/usbguard-icon-inactive.svg"));
  }
}

void MainWindow::flashStep()
{
  if (_flash_state) {
    systray->setIcon(QIcon(":/usbguard-icon-warning.svg"));
    systray->show();
    _flash_timer.setInterval(250);
    _flash_state = false;
  }
  else {
    if (_bridge.isConnected()) {
      systray->setIcon(QIcon(":/usbguard-icon.svg"));
    }
    else {
      systray->setIcon(QIcon(":/usbguard-icon-inactive.svg"));
    }

    systray->show();
    _flash_timer.setInterval(500);
    _flash_state = true;
  }
}

void MainWindow::dbusTryConnect()
{
  USBGUARD_LOG(Trace);

  QDBusReply<bool> reply = _bridge.tryConnect();
  if (!reply.isValid()) {
    showMessage(QString("Connection failed: %1")
      .arg(reply.error().message()),
      /*alert=*/true);
  }
  else if (!reply.value()) {
    showMessage(QString("Connection failed: D-Bus service not available"),
      /*alert=*/true);
  }
}

void MainWindow::allowDevice(quint32 id, bool permanent)
{
  USBGUARD_LOG(Trace) << "id=" << id << " permanent=" << permanent;

  QDBusPendingReply<uint> reply = _bridge.applyDevicePolicy(id, usbguard::Rule::Target::Allow, permanent);
  if (!reply.isValid()) {
    showMessage(QString("D-Bus call failed: %1: %2")
      .arg("allowDevice")
      .arg(reply.error().message()),
      /*alert=*/true);
  }
}

void MainWindow::blockDevice(quint32 id, bool permanent)
{
  USBGUARD_LOG(Trace) << "id=" << id << " permanent=" << permanent;

  QDBusPendingReply<uint> reply = _bridge.applyDevicePolicy(id, usbguard::Rule::Target::Block, permanent);
  if (!reply.isValid()) {
    showMessage(QString("D-Bus call failed: %1: %2")
      .arg("blockDevice")
      .arg(reply.error().message()),
      /*alert=*/true);
  }
}

void MainWindow::rejectDevice(quint32 id, bool permanent)
{
  USBGUARD_LOG(Trace) << "id=" << id << " permanent=" << permanent;

  QDBusPendingReply<uint> reply = _bridge.applyDevicePolicy(id, usbguard::Rule::Target::Reject, permanent);
  if (!reply.isValid()) {
    showMessage(QString("D-Bus call failed: %1: %2")
      .arg("rejectDevice")
      .arg(reply.error().message()),
      /*alert=*/true);
  }
}

void MainWindow::handleDBusConnect()
{
  USBGUARD_LOG(Trace);
  notifyDBusConnected();
  systray->setIcon(QIcon(":/usbguard-icon.svg"));
  ui->device_view->setDisabled(false);
  loadDeviceList();
}

void MainWindow::handleDBusDisconnect()
{
  USBGUARD_LOG(Trace);
  notifyDBusDisconnected();
  systray->setIcon(QIcon(":/usbguard-icon-inactive.svg"));
  clearDeviceList();
  ui->device_view->setDisabled(true);
}

void MainWindow::handleDeviceInsert(quint32 id, const usbguard::Rule& device_rule)
{
  USBGUARD_LOG(Trace) << "id=" << id << " device_rule=" << device_rule.toString();
  loadDeviceList();
}

void MainWindow::handleDeviceRemove(quint32 id, const usbguard::Rule& device_rule)
{
  USBGUARD_LOG(Trace) << "id=" << id << " device_rule=" << device_rule.toString();
  ui->device_view->selectionModel()->clearSelection();
  ui->device_view->reset();
  _device_model.removeDevice(id);
  ui->device_view->expandAll();
}

void MainWindow::loadSettings()
{
  USBGUARD_LOG(Trace);
  _settings.sync();
  _settings.beginGroup("Notifications");
  ui->notify_inserted->setChecked(_settings.value("Inserted", true).toBool());
  ui->notify_removed->setChecked(_settings.value("Removed", false).toBool());
  ui->notify_allowed->setChecked(_settings.value("Allowed", true).toBool());
  ui->notify_blocked->setChecked(_settings.value("Blocked", true).toBool());
  ui->notify_rejected->setChecked(_settings.value("Rejected", true).toBool());
  ui->notify_present->setChecked(_settings.value("Present", false).toBool());
  // Left as IPCStatus for compatibility.
  ui->notify_dbus->setChecked(_settings.value("IPCStatus", false).toBool());
  _settings.endGroup();
  _settings.beginGroup("DeviceDialog");
  const int default_decision_index = _settings.value("DefaultDecision", 1).toInt();

  if (default_decision_index >= 0 && default_decision_index < ui->default_decision_combobox->count()) {
    ui->default_decision_combobox->setCurrentIndex(default_decision_index);
  }

  const int decision_method_index = _settings.value("DecisionMethod", 0).toInt();

  if (decision_method_index >= 0 && decision_method_index < ui->decision_method_combobox->count()) {
    ui->decision_method_combobox->setCurrentIndex(decision_method_index);
  }

  ui->decision_timeout->setValue(_settings.value("DefaultDecisionTimeout", 23).toInt());
  ui->decision_permanent_checkbox->setChecked(_settings.value("DecisionIsPermanent", false).toBool());
  ui->show_reject_button_checkbox->setChecked(_settings.value("ShowRejectButton", false).toBool());
  ui->randomize_position_checkbox->setChecked(_settings.value("RandomizeWindowPosition", true).toBool());
  ui->mask_serial_checkbox->setChecked(_settings.value("MaskSerialNumber", true).toBool());
  _settings.endGroup();
}

void MainWindow::saveSettings()
{
  USBGUARD_LOG(Trace);
  _settings.clear();
  _settings.beginGroup("Notifications");
  _settings.setValue("Inserted", ui->notify_inserted->isChecked());
  _settings.setValue("Removed", ui->notify_removed->isChecked());
  _settings.setValue("Allowed", ui->notify_allowed->isChecked());
  _settings.setValue("Blocked", ui->notify_blocked->isChecked());
  _settings.setValue("Rejected", ui->notify_rejected->isChecked());
  _settings.setValue("Present", ui->notify_present->isChecked());
  // Left as IPCStatus for compatibility.
  _settings.setValue("IPCStatus", ui->notify_dbus->isChecked());
  _settings.endGroup();
  _settings.beginGroup("DeviceDialog");
  _settings.setValue("DefaultDecision", ui->default_decision_combobox->currentIndex());
  _settings.setValue("DefaultDecisionTimeout", ui->decision_timeout->value());
  _settings.setValue("DecisionMethod", ui->decision_method_combobox->currentIndex());
  _settings.setValue("DecisionIsPermanent", ui->decision_permanent_checkbox->isChecked());
  _settings.setValue("ShowRejectButton", ui->show_reject_button_checkbox->isChecked());
  _settings.setValue("RandomizeWindowPosition", ui->randomize_position_checkbox->isChecked());
  _settings.setValue("MaskSerialNumber", ui->mask_serial_checkbox->isChecked());
  _settings.endGroup();
  _settings.sync();
}

void MainWindow::loadDeviceList()
{
  USBGUARD_LOG(Trace);

  QDBusPendingReply<DBusRules> reply = _bridge.listDevices("match");
  if (!reply.isValid()) {
    showMessage(QString("D-Bus call failed: %1: %2")
      .arg("listDevices")
      .arg(reply.error().message()),
      /*alert=*/true);
    return;
  }

  const DBusRules rules = reply.value();
  for (auto rule : rules) {
    auto device_rule = usbguard::Rule::fromString(rule.second.toStdString());
    device_rule.setRuleID(rule.first);
    if (!_device_model.containsDevice(device_rule.getRuleID())) {
      _device_model.insertDevice(device_rule);
    }
  }

  ui->device_view->expandAll();
}

void MainWindow::editDeviceListRow(const QModelIndex& index)
{
  ui->device_view->edit(_device_model.createRowEditIndex(index));
}

void MainWindow::commitDeviceListChanges()
{
  auto modified_map = _device_model.getModifiedDevices();
  auto modified_it = modified_map.begin();
  const bool permanent = ui->permanent_checkbox->isChecked();

  while (modified_it != modified_map.end()) {
    auto id = modified_it.key();
    auto target = modified_it.value();

    switch (target) {
    case usbguard::Rule::Target::Allow:
      allowDevice(id, permanent);
      break;

    case usbguard::Rule::Target::Block:
      blockDevice(id, permanent);
      break;

    case usbguard::Rule::Target::Reject:
      rejectDevice(id, permanent);
      break;

    case usbguard::Rule::Target::Match:
    case usbguard::Rule::Target::Invalid:
    case usbguard::Rule::Target::Empty:
    case usbguard::Rule::Target::Unknown:
    case usbguard::Rule::Target::Device:
    default:
      break;
    }

    ++modified_it;
  }
}

void MainWindow::clearDeviceList()
{
  ui->device_view->clearSelection();
  ui->device_view->reset();
  _device_model.clear();
}

void MainWindow::resetDeviceList()
{
  clearDeviceList();
  loadDeviceList();
  ui->device_view->expandAll();
}

void MainWindow::changeEvent(QEvent* e)
{
  USBGUARD_LOG(Trace) << "e->type=" << e->type();

  if (e->type() == QEvent::LanguageChange) {
    USBGUARD_LOG(Trace) << "QEvent::LanguageChange";
    ui->retranslateUi(this);
  }
  else if (e->type() == QEvent::WindowStateChange) {
    USBGUARD_LOG(Trace) << "QEvent::WindowStateChange";
    QWindowStateChangeEvent* event = \
      static_cast<QWindowStateChangeEvent*>(e);

    if (!(event->oldState() & Qt::WindowMinimized)
      && (windowState() & Qt::WindowMinimized)) {
      USBGUARD_LOG(Trace) << "Qt::WindowMinimized";
      QTimer::singleShot(250, this, SLOT(hide()));
    }
  }

  QMainWindow::changeEvent(e);
}

void MainWindow::closeEvent(QCloseEvent* e)
{
  USBGUARD_LOG(Trace) << "e=" << e;
  showMinimized();
  e->accept();
}

/* vim: set ts=2 sw=2 et */

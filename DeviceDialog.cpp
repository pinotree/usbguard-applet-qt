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
//

#include "DeviceDialog.h"
#include "Log.h"
#include <ui_DeviceDialog.h>

#include <QRandomGenerator>
#include <QScreen>
#include <QStyle>

DeviceDialog::DeviceDialog(quint32 id, QWidget* parent) :
  QDialog(parent),
  ui(new Ui::DeviceDialog),
  device_id(id)
{
  qCDebug(LOG) << "Creating DeviceDialog for device_id=" << id;
  ui->setupUi(this);
  setWindowTitle(tr("USB Device Inserted"));
  setWindowIcon(QIcon(QLatin1String(":/usbguard-icon.svg")));
  setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowStaysOnTopHint);
  connect(&timer, &QTimer::timeout, this, &DeviceDialog::timerUpdate);
  setDecisionMethod(DecisionMethod::Buttons);
  setDefaultDecisionTimeout(23);
  setRandomizePosition(false);
  setDefaultDecision(Rule::Target::Block);
  updateDialog();
  timer.start(1000);
}

void DeviceDialog::setName(const QString& name)
{
  ui->name_label->setText(name);
}

void DeviceDialog::setDeviceID(const QString& vendor_id, const QString& product_id)
{
  ui->deviceid_label->setText(QString::fromLatin1("%1:%2").arg(vendor_id).arg(product_id));
}

void DeviceDialog::setSerial(const QString& serial)
{
  qCDebug(LOG) << "Masking serial number value";
  _serial = serial;

  if (_mask_serial_number) {
    for (int i = _serial.size(), p = 1; i > 0; --i, ++p) {
      if ((p % 2) == 0) {
        _serial[i - 1] = QLatin1Char('*');
      }
    }
  }

  ui->serial_label->setText(_serial);
}

void DeviceDialog::setInterfaceTypes(const std::vector<usbguard::USBInterfaceType>& interfaces)
{
  ui->interface_list->clear();

  for (auto const& type : interfaces) {
    ui->interface_list->addItem(QString::fromStdString(type.typeString()));
  }
}

void DeviceDialog::setDefaultDecision(Rule::Target target)
{
  switch (target) {
  case Rule::Target::Allow:
    ui->allow_button->setFocus();
    break;

  case Rule::Target::Block:
    ui->block_button->setFocus();
    break;

  case Rule::Target::Reject:
  case Rule::Target::Unknown:
  case Rule::Target::Empty:
  case Rule::Target::Invalid:
  case Rule::Target::Match:
  case Rule::Target::Device:
  default:
    ui->reject_button->setFocus();
  }

  _default_decision = target;
}

void DeviceDialog::setDefaultDecisionTimeout(quint32 seconds)
{
  _default_decision_timeout = seconds;
  time_left = seconds;
}

void DeviceDialog::setDecisionMethod(DeviceDialog::DecisionMethod method)
{
  _decision_method = method;
}

void DeviceDialog::setDecisionIsPermanent(bool state)
{
  _decision_is_permanent = state;
  ui->permanent_checkbox->setChecked(state);
}

void DeviceDialog::setRejectVisible(bool state)
{
  ui->reject_button->setHidden(!state);
}

static int randomInteger(int a, int b)
{
  int min_val;
  int max_val;
  if (a > b) {
    min_val = b;
    max_val = a;
  } else  {
    min_val = a;
    max_val = b;
  }
  return QRandomGenerator::global()->bounded(min_val, max_val);
}

void DeviceDialog::setRandomizePosition(bool randomize)
{
  QRect position_rect = \
    QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
      size(), qGuiApp->primaryScreen()->availableGeometry());

  if (randomize) {
    const int h = ui->block_button->height();
    const int w = ui->block_button->width();
    const int dy = randomInteger(-2.618*h, 2.618*h);
    const int dx = randomInteger(-2.618*w, 2.618*w);
    position_rect.translate(dx, dy);
  }

  setGeometry(position_rect);
}

void DeviceDialog::setMaskSerialNumber(bool state)
{
  _mask_serial_number = state;
}

void DeviceDialog::timerUpdate()
{
  if (time_left > 0) {
    --time_left;
    updateDialog();
  }
  else {
    timer.stop();
    executeDefaultDecision();
  }
}

void DeviceDialog::reject()
{
  if (timer.isActive()) {
    timer.stop();
    updateDialog();
  }
  else {
    QDialog::reject();
  }
}

void DeviceDialog::accept()
{
  if (timer.isActive()) {
    timer.stop();
  }

  QDialog::accept();
}

void DeviceDialog::updateDialog()
{
  QPushButton* button = nullptr;
  QString label;

  switch (_default_decision) {
  case Rule::Target::Allow:
    button = ui->allow_button;
    label = tr("Allow");
    break;

  case Rule::Target::Block:
    button = ui->block_button;
    label = tr("Block");
    break;

  case Rule::Target::Reject:
  case Rule::Target::Match:
  case Rule::Target::Device:
  case Rule::Target::Invalid:
  case Rule::Target::Empty:
  case Rule::Target::Unknown:
  default:
    button = ui->reject_button;
    label = tr("Reject");
  }

  if (timer.isActive()) {
    button->setText(QString::fromLatin1("%1 [%2]").arg(label).arg(time_left));
  }
  else {
    button->setText(label);
    ui->hint_label->setText(tr("(Press Escape to close this window)"));
  }
}

void DeviceDialog::executeDefaultDecision()
{
  qCDebug(LOG) << "Executing default decision: "
    << _default_decision;

  switch (_default_decision) {
  case Rule::Target::Allow:
    on_allow_button_clicked();
    break;

  case Rule::Target::Block:
    on_block_button_clicked();
    break;

  case Rule::Target::Reject:
  case Rule::Target::Unknown:
  case Rule::Target::Empty:
  case Rule::Target::Invalid:
  case Rule::Target::Device:
  case Rule::Target::Match:
  default:
    on_allow_button_clicked();
  }
}

DeviceDialog::~DeviceDialog()
{
  delete ui;
}

void DeviceDialog::on_allow_button_clicked()
{
  Q_EMIT allowed(device_id, ui->permanent_checkbox->isChecked());
  accept();
}

void DeviceDialog::on_block_button_clicked()
{
  Q_EMIT blocked(device_id, ui->permanent_checkbox->isChecked());
  accept();
}

void DeviceDialog::on_reject_button_clicked()
{
  Q_EMIT rejected(device_id, ui->permanent_checkbox->isChecked());
  accept();
}

/* vim: set ts=2 sw=2 et */

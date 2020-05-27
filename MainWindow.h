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
#pragma once
#ifdef HAVE_BUILD_CONFIG_H
  #include <build-config.h>
#endif

#include "DBusBridge.h"
#include "DeviceModel.h"
#include "TargetDelegate.h"

#include <QSystemTrayIcon>
#include <QMainWindow>
#include <QTimer>
#include <QSettings>

namespace Ui
{
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

protected slots:
  void switchVisibilityState(QSystemTrayIcon::ActivationReason reason);
  void flashStep();
  void dbusTryConnect();

  void showDeviceDialog(quint32 id, const usbguard::Rule& device_rule);
  void showMessage(const QString& message, bool alert = false, bool statusbar = false);
  void showNotification(QSystemTrayIcon::MessageIcon icon, const QString& title, const QString& message);

  void handleDevicePresenceChange(uint id,
    usbguard::DeviceManager::EventType event,
    usbguard::Rule::Target target,
    const QString& device_rule);

  void handleDevicePolicyChange(uint id,
    usbguard::Rule::Target target_old,
    usbguard::Rule::Target target_new,
    const QString& device_rule,
    uint rule_id);

  void notifyDBusConnected();
  void notifyDBusDisconnected();
  void notifyDevicePresenceChanged(usbguard::DeviceManager::EventType event, const usbguard::Rule& device_rule);
  void notifyDevicePolicyChanged(const usbguard::Rule& device_rule, quint32 rule_id);
  void notify(const QString& title, QSystemTrayIcon::MessageIcon icon, const usbguard::Rule& device_rule, bool show_notification);

  void allowDevice(quint32 id, bool permanent);
  void blockDevice(quint32 id, bool permanent);
  void rejectDevice(quint32 id, bool permanent);

  void handleDBusConnect();
  void handleDBusDisconnect();

  void handleDeviceInsert(quint32 id, const usbguard::Rule& device_rule);
  void handleDeviceRemove(quint32 id, const usbguard::Rule& device_rule);

  void loadSettings();
  void saveSettings();

  void loadDeviceList();
  void editDeviceListRow(const QModelIndex& index);
  void commitDeviceListChanges();
  void clearDeviceList();
  void resetDeviceList();

  void changeEvent(QEvent* e) override;
  void closeEvent(QCloseEvent* e) override;

  void setupSystemTray();
  void setupSettingsWatcher();
  void startFlashing();
  void stopFlashing();

private:
  Ui::MainWindow* ui;
  QSystemTrayIcon* systray;
  QTimer _flash_timer;
  bool _flash_state;
  QSettings _settings;
  DeviceModel _device_model;
  TargetDelegate _target_delegate;
  DBusBridge _bridge;
};

/* vim: set ts=2 sw=2 et */

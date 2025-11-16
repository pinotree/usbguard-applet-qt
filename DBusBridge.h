//
// Copyright (C) 2020 Pino Toscano
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
// Authors: Pino Toscano <toscano.pino@tiscali.it>
//
#pragma once

#include "DBusTypes.h"
#include "LibUsbguard.h"

#include <QDBusPendingReply>
#include <QDBusReply>
#include <QObject>
#include <QTimer>

class QDBusServiceWatcher;
class OrgUsbguardDevices1Interface;

class DBusBridge : public QObject
{
  Q_OBJECT

public:
  explicit DBusBridge(QObject* parent = nullptr);
  ~DBusBridge();

  QDBusReply<bool> tryConnect();
  bool isConnected() const;

  QDBusPendingReply<DBusRules> listDevices(const QString& query);
  QDBusPendingReply<uint> applyDevicePolicy(uint id, Rule::Target target, bool permanent);

Q_SIGNALS:
  void serviceAvailable();
  void serviceUnavailable();
  void devicePolicyApplied(uint id, Rule::Target target_new, const QString& device_rule, uint rule_id);
  void devicePolicyChanged(uint id, Rule::Target target_old, Rule::Target target_new, const QString& device_rule, uint rule_id);
  void devicePresenceChanged(uint id, DeviceManager::EventType event, Rule::Target target, const QString& device_rule);

private Q_SLOTS:
  void createInterfaces();
  void destroyInterfaces();
  void dbusServiceRegistered();
  void dbusDevicePolicyApplied(uint id, uint target_new, const QString& device_rule, uint rule_id, DBusAttributes attributes);
  void dbusDevicePolicyChanged(uint id, uint target_old, uint target_new, const QString& device_rule, uint rule_id, DBusAttributes attributes);
  void dbusDevicePresenceChanged(uint id, uint event, uint target, const QString& device_rule, DBusAttributes attributes);

private:
  QTimer _reconnect_timer;
  QDBusServiceWatcher* _watcher = nullptr;
  OrgUsbguardDevices1Interface* _devices_interface = nullptr;

  static const QString service;
};

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

#include "DBusBridge.h"

#include <OrgUsbguardInterface.h>

#include <QDBusConnection>
#include <QDBusServiceWatcher>

Q_DECLARE_METATYPE(DBusRule);
Q_DECLARE_METATYPE(DBusRules);
Q_DECLARE_METATYPE(DBusAttributes);

const QString DBusBridge::service = QLatin1String("org.usbguard1");

DBusBridge::DBusBridge(QObject* parent) :
  QObject(parent),
  _reconnect_timer(this),
  _watcher(nullptr),
  _devices_interface(nullptr)
{
  qDBusRegisterMetaType<DBusRule>();
  qDBusRegisterMetaType<DBusRules>();
  qDBusRegisterMetaType<DBusAttributes>();

  /*
   * Wait 5 seconds before trying to connect when the D-Bus service appears;
   * this is because usbguard-dbus has a 1 second timeout between the D-Bus
   * registration and the IPC connection.
   */
  _reconnect_timer.setInterval(5000);
  _reconnect_timer.setSingleShot(true);
  QObject::connect(&_reconnect_timer, SIGNAL(timeout()), this, SLOT(createInterfaces()));
}

DBusBridge::~DBusBridge()
{
}

QDBusReply<bool> DBusBridge::tryConnect()
{
  QDBusConnection bus = QDBusConnection::systemBus();

  if (!_watcher) {
    _watcher = new QDBusServiceWatcher(this);
    _watcher->setConnection(bus);
    _watcher->setWatchMode(QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration);
    _watcher->addWatchedService(service);
    QObject::connect(_watcher, SIGNAL(serviceRegistered(const QString&)),
      this, SLOT(dbusServiceRegistered()));
    QObject::connect(_watcher, SIGNAL(serviceUnregistered(const QString&)),
      this, SLOT(destroyInterfaces()));
  }

  QDBusReply<bool> reply = bus.interface()->isServiceRegistered(service);
  if (reply.isValid() && reply.value()) {
    createInterfaces();
  }
  return reply;
}

bool DBusBridge::isConnected() const
{
  return _devices_interface;
}

QDBusPendingReply<DBusRules> DBusBridge::listDevices(const QString& query)
{
  QDBusPendingReply<DBusRules> reply = _devices_interface->listDevices(query);
  reply.waitForFinished();
  return reply;
}

QDBusPendingReply<uint> DBusBridge::applyDevicePolicy(uint id, Rule::Target target, bool permanent)
{
  QDBusPendingReply<uint> reply = _devices_interface->applyDevicePolicy(id, static_cast<uint>(target), permanent);
  reply.waitForFinished();
  return reply;
}

void DBusBridge::createInterfaces()
{
  QDBusConnection bus = QDBusConnection::systemBus();

  _devices_interface = new OrgUsbguardDevices1Interface(service, QLatin1String("/org/usbguard1/Devices"), bus, this);
  QObject::connect(_devices_interface, SIGNAL(DevicePolicyApplied(uint, uint, const QString&, uint, DBusAttributes)),
    this, SLOT(dbusDevicePolicyApplied(uint, uint, const QString&, uint, DBusAttributes)));
  QObject::connect(_devices_interface, SIGNAL(DevicePolicyChanged(uint, uint, uint, const QString&, uint, DBusAttributes)),
    this, SLOT(dbusDevicePolicyChanged(uint, uint, uint, const QString&, uint, DBusAttributes)));
  QObject::connect(_devices_interface, SIGNAL(DevicePresenceChanged(uint, uint, uint, const QString&, DBusAttributes)),
    this, SLOT(dbusDevicePresenceChanged(uint, uint, uint, const QString&, DBusAttributes)));

  Q_EMIT serviceAvailable();
}

void DBusBridge::destroyInterfaces()
{
  _reconnect_timer.stop();
  Q_EMIT serviceUnavailable();

  delete _devices_interface;
  _devices_interface = nullptr;
}

void DBusBridge::dbusServiceRegistered()
{
  _reconnect_timer.start();
}

void DBusBridge::dbusDevicePolicyApplied(uint id, uint target_new, const QString& device_rule, uint rule_id, DBusAttributes attributes)
{
  Q_EMIT devicePolicyApplied(id, static_cast<Rule::Target>(target_new), device_rule, rule_id);
}

void DBusBridge::dbusDevicePolicyChanged(uint id, uint target_old, uint target_new, const QString& device_rule, uint rule_id, DBusAttributes attributes)
{
  Q_EMIT devicePolicyChanged(id, static_cast<Rule::Target>(target_old), static_cast<Rule::Target>(target_new), device_rule, rule_id);
}

void DBusBridge::dbusDevicePresenceChanged(uint id, uint event, uint target, const QString& device_rule, DBusAttributes attributes)
{
  Q_EMIT devicePresenceChanged(id, static_cast<DeviceManager::EventType>(event), static_cast<Rule::Target>(target), device_rule);
}

//
// Copyright (C) 2016 Red Hat, Inc.
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

#include "LibUsbguard.h"

#include <QAbstractItemModel>
#include <QList>
#include <QVariant>
#include <QMap>

class DeviceModelItem
{
public:
  DeviceModelItem();
  explicit DeviceModelItem(const Rule& device_rule, DeviceModelItem* parent);
  ~DeviceModelItem();

  void appendChild(DeviceModelItem* child);
  void removeChild(DeviceModelItem* child);

  DeviceModelItem* child(int row);
  int childCount() const;
  int columnCount() const;
  QVariant data(int column);
  int row() const;
  DeviceModelItem* parent();

  QString getDeviceHash() const;
  quint32 getDeviceID() const;

  Rule::Target getRequestedTarget() const;
  Rule::Target getDeviceTarget() const;

  void setRequestedTarget(Rule::Target target);
  void setDeviceTarget(Rule::Target target);

private:
  QList<DeviceModelItem*> _children;
  DeviceModelItem* _parent;
  Rule _device_rule;
  Rule::Target _requested_target;
};

class DeviceModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  enum {
    RuleTarget = Qt::UserRole + 1,  /* Rule::Target */
  };

  explicit DeviceModel(QObject* parent = nullptr);
  ~DeviceModel();

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  QModelIndex index(int row, int column,
    const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  void insertDevice(const Rule& device_rule);
  void updateDeviceTarget(quint32 device_id, Rule::Target target);

  void removeDevice(quint32 device_id);
  bool containsDevice(quint32 device_id) const;

  QModelIndex createRowEditIndex(const QModelIndex& index) const;
  QMap<quint32, Rule::Target> getModifiedDevices() const;

  void clear();

private:
  void removeDevice(DeviceModelItem* item, bool notify = false);

  QMap<QString, DeviceModelItem*> _hash_map;
  QMap<uint32_t, DeviceModelItem*> _id_map;
  DeviceModelItem* _root_item;
};

/* vim: set ts=2 sw=2 et */

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

#include "DeviceModel.h"
#include "Log.h"

#include <iostream>
#include <QList>
#include <QVector>
#include <QCoreApplication>

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

DeviceModelItem::DeviceModelItem()
{
  _parent = nullptr;
  _requested_target = Rule::Target::Invalid;
}

DeviceModelItem::DeviceModelItem(const Rule& device_rule, DeviceModelItem* parent)
{
  _parent = parent;
  _device_rule = device_rule;
  _requested_target = device_rule.getTarget();
}

DeviceModelItem::~DeviceModelItem()
{
  _parent = nullptr;
  qDeleteAll(_children);
}

void DeviceModelItem::appendChild(DeviceModelItem* child)
{
  _children.append(child);
}

void DeviceModelItem::removeChild(DeviceModelItem* child)
{
  (void)_children.takeAt(_children.indexOf(child));
}

DeviceModelItem* DeviceModelItem::child(int row)
{
  return _children.value(row);
}

int DeviceModelItem::childCount() const
{
  return _children.count();
}

int DeviceModelItem::columnCount() const
{
  return 8;
}

QVariant DeviceModelItem::data(int column)
{
  switch (column) {
  case 0:
    return QVariant(_device_rule.getRuleID());

  case 1:
    return QVariant(_requested_target != _device_rule.getTarget() ? QLatin1String("*") : QString());

  case 2:
    switch (_requested_target) {
    case Rule::Target::Allow:
      return QCoreApplication::translate("DeviceModel", "Allow");

    case Rule::Target::Block:
      return QCoreApplication::translate("DeviceModel", "Block");

    case Rule::Target::Reject:
      return QCoreApplication::translate("DeviceModel", "Reject");

    case Rule::Target::Unknown:
    case Rule::Target::Empty:
    case Rule::Target::Invalid:
    case Rule::Target::Match:
    case Rule::Target::Device:
    default:
      return QVariant(Rule::targetToString(_requested_target));
    }

  case 3:
    return QVariant(QString::fromStdString(_device_rule.getDeviceID().toString()));

  case 4:
    return QVariant(_device_rule.getName());

  case 5:
    return QVariant(_device_rule.getSerial());

  case 6:
    return QVariant(_device_rule.getViaPort());

  case 7: {
    QString interface_string;

    for (auto interface : _device_rule.attributeWithInterface().values()) {
      interface_string.append(QString::fromStdString(interface.toRuleString()));
      interface_string.append(QLatin1String(" "));
    }

    return QVariant(interface_string);
  }

  default:
    return QVariant();
  }
}

int DeviceModelItem::row() const
{
  if (_parent) {
    return _parent->_children.indexOf(const_cast<DeviceModelItem*>(this));
  }
  else {
    return 0;
  }
}

DeviceModelItem* DeviceModelItem::parent()
{
  return _parent;
}

Rule::Target DeviceModelItem::getRequestedTarget() const
{
  return _requested_target;
}

Rule::Target DeviceModelItem::getDeviceTarget() const
{
  return _device_rule.getTarget();
}

void DeviceModelItem::setRequestedTarget(Rule::Target target)
{
  _requested_target = target;
}

void DeviceModelItem::setDeviceTarget(Rule::Target target)
{
  _device_rule.setTarget(target);
  _requested_target = target;
}

QString DeviceModelItem::getDeviceHash() const
{
  return _device_rule.getHash();
}

quint32 DeviceModelItem::getDeviceID() const
{
  return _device_rule.getRuleID();
}

DeviceModel::DeviceModel(QObject* parent)
  : QAbstractItemModel(parent)
{
  _root_item = new DeviceModelItem();
}

DeviceModel::~DeviceModel()
{
  clear();
  delete _root_item;
}

QVariant DeviceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation != Qt::Horizontal) {
    return QVariant();
  }

  if (role != Qt::DisplayRole && role != Qt::TextAlignmentRole) {
    return QVariant();
  }

  switch (section) {
  case 0:
    if (role == Qt::DisplayRole) {
      return tr("ID");
    }
    else {
      return Qt::AlignCenter;
    }

    break;

  case 1:
    if (role == Qt::DisplayRole) {
      return tr(" M "); /* Modified flag */
    }

    if (role == Qt::TextAlignmentRole) {
      return Qt::AlignCenter;
    }

    break;

  case 2:
    if (role == Qt::DisplayRole) {
      return tr("Target");
    }

    if (role == Qt::TextAlignmentRole) {
      return Qt::AlignCenter;
    }

    break;

  case 3:
    if (role == Qt::DisplayRole) {
      return tr("USB ID");
    }

    if (role == Qt::TextAlignmentRole) {
      return Qt::AlignCenter;
    }

    break;

  case 4:
    if (role == Qt::DisplayRole) {
      return tr("Name");
    }

    break;

  case 5:
    if (role == Qt::DisplayRole) {
      return tr("Serial");
    }

    break;

  case 6:
    if (role == Qt::DisplayRole) {
      return tr("Port");
    }

    break;

  case 7:
    if (role == Qt::DisplayRole) {
      return tr("Interfaces");
    }

    break;

  default:
    break;
  }

  return QVariant();
}

QModelIndex DeviceModel::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  DeviceModelItem* parent_item = nullptr;

  if (!parent.isValid()) {
    parent_item = _root_item;
  }
  else {
    parent_item = static_cast<DeviceModelItem*>(parent.internalPointer());
  }

  DeviceModelItem* child_item = parent_item->child(row);

  if (child_item) {
    return createIndex(row, column, child_item);
  }
  else {
    return QModelIndex();
  }
}

QModelIndex DeviceModel::parent(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return QModelIndex();
  }

  DeviceModelItem* child_item = static_cast<DeviceModelItem*>(index.internalPointer());
  DeviceModelItem* parent_item = child_item->parent();

  if (parent_item == _root_item) {
    return QModelIndex();
  }
  else {
    return createIndex(parent_item->row(), 0, parent_item);
  }
}

int DeviceModel::rowCount(const QModelIndex& parent) const
{
  if (parent.column() > 0) {
    return 0;
  }

  DeviceModelItem* parent_item = nullptr;

  if (!parent.isValid()) {
    parent_item = _root_item;
  }
  else {
    parent_item = static_cast<DeviceModelItem*>(parent.internalPointer());
  }

  return parent_item->childCount();
}

int DeviceModel::columnCount(const QModelIndex& parent) const
{
  if (!parent.isValid()) {
    return _root_item->columnCount();
  }
  else {
    return static_cast<DeviceModelItem*>(parent.internalPointer())->columnCount();
  }
}

QVariant DeviceModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  if (role == Qt::TextAlignmentRole) {
    return headerData(index.column(), Qt::Horizontal, role);
  }

  DeviceModelItem* item = static_cast<DeviceModelItem*>(index.internalPointer());

  switch (role) {
  case Qt::DisplayRole:
    return item->data(index.column());

  case RuleTarget:
    return QVariant::fromValue(item->getDeviceTarget());

  default:
    return QVariant();
  }
}

Qt::ItemFlags DeviceModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }

  if (index.column() == 2) {
    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
  }
  else {
    return QAbstractItemModel::flags(index);
  }
}

bool DeviceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid()) {
    return false;
  }

  DeviceModelItem* item = static_cast<DeviceModelItem*>(index.internalPointer());

  switch (role) {
  case RuleTarget: {
    const Rule::Target target = value.value<Rule::Target>();
    qCDebug(LOG) << "item=" << item << " target=" << target;

    if (item->getRequestedTarget() != target) {
      item->setRequestedTarget(target);
      Q_EMIT dataChanged(createIndex(item->row(), 0, item),
        createIndex(item->row(), item->columnCount() - 1, item),
        QVector<int>() << Qt::DisplayRole);
      return true;
    }

    return false;
  }

  default:
    return false;
  }
}

void DeviceModel::insertDevice(const Rule& device_rule)
{
  qCDebug(LOG) << "device_rule=" << device_rule;
  const uint32_t device_id = device_rule.getRuleID();
  const QString device_hash = device_rule.getHash();
  const QString parent_hash = device_rule.getParentHash();
  DeviceModelItem* parent_item = _hash_map.value(parent_hash, _root_item);
  DeviceModelItem* child_item = new DeviceModelItem(device_rule, parent_item);
  beginInsertRows(createIndex(parent_item->row(), 0, parent_item),
    parent_item->childCount(), parent_item->childCount());
  parent_item->appendChild(child_item);
  _hash_map.insert(device_hash, child_item);
  _id_map.insert(device_id, child_item);
  endInsertRows();
}

void DeviceModel::updateDeviceTarget(quint32 device_id, Rule::Target target)
{
  qCDebug(LOG) << "device_id=" << device_id
    << " target=" << target;
  DeviceModelItem* item = _id_map.value(device_id, nullptr);

  if (item == nullptr) {
    return;
  }

  if (item->getDeviceTarget() != target) {
    item->setDeviceTarget(target);
    Q_EMIT dataChanged(createIndex(item->row(), 0, item),
      createIndex(item->row(), item->columnCount() - 1, item),
      QVector<int>() << Qt::DisplayRole);
  }
}

void DeviceModel::removeDevice(quint32 device_id)
{
  qCDebug(LOG) << "device_id=" << device_id;
  DeviceModelItem* item = _id_map.value(device_id, nullptr);

  if (item == nullptr) {
    return;
  }
  else {
    removeDevice(item, /*notify=*/true);
  }
}

void DeviceModel::removeDevice(DeviceModelItem* item, bool notify)
{
  qCDebug(LOG) << "item=" << item << " notify=" << notify;
  DeviceModelItem* parent_item = item->parent();

  if (parent_item == nullptr) {
    return;
  }

  if (notify) {
    beginRemoveRows(createIndex(parent_item->row(), 0, parent_item), item->row(), item->row());
  }

  while (item->childCount() > 0) {
    removeDevice(item->child(0), /*notify=*/false);
  }

  _hash_map.remove(item->getDeviceHash());
  _id_map.remove(item->getDeviceID());
  parent_item->removeChild(item);
  delete item;

  if (notify) {
    endRemoveRows();
  }
}

bool DeviceModel::containsDevice(quint32 device_id) const
{
  return _id_map.count(device_id) > 0;
}

QModelIndex DeviceModel::createRowEditIndex(const QModelIndex& index) const
{
  return createIndex(index.row(), 2, index.internalPointer());
}

QMap<quint32, Rule::Target> DeviceModel::getModifiedDevices() const
{
  QMap<quint32, Rule::Target> modified_map;

  for (auto item : _id_map) {
    if (item->getDeviceTarget() != item->getRequestedTarget()) {
      modified_map.insert(item->getDeviceID(), item->getRequestedTarget());
    }
  }

  return modified_map;
}

void DeviceModel::clear()
{
  qCDebug(LOG);
  beginResetModel();

  while (_root_item->childCount() > 0) {
    removeDevice(_root_item->child(0), /*notify=*/false);
  }

  endResetModel();
}

/* vim: set ts=2 sw=2 et */

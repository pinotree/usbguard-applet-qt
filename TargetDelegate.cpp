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

#include "TargetDelegate.h"
#include "DeviceModel.h"

#include <QComboBox>
#include <QCoreApplication>

TargetDelegate::TargetDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QWidget* TargetDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  (void)option;
  (void)index;
  QComboBox* editor = new QComboBox(parent);
  editor->setFrame(false);
  editor->setAutoFillBackground(true);
  editor->addItem(QCoreApplication::translate("DeviceModel", "Allow"), QVariant::fromValue(Rule::Target::Allow));
  editor->addItem(QCoreApplication::translate("DeviceModel", "Block"), QVariant::fromValue(Rule::Target::Block));
  editor->addItem(QCoreApplication::translate("DeviceModel", "Reject"), QVariant::fromValue(Rule::Target::Reject));
  return editor;
}

void TargetDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QComboBox* combobox = static_cast<QComboBox*>(editor);
  const int value_index = combobox->findData(index.data(DeviceModel::RuleTarget));

  if (value_index != -1) {
    combobox->setCurrentIndex(value_index);
  }
}

void TargetDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QComboBox* combobox = static_cast<QComboBox*>(editor);
  model->setData(index, combobox->itemData(combobox->currentIndex()), DeviceModel::RuleTarget);
}

void TargetDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  (void)index;
  editor->setGeometry(option.rect);
}

/* vim: set ts=2 sw=2 et */

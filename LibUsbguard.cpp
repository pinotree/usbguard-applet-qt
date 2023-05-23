//
// Copyright (C) 2023 Pino Toscano
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

#include "LibUsbguard.h"

#include <QDebug>

#include <limits>

// MUST match usbguard::Rule::ImplicitID
const uint32_t Rule::ImplicitID = std::numeric_limits<uint32_t>::max() - 1;

Rule::Rule()
{
}

Rule::~Rule()
{
}

const usbguard::Rule::Attribute<usbguard::USBInterfaceType>& Rule::attributeWithInterface() const
{
  return _rule.attributeWithInterface();
}

const usbguard::USBDeviceID& Rule::getDeviceID() const
{
  return _rule.getDeviceID();
}

QString Rule::getHash() const
{
  return QString::fromStdString(_rule.getHash());
}

QString Rule::getName() const
{
  return QString::fromStdString(_rule.getName());
}

QString Rule::getParentHash() const
{
  return QString::fromStdString(_rule.getParentHash());
}

uint32_t Rule::getRuleID() const
{
  return _rule.getRuleID();
}

QString Rule::getSerial() const
{
  return QString::fromStdString(_rule.getSerial());
}

Rule::Target Rule::getTarget() const
{
  return static_cast<Rule::Target>(_rule.getTarget());
}

QString Rule::getViaPort() const
{
  return QString::fromStdString(_rule.getViaPort());
}

void Rule::setRuleID(uint32_t rule_id)
{
  _rule.setRuleID(rule_id);
}

void Rule::setTarget(Rule::Target target)
{
  _rule.setTarget(static_cast<usbguard::Rule::Target>(target));
}

QString Rule::toString() const
{
  return QString::fromStdString(_rule.toString());
}

Rule Rule::fromString(const QString& str)
{
  Rule r;
  r._rule = usbguard::Rule::fromString(str.toStdString());
  return r;
}

QString Rule::targetToString(Rule::Target target)
{
  return QString::fromStdString(usbguard::Rule::targetToString(static_cast<usbguard::Rule::Target>(target)));
}

QDebug& operator<<(QDebug& out, const Rule& rule)
{
  QDebugStateSaver saver(out);
  out.noquote() << rule.toString();
  return out;
}

QDebug& operator<<(QDebug& out, const Rule::Target& target)
{
  QDebugStateSaver saver(out);
  out.noquote() << Rule::targetToString(target);
  return out;
}

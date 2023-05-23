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
#pragma once

#include <QMetaType>
#include <QString>

#include <stdint.h>

#include <Rule.hpp>
#include <USB.hpp>

class QDebug;

class Rule
{
public:
  // MUST match usbguard::Rule::Target
  enum class Target {
    Allow = 0,
    Block = 1,
    Reject = 2,
    Match = 3,
    Unknown = 4,
    Device = 5,
    Empty = 6,
    Invalid = 7,
  };

  static const uint32_t ImplicitID;

  Rule();
  ~Rule();

  const usbguard::Rule::Attribute<usbguard::USBInterfaceType>& attributeWithInterface() const;
  const usbguard::USBDeviceID& getDeviceID() const;
  QString getHash() const;
  QString getName() const;
  QString getParentHash() const;
  uint32_t getRuleID() const;
  QString getSerial() const;
  Target getTarget() const;
  QString getViaPort() const;

  void setRuleID(uint32_t rule_id);
  void setTarget(Target target);

  static Rule fromString(const QString& str);
  static QString targetToString(Target target);

private:
  QString toString() const;

  usbguard::Rule _rule;

  friend QDebug& operator<<(QDebug& out, const Rule& rule);
};

QDebug& operator<<(QDebug& out, const Rule& rule);
QDebug& operator<<(QDebug& out, const Rule::Target& target);

Q_DECLARE_METATYPE(Rule::Target);

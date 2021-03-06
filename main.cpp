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

#include "MainWindow.h"

#include "usbguard/Logger.hpp"

#include <QApplication>
#include <QLocale>
#include <QSessionManager>
#include <QTranslator>
#include <QString>

int main(int argc, char* argv[])
{
  QApplication a(argc, argv);
  QTranslator translator;
  USBGUARD_LOG(Debug) << "Loading translations for locale: "
    << QLocale::system().name().toStdString();

  if (translator.load(QLocale::system(),
      /*filename=*/QString(),
      /*prefix=*/QString(),
      /*directory=*/":/translations",
      /*suffix=*/".qm")) {
    a.installTranslator(&translator);
  }
  else {
    USBGUARD_LOG(Debug) << "Translations not available for the current locale.";
  }

  auto disableSessionManagement = [](QSessionManager &sm) {
    sm.setRestartHint(QSessionManager::RestartNever);
  };
  QObject::connect(&a, &QGuiApplication::commitDataRequest, disableSessionManagement);
  QObject::connect(&a, &QGuiApplication::saveStateRequest, disableSessionManagement);

  MainWindow w;
  a.setQuitOnLastWindowClosed(false);
  return a.exec();
}

/* vim: set ts=2 sw=2 et */

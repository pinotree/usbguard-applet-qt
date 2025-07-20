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
#include "Log.h"

#include <QApplication>
#include <QLocale>
#include <QSessionManager>
#include <QTranslator>
#include <QString>

int main(int argc, char* argv[])
{
  QApplication a(argc, argv);
  QTranslator translator;
  qCDebug(LOG) << "Loading translations for locale: "
    << QLocale::system().name();

  if (translator.load(QLocale::system(),
      /*filename=*/QString(),
      /*prefix=*/QString(),
      /*directory=*/QLatin1String(":/translations"),
      /*suffix=*/QLatin1String(".qm"))) {
    a.installTranslator(&translator);
  }
  else {
    qCDebug(LOG) << "Translations not available for the current locale.";
  }

  auto disableSessionManagement = [](QSessionManager &sm) {
    sm.setRestartHint(QSessionManager::RestartNever);
  };
  QObject::connect(&a, &QGuiApplication::commitDataRequest, &a, disableSessionManagement);
  QObject::connect(&a, &QGuiApplication::saveStateRequest, &a, disableSessionManagement);

  MainWindow w;
  a.setQuitOnLastWindowClosed(false);
  return a.exec();
}

/* vim: set ts=2 sw=2 et */

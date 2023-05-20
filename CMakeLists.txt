cmake_minimum_required(VERSION 3.4)
project(usbguard-applet-qt)

find_package(Qt5 5.10 CONFIG REQUIRED COMPONENTS Core Gui Widgets DBus LinguistTools)
find_package(PkgConfig REQUIRED)
pkg_check_modules(USBGUARD REQUIRED IMPORTED_TARGET libusbguard)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB ts_files
  "translations/*.ts"
)
qt5_add_translation(qm_files ${ts_files})
add_custom_command(
  OUTPUT translations.qrc
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/translations/generate_translations_qrc.sh" translations.qrc ${qm_files}
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/translations/generate_translations_qrc.sh" ${qm_files}
)

set(applet_SOURCES
  DBusBridge.cpp
  DeviceDialog.cpp
  DeviceModel.cpp
  Log.cpp
  MainWindow.cpp
  TargetDelegate.cpp
  main.cpp
)
set(applet_FORMS
  DeviceDialog.ui
  MainWindow.ui
)
set(applet_RESOURCES
  resources/icon.qrc
  "${CMAKE_BINARY_DIR}/translations.qrc"
)
set_source_files_properties(org.usbguard1.xml PROPERTIES INCLUDE DBusTypes.h)
qt5_add_dbus_interface(applet_SOURCES
  org.usbguard1.xml
  OrgUsbguardInterface
)

add_executable(usbguard-applet-qt
  ${applet_SOURCES}
  ${applet_FORMS}
  ${applet_RESOURCES}
)

target_link_libraries(usbguard-applet-qt
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::DBus
  PkgConfig::USBGUARD
)

install(TARGETS usbguard-applet-qt
  DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES usbguard-applet-qt.desktop
  DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
install(FILES resources/usbguard-icon.svg
  DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps")

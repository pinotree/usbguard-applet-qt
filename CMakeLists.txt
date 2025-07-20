cmake_minimum_required(VERSION 3.10)
project(usbguard-applet-qt)

find_package(Qt6 6.0 CONFIG REQUIRED COMPONENTS Core Gui Widgets DBus LinguistTools)
find_package(PkgConfig REQUIRED)
pkg_check_modules(USBGUARD REQUIRED IMPORTED_TARGET libusbguard)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-DQT_ENABLE_STRICT_MODE_UP_TO=0x060800)
add_definitions(-DQT_DISABLE_DEPRECATED_UP_TO=0x060000)
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DQT_NO_CAST_FROM_ASCII)

set(applet_SOURCES
  DBusBridge.cpp
  DeviceDialog.cpp
  DeviceModel.cpp
  LibUsbguard.cpp
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
)
set_source_files_properties(org.usbguard1.xml PROPERTIES INCLUDE DBusTypes.h)
qt6_add_dbus_interface(applet_SOURCES
  org.usbguard1.xml
  OrgUsbguardInterface
)

add_executable(usbguard-applet-qt
  ${applet_SOURCES}
  ${applet_FORMS}
  ${applet_RESOURCES}
)

target_link_libraries(usbguard-applet-qt
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  Qt6::DBus
  PkgConfig::USBGUARD
)

file(GLOB ts_files
  "translations/*.ts"
)
qt6_add_lrelease(
  TS_FILES ${ts_files}
  QM_FILES_OUTPUT_VARIABLE qm_files
)
qt6_add_resources(usbguard-applet-qt translations
  PREFIX "/translations"
  BASE "${CMAKE_CURRENT_BINARY_DIR}"
  FILES "${qm_files}"
)

install(TARGETS usbguard-applet-qt
  DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES usbguard-applet-qt.desktop
  DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
install(FILES resources/usbguard-icon.svg
  DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps")

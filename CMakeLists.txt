cmake_minimum_required(VERSION 2.8.5)
project(usbguard-applet-qt)

find_package(Qt5 CONFIG REQUIRED COMPONENTS Core Gui Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(USBGUARD REQUIRED libusbguard)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${USBGUARD_INCLUDE_DIRS})

set(applet_SOURCES
  DeviceDialog.cpp
  DeviceModel.cpp
  MainWindow.cpp
  SessionBlocker.cpp
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

add_executable(usbguard-applet-qt
  ${applet_SOURCES}
  ${applet_FORMS}
  ${applet_RESOURCES}
)

target_link_libraries(usbguard-applet-qt
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  ${USBGUARD_LIBRARIES}
)

install(TARGETS usbguard-applet-qt
  DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES usbguard-applet-qt.desktop
  DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
install(FILES resources/usbguard-icon.svg
  DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps")

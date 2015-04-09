cmake_minimum_required(VERSION 2.8)
include(GNUInstallDirs)
project(usbguard-applet-qt)

find_package(Qt4 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(USBGUARD REQUIRED libusbguard)
include_directories(${USBGUARD_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
set(applet_SOURCES src/main.cpp src/mainwindow.cpp src/devicedialog.cpp)
set(applet_HEADERS src/mainwindow.h src/devicedialog.h)
set(applet_FORMS src/mainwindow.ui src/devicedialog.ui)
set(applet_RESOURCES src/icon.qrc)

qt4_wrap_cpp(applet_HEADERS_MOC ${applet_HEADERS})
qt4_wrap_ui(applet_FORMS_HEADERS ${applet_FORMS})
qt4_add_resources(applet_RESOURCES_RCC ${applet_RESOURCES})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_executable(usbguard-applet-qt ${applet_SOURCES}
				  ${applet_HEADERS_MOC}
				  ${applet_FORMS_HEADERS}
				  ${applet_RESOURCES_RCC})

target_link_libraries(usbguard-applet-qt ${QT_LIBRARIES} ${USBGUARD_LIBRARIES})

add_custom_target(man ALL)
add_custom_command(
  TARGET man
  SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/doc/usbguard-applet-qt.roff
  COMMAND sh ARGS -c 'gzip -c ${CMAKE_CURRENT_SOURCE_DIR}/doc/usbguard-applet-qt.roff > ${CMAKE_CURRENT_BINARY_DIR}/usbguard-applet-qt.1.gz'
  OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/usbguard-applet-qt.1.gz
)
add_custom_command(
  TARGET man
  SOURCE man
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/usbguard-applet-qt.1.gz
)

install(TARGETS "usbguard-applet-qt"
  DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/usbguard-applet-qt.1.gz"
  DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/dist/usbguard-applet-qt.desktop"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")

set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "3")
set(CPACK_PACKAGE_DESCRIPTION "Qt applet for interacting with the USBGuard daemon")
set(CPACK_PACKAGE_CONTACT "Daniel Kopecek dkopecek@redhat.com")
set(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
set(CPACK_SOURCE_IGNORE_FILES
  "/_build/;/.git/;~$;${CPACK_SOURCE_IGNORE_FILES}")
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
include(CPack)

#-------------------------------------------------
#
# Project created by QtCreator 2015-03-13T19:18:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

VERSION = 0.1
TARGET = usbguard-applet-qt
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES +=\
    main.cpp\
    mainwindow.cpp \
    devicedialog.cpp

HEADERS +=\
    mainwindow.h \
    devicedialog.h

FORMS += mainwindow.ui \
    devicedialog.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libusbguard

RESOURCES += \
    icon.qrc


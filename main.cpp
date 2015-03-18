#include "mainwindow.h"
#include <QApplication>
#include <usbguard/Logging.hpp>

int main(int argc, char *argv[])
{
    usbguard::setupLogger(true, false, true, std::string());
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    a.setQuitOnLastWindowClosed(false);
    return a.exec();
}

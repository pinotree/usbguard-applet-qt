#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "devicedialog.h"
#include <QString>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QMessageBox>
#include <QApplication>
#include <QResource>
#include <QMenu>
#include <QAction>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("USBGuard");
    setWindowIcon(QIcon(":/icon.svg"));
    setupSystemTray();
    showMinimized();

    QObject::connect(&_ipc_timer, SIGNAL(timeout()), this, SLOT(ipcTryConnect()));
    QObject::connect(this, SIGNAL(DeviceInserted(quint32,QString,bool)), this, SLOT(showDeviceDialog(quint32,QString,bool)));
    QObject::connect(this, SIGNAL(DeviceInserted(quint32,QString,bool)), this, SLOT(notifyInserted(quint32,QString,bool)));
    QObject::connect(this, SIGNAL(DeviceAllowed(quint32,QString)), this, SLOT(notifyAllowed(quint32,QString)));
    QObject::connect(this, SIGNAL(DeviceBlocked(quint32,QString)), this, SLOT(notifyBlocked(quint32,QString)));
    QObject::connect(this, SIGNAL(DeviceRejected(quint32,QString)), this, SLOT(notifyRejected(quint32,QString)));
    QObject::connect(this, SIGNAL(DeviceRemoved(quint32,QString)), this, SLOT(notifyRemoved(quint32,QString)));
    QObject::connect(this, SIGNAL(Connected()), this, SLOT(handleIPCConnect()));
    QObject::connect(this, SIGNAL(Disconnected()), this, SLOT(handleIPCDisconnect()));

    _ipc_timer.setInterval(1000);
    _ipc_timer.start();
    ui->statusBar->showMessage("Inactive. No IPC connection.");

#if 1
    /* Hide things which arent' working yet */
    ui->tabWidget->removeTab(0);
    ui->tabWidget->removeTab(1);
    ui->device_tab->hide();
    ui->settings_tab->hide();
#endif
    return;
}

void MainWindow::setupSystemTray()
{
    systray = new QSystemTrayIcon(QIcon(":/icon.svg"), this);
    systray->setToolTip("USBGuard");
    auto menu = new QMenu();
    auto quit_action = new QAction("Quit", systray);
    menu->addAction(quit_action);
    systray->setContextMenu(menu);
    QObject::connect(quit_action, SIGNAL(triggered()), qApp, SLOT(quit()));
    QObject::connect(systray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                     this, SLOT(switchVisibilityState(QSystemTrayIcon::ActivationReason)));
    QObject::connect(&_flash_timer, SIGNAL(timeout()), this, SLOT(flashStep()));    
    systray->show();
}

void MainWindow::switchVisibilityState(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Context) {
        systray->contextMenu()->show();
        return;
    } else {
        if (this->windowState() & Qt::WindowMinimized) {
            showNormal();
            stopFlashing();
        } else {
            showMinimized();
        }
    }
}

MainWindow::~MainWindow()
{
    IPCClient::disconnect();
    delete ui;
}

void MainWindow::showDeviceDialog(quint32 seqn, const QString &name, bool rule_match)
{
    if (!rule_match) {
        DeviceDialog *d = new DeviceDialog(seqn, QString(name), QString());
        QObject::connect(d, SIGNAL(allowed(quint32,bool)), this, SLOT(allowDevice(quint32,bool)));
        QObject::connect(d, SIGNAL(rejected(quint32,bool)), this, SLOT(rejectDevice(quint32,bool)));
        QObject::connect(d, SIGNAL(blocked(quint32,bool)), this, SLOT(blockDevice(quint32,bool)));
        d->setModal(false);
        d->show();
    }
}

void MainWindow::showMessage(const QString& message, bool alert)
{
    const QString text("[%1] %2%3%4");
    const QString dt_string = QDateTime::currentDateTime().toString();
    ui->messages_text->append(text.arg(dt_string).arg(alert?"<b>":"").arg(message).arg(alert?"</b>":""));

    return;
}

void MainWindow::notifyInserted(quint32 seqn,const QString &name, bool rule_matched)
{
    if (rule_matched) {
        systray->showMessage("USB Device Inserted", QString("Name: %1").arg(name), QSystemTrayIcon::Information);
    }
    showMessage(QString("<i>Inserted</i>: %1").arg(name));
}

void MainWindow::notifyRemoved(quint32 seqn,const QString &name)
{
    systray->showMessage("USB Device Removed", QString("Name: %1").arg(name), QSystemTrayIcon::Information);
    showMessage(QString("<i>Removed</i>: %1").arg(name));
}

void MainWindow::notifyAllowed(quint32 seqn, const QString &name)
{
    systray->showMessage("USB Device Allowed", QString("Name: %1").arg(name), QSystemTrayIcon::Information);
    showMessage(QString("Allowed: %1").arg(name));
}

void MainWindow::notifyBlocked(quint32 seqn, const QString &name)
{
    systray->showMessage("USB Device Blocked", QString("Name: %1").arg(name), QSystemTrayIcon::Warning);
    showMessage(QString("Blocked: %1").arg(name));

}

void MainWindow::notifyRejected(quint32 seqn, const QString &name)
{
    systray->showMessage("USB Device Rejected", QString("Name: %1").arg(name), QSystemTrayIcon::Critical);
    showMessage(QString("Rejected: %1").arg(name), true);
    if (this->windowState() & Qt::WindowMinimized) {
        startFlashing();
    }
}

void MainWindow::notifyIPCConnected()
{
    systray->showMessage("IPC Connection Established", "", QSystemTrayIcon::Information);
    showMessage("IPC connection established");
    ui->statusBar->showMessage("IPC connection established.");
}

void MainWindow::notifyIPCDisconnected()
{
    systray->showMessage("IPC Connection Lost", "", QSystemTrayIcon::Information);
    showMessage("IPC connection lost");
    ui->statusBar->showMessage("Inactive. No IPC connection.");
}

void MainWindow::startFlashing()
{
    _flash_state = false;
    _flash_timer.setInterval(500);
    _flash_timer.start();
}

void MainWindow::stopFlashing()
{
    _flash_state = false;
    _flash_timer.stop();
    systray->setIcon(QIcon(":/icon.svg"));
}

void MainWindow::flashStep()
{
    if (_flash_state) {
        systray->setIcon(QIcon(":/icon-warning.svg"));
        systray->show();
        _flash_timer.setInterval(250);
        _flash_state = false;
    }
    else {
        systray->setIcon(QIcon(":/icon.svg"));
        systray->show();
        _flash_timer.setInterval(500);
        _flash_state = true;
    }
}

void MainWindow::ipcTryConnect()
{
    try {
        IPCClient::connect();
    }
    catch(...) {
    }
}

void MainWindow::allowDevice(quint32 seqn, bool append)
{
    try {
        IPCClient::allowDevice(seqn, append, 0);
    }
    catch(...) {
    }
}

void MainWindow::blockDevice(quint32 seqn, bool append)
{
    try {
        IPCClient::blockDevice(seqn, append, 0);
    }
    catch(...) {
    }
}

void MainWindow::rejectDevice(quint32 seqn, bool append)
{
    try {
        IPCClient::rejectDevice(seqn, append, 0);
    } catch(...) {
    }
}

void MainWindow::handleIPCConnect()
{
    _ipc_timer.stop();
    notifyIPCConnected();
}

void MainWindow::handleIPCDisconnect()
{
    _ipc_timer.start();
    notifyIPCDisconnected();
}

void MainWindow::changeEvent(QEvent* e)
{
    switch (e->type())
    {
        case QEvent::LanguageChange:
            this->ui->retranslateUi(this);
            break;
        case QEvent::WindowStateChange:
            {
                if (this->windowState() & Qt::WindowMinimized)
                {
                    QTimer::singleShot(250, this, SLOT(hide()));
                }
                break;
            }
        default:
            break;
    }

    QMainWindow::changeEvent(e);
}

void MainWindow::DeviceInserted(quint32 seqn, const std::string &name, const std::string &usb_class, const std::string &vendor_id, const std::string &product_id, bool rule_match, quint32 rule_seqn)
{
    emit DeviceInserted(seqn, QString(name.c_str()), rule_match);
}

void MainWindow::DeviceRemoved(quint32 seqn, const std::string &name, const std::string &usb_class, const std::string &vendor_id, const std::string &product_id)
{
    emit DeviceRemoved(seqn, QString(name.c_str()));
}

void MainWindow::DeviceAllowed(quint32 seqn, const std::string &name, const std::string &usb_class, const std::string &vendor_id, const std::string &product_id, bool rule_match, quint32 rule_seqn)
{
    emit DeviceAllowed(seqn, QString(name.c_str()));
}

void MainWindow::DeviceBlocked(quint32 seqn, const std::string &name, const std::string &usb_class, const std::string &vendor_id, const std::string &product_id, bool rule_match, quint32 rule_seqn)
{
    emit DeviceBlocked(seqn, QString(name.c_str()));
}

void MainWindow::DeviceRejected(quint32 seqn, const std::string &name, const std::string &usb_class, const std::string &vendor_id, const std::string &product_id, bool rule_match, quint32 rule_seqn)
{
    emit DeviceRejected(seqn, QString(name.c_str()));
}

void MainWindow::IPCConnected()
{
    emit Connected();
}

void MainWindow::IPCDisconnected()
{
    emit Disconnected();
}

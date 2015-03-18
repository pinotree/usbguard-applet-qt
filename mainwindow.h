#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSystemTrayIcon>
#include <QMainWindow>
#include <QTimer>
#include <usbguard/IPCClient.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public usbguard::IPCClient
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void DeviceInserted(quint32 seqn, const QString& name, bool rule_match);
    void DeviceRemoved(quint32 seqn, const QString& name);
    void DeviceAllowed(quint32 seqn, const QString& name);
    void DeviceDenied(quint32 seqn, const QString& name);
    void DeviceRejected(quint32 seqn, const QString& name);
    void Connected();
    void Disconnected();

protected slots:
    void switchVisibilityState(QSystemTrayIcon::ActivationReason reason);
    void flashStep();
    void ipcTryConnect();

    void showDeviceDialog(quint32 seqn, const QString& name, bool rule_match);
    void showMessage(const QString &message, bool alert = false);

    void notifyInserted(quint32 seqn, const QString &name, bool rule_matched);
    void notifyRemoved(quint32 seqn, const QString& name);
    void notifyAllowed(quint32 seqn, const QString& name);
    void notifyDenied(quint32 seqn, const QString& name);
    void notifyRejected(quint32 seqn, const QString& name);
    void notifyIPCConnected();
    void notifyIPCDisconnected();

    void allowDevice(quint32 seqn, bool append);
    void denyDevice(quint32 seqn, bool append);
    void rejectDevice(quint32 seqn, bool append);

    void handleIPCConnect();
    void handleIPCDisconnect();

protected:
    void changeEvent(QEvent *e);
    void setupSystemTray();
    void startFlashing();
    void stopFlashing();

    void DeviceInserted(quint32 seqn, const std::string &name, const std::string &usb_class, const std::string &vendor_id, const std::string &product_id, bool rule_match, quint32 rule_seqn);
    void DeviceRemoved(quint32 seqn, const std::string &name, const std::string &usb_class, const std::string &vendor_id, const std::string &product_id);
    void DeviceAllowed(quint32 seqn, const std::string &name, const std::string &usb_class, const std::string &vendor_id, const std::string &product_id, bool rule_match, quint32 rule_seqn);
    void DeviceDenied(quint32 seqn, const std::string &name, const std::string &usb_class, const std::string &vendor_id, const std::string &product_id, bool rule_match, quint32 rule_seqn);
    void DeviceRejected(quint32 seqn, const std::string &name, const std::string &usb_class, const std::string &vendor_id, const std::string &product_id, bool rule_match, quint32 rule_seqn);
    void IPCConnected();
    void IPCDisconnected();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *systray;
    QTimer _flash_timer;
    bool _flash_state;
    QTimer _ipc_timer;
};

#endif // MAINWINDOW_H

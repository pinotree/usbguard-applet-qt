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
    void uiDeviceInserted(quint32 seqn, const std::map<std::string, std::string>& attributes, const std::vector<usbguard::USBInterfaceType>& interfaces, bool rule_match);
    void uiDevicePresent(quint32 seqn, const std::map<std::string, std::string>& attributes, const std::vector<usbguard::USBInterfaceType>& interfaces, usbguard::Rule::Target target);
    void uiDeviceRemoved(quint32 seqn, const std::map<std::string, std::string>& attributes);
    void uiDeviceAllowed(quint32 seqn, const std::map<std::string, std::string>& attributes);
    void uiDeviceBlocked(quint32 seqn, const std::map<std::string, std::string>& attributes);
    void uiDeviceRejected(quint32 seqn, const std::map<std::string, std::string>& attributes);
    void uiConnected();
    void uiDisconnected();

protected slots:
    void switchVisibilityState(QSystemTrayIcon::ActivationReason reason);
    void flashStep();
    void ipcTryConnect();

    void showDeviceDialog(quint32 seqn, const std::map<std::string, std::string>& attributes, const std::vector<usbguard::USBInterfaceType>& interfaces, bool rule_match);
    void showMessage(const QString &message, bool alert = false);

    void notifyInserted(quint32 seqn, const std::map<std::string, std::string>& attributes, const std::vector<usbguard::USBInterfaceType>& interfaces, bool rule_matched);
    void notifyPresent(quint32 seqn, const std::map<std::string, std::string>& attributes, const std::vector<usbguard::USBInterfaceType>& interfaces, usbguard::Rule::Target target);
    void notifyRemoved(quint32 seqn, const std::map<std::string, std::string>& attributes);
    
    void notifyAllowed(quint32 seqn, const std::map<std::string, std::string>& attributes);
    void notifyBlocked(quint32 seqn, const std::map<std::string, std::string>& attributes);
    void notifyRejected(quint32 seqn, const std::map<std::string, std::string>& attributes);

    void notifyIPCConnected();
    void notifyIPCDisconnected();

    void allowDevice(quint32 seqn, bool append);
    void blockDevice(quint32 seqn, bool append);
    void rejectDevice(quint32 seqn, bool append);

    void handleIPCConnect();
    void handleIPCDisconnect();

protected:
    void changeEvent(QEvent *e);
    void setupSystemTray();
    void startFlashing();
    void stopFlashing();

    void DeviceInserted(quint32 seqn, const std::map<std::string, std::string>& attributes, const std::vector<usbguard::USBInterfaceType>& interfaces, bool rule_match, quint32 rule_seqn);
    void DevicePresent(quint32 seqn, const std::map<std::string, std::string>& attributes, const std::vector<usbguard::USBInterfaceType>& interfaces, usbguard::Rule::Target target);
    void DeviceRemoved(quint32 seqn, const std::map<std::string, std::string>& attributes);

    void DeviceAllowed(quint32 seqn, const std::map<std::string, std::string>& attributes, bool rule_match, quint32 rule_seqn);
    void DeviceBlocked(quint32 seqn, const std::map<std::string, std::string>& attributes, bool rule_match, quint32 rule_seqn);
    void DeviceRejected(quint32 seqn, const std::map<std::string ,std::string>& attributes, bool rule_match, quint32 rule_seqn);

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

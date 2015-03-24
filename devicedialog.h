#ifndef DEVICEDIALOG_H
#define DEVICEDIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class DeviceDialog;
}

class DeviceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceDialog(quint32 seqn, const QString& name, const QString& rule, QWidget *parent = 0);
    ~DeviceDialog();

signals:
    void allowed(quint32 seqn, bool permanent);
    void blocked(quint32 seqn, bool permanent);
    void rejected(quint32 seqn, bool permanent);

protected slots:
    void timerUpdate();

protected:
    void reject();
    void accept();
    void updateDialog();
    void executeDefaultDecision();
private slots:
    void on_allow_button_clicked();
    void on_block_button_clicked();
    void on_reject_button_clicked();
    void on_timeout_checkbox_toggled(bool checked);
    void on_permanent_checkbox_toggled(bool checked);

private:
    Ui::DeviceDialog *ui;
    QTimer timer;
    int time_left;
    quint32 device_seqn;
};

#endif // DEVICEDIALOG_H

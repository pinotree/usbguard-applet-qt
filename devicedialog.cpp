#include "devicedialog.h"
#include "ui_devicedialog.h"

#include <QStyle>
#include <QDesktopWidget>

DeviceDialog::DeviceDialog(quint32 seqn, const QString &name, const QString &rule, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceDialog)
{
    ui->setupUi(this);
    ui->name_label->setText(name);
    setWindowTitle(QString("USB Device Inserted: %1").arg(name));
    setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowStaysOnTopHint);
    time_left = 23;
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    timer.start(1000);
    updateDialog();
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
                                    size(),
                                    qApp->desktop()->availableGeometry()));
    ui->deny_button->setFocus();
    device_seqn = seqn;

#if 1
    /* Hide things which aren't working yet */
    ui->permanent_checkbox->hide();
    ui->timeout_checkbox->hide();
    ui->timeout_combobox->hide();
    ui->timeout_lineedit->hide();
#endif
}

void DeviceDialog::timerUpdate()
{
    if (time_left > 0) {
        --time_left;
        updateDialog();
    } else {
        timer.stop();
        executeDefaultDecision();
    }
}

void DeviceDialog::reject()
{
    if (timer.isActive()) {
        timer.stop();
        updateDialog();
    } else {
        QDialog::reject();
    }
}

void DeviceDialog::accept()
{
    if (timer.isActive()) {
        timer.stop();
    }
    QDialog::accept();
}

void DeviceDialog::updateDialog()
{
    if (timer.isActive()) {
        ui->deny_button->setText(QString("Block [%1]").arg(time_left));
    } else {
        ui->deny_button->setText(QString("Block"));
        ui->hint_label->setText("(Press Escape to close this window)");
    }
}

void DeviceDialog::executeDefaultDecision()
{
    on_deny_button_clicked();
    return;
}

DeviceDialog::~DeviceDialog()
{
    delete ui;
}

void DeviceDialog::on_allow_button_clicked()
{
    emit allowed(device_seqn, ui->permanent_checkbox->isChecked());
    accept();
}

void DeviceDialog::on_deny_button_clicked()
{
    emit denied(device_seqn, ui->permanent_checkbox->isChecked());
    accept();
}

void DeviceDialog::on_reject_button_clicked()
{
    emit rejected(device_seqn, ui->permanent_checkbox->isChecked());
    accept();
}

void DeviceDialog::on_timeout_checkbox_toggled(bool checked)
{
    if (checked) {
        ui->permanent_checkbox->setChecked(false);
    }
}

void DeviceDialog::on_permanent_checkbox_toggled(bool checked)
{
    if (checked) {
        ui->timeout_checkbox->setChecked(false);
    }
}

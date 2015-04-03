#include "devicedialog.h"
#include "ui_devicedialog.h"

#include <QStyle>
#include <QDesktopWidget>

DeviceDialog::DeviceDialog(quint32 seqn, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceDialog)
{
    ui->setupUi(this);

    setWindowTitle(QString("USB Device Inserted"));
    setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowStaysOnTopHint);

    device_seqn = seqn;

    time_left = 23;
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    timer.start(1000);

    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
                                    size(),
                                    qApp->desktop()->availableGeometry()));
    updateDialog();

    ui->block_button->setFocus();
#if 1
    /* Hide things which aren't working yet */
    ui->timeout_checkbox->hide();
    ui->timeout_combobox->hide();
    ui->timeout_lineedit->hide();
#endif
}

void DeviceDialog::setName(const QString& name)
{
    ui->name_label->setText(name);
    return;
}

void DeviceDialog::setDeviceID(const QString& vendor_id, const QString& product_id)
{
    ui->deviceid_label->setText(QString("%1:%2").arg(vendor_id).arg(product_id));
    return;
}

void DeviceDialog::setSerial(const QString &serial)
{
    ui->serial_label->setText(serial);
    return;
}

void DeviceDialog::setInterfaceTypes(const std::vector<usbguard::USBInterfaceType>& interfaces)
{
  ui->interface_list->clear();
  for (auto const& type : interfaces) {
      ui->interface_list->addItem(QString::fromStdString(type.typeString()));
  }
  return;
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
        ui->block_button->setText(QString("Block [%1]").arg(time_left));
    } else {
        ui->block_button->setText(QString("Block"));
        ui->hint_label->setText("(Press Escape to close this window)");
    }
}

void DeviceDialog::executeDefaultDecision()
{
    on_block_button_clicked();
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

void DeviceDialog::on_block_button_clicked()
{
    emit blocked(device_seqn, ui->permanent_checkbox->isChecked());
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

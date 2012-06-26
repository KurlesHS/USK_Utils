#include "selectpacketdialog.h"
#include "ui_selectpacketdialog.h"

SelectPacketDialog::SelectPacketDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectPacketDialog)
{
    ui->setupUi(this);
    lastWidget = 0;

    MyWidget::setUtf8();
    ui->listWidget->addItem("Время");
    ui->listWidget->addItem("Сообщение");
    ui->listWidget->addItem("Передача данных на КПУ");
    setWindowTitle("Добавление пакета");
    MyWidget::setWin1251();
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SIGNAL(cancelSendPacketButtonPushed()));
    connect(ui->pushButtonSend, SIGNAL(clicked()), this, SLOT(onSendPacketButtonPushed()));
    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), SLOT(onCurrenPacketTypeChanged(int)));
    QSettings setting(QString("USKTerminal.ini"),  QSettings::IniFormat);
    setting.beginGroup("SelectPacketDialog");
    if (setting.contains("numUSK"))
        ui->spinBox->setValue(setting.value("numUSK").toInt());
    if (setting.contains("currentPacket"))
    {
        int row = setting.value("currentPacket").toInt();
        if (row < ui->listWidget->count())
            ui->listWidget->setCurrentRow(row);
        else
            ui->listWidget->setCurrentRow(0);
    } else
        ui->listWidget->setCurrentRow(0);
    setting.endGroup();
}

SelectPacketDialog::~SelectPacketDialog()
{
    QSettings setting(QString("USKTerminal.ini"),  QSettings::IniFormat);
    setting.beginGroup("SelectPacketDialog");
    setting.setValue("numUSK", ui->spinBox->value());
    setting.setValue("currentPacket", ui->listWidget->currentRow());
    setting.endGroup();
    delete ui;
}

void SelectPacketDialog::onCurrenPacketTypeChanged(int index)
{
    if (lastWidget){
        ui->horizontalLayoutForWidgets->removeWidget(lastWidget);
        delete lastWidget;
        lastWidget = 0;
    }

    switch (index){
    case 0:
        lastWidget = new USKTimePacket(this);

        break;
    case 1:
        lastWidget = new USKMessagePacket(this);
        break;
    case 2:
        lastWidget = new USKKPUData(this);
        break;
    }
    if (lastWidget){
        ui->horizontalLayoutForWidgets->addWidget(lastWidget);
        connect(lastWidget, SIGNAL(okButtonPushed()), this, SLOT(accept()));
    }
}

void SelectPacketDialog::onSendPacketButtonPushed()
{
    if (!lastWidget) return;
    emit sendPacketButtonPushed(getData());
}

QByteArray SelectPacketDialog::getData(){
    if (!lastWidget) return QByteArray();
    QByteArray res;
    res.append(MyWidget::prepareComplexNumber(ui->spinBox->value()));
    res.append(lastWidget->getData());
    return res;
}

void SelectPacketDialog::setPortIsOpen(bool isOpen)
{
    ui->pushButtonCancel->setEnabled(isOpen);
    ui->pushButtonSend->setEnabled(isOpen);
}

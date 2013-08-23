#include "uskmessagepacket.h"
#include "ui_uskmessagepacket.h"
#include <QSettings>

USKMessagePacket::USKMessagePacket(QWidget *parent) :
    MyWidget(parent),
    ui(new Ui::USKMessagePacket)
{
    ui->setupUi(this);
    setUtf8();
    ui->comboBox->addItem("16 байт");
    ui->comboBox->addItem("32 байта");
    setWin1251();
    QSettings setting(QString("USKTerminal.ini"),  QSettings::IniFormat);
    setting.beginGroup("MessagePacket");
    if (setting.contains("text"))
        ui->lineEdit->setText(setting.value("text").toString());
    if (setting.contains("MessageType"))
    {
        int index = setting.value("MessageType").toInt();
        if (index < ui->comboBox->count())
            ui->comboBox->setCurrentIndex(index);
    }
    setting.endGroup();

}

USKMessagePacket::~USKMessagePacket()
{
    QSettings setting(QString("USKTerminal.ini"),  QSettings::IniFormat);
    setting.setValue("MessagePacket/text", ui->lineEdit->text());
    setting.setValue("MessagePacket/MessageType", ui->comboBox->currentIndex());
    delete ui;
}

QByteArray USKMessagePacket::getData()
{
    QByteArray array;
    quint8 flags;
    switch (ui->comboBox->currentIndex()){
    case 0:
        flags = 0x10; //00010000b;
        break;
    case 1:
        flags = 0x08; //00001000b;
        break;
    case 2:
        flags = 0x18; //00011000b;
        break;
    default:
        flags = 0x10;
    }
    array.append(flags);
    int lenmessage = ui->lineEdit->text().count();
    if (ui->comboBox->currentIndex() == 0){
        char *buff = new char[0x10];
        for (int i=0; i<0x10; ++i)
            *(buff+i) = 0x20;
        if (lenmessage > 0x10) lenmessage = 0x10;
        memcpy(buff, ui->lineEdit->text().toAscii().data(),lenmessage);
        array.append(buff,0x10);
        delete buff;
    }

    if (ui->comboBox->currentIndex() == 1){
        char *buff = new char[0x20];
        for (int i=0; i<0x20; ++i)
            *(buff+i) = 0x20;
        if (lenmessage > 0x20) lenmessage = 0x20;
        memcpy(buff, ui->lineEdit->text().toAscii().data(),lenmessage);
        array.append(buff,0x20);
        delete buff;
    }

    if (ui->comboBox->currentIndex() == 2){
        array.append(prepareComplexNumber(lenmessage));
        array.append(ui->lineEdit->text().toAscii());
    }
    //array.append(char(0));
    return array;
}


void USKMessagePacket::on_pushButton_clicked()
{
    emit okButtonPushed();
}

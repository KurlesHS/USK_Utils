#include "usktimepacket.h"
#include "ui_usktimepacket.h"

USKTimePacket::USKTimePacket(QWidget *parent) :
    MyWidget(parent),
    ui(new Ui::USKTimePacket)
{
    ui->setupUi(this);
    ui->timeEdit->setTime(QTime::currentTime());
}

USKTimePacket::~USKTimePacket()
{
    delete ui;
}

void USKTimePacket::on_pushButtonCurrentTime_clicked()
{
    ui->timeEdit->setTime(QTime::currentTime());
}

QByteArray USKTimePacket::getData()
{
    QByteArray array;
    //array.append((char)0x05); //установлены флаги F4 и F6
    array.append((char)0x40); //установлен флаг F1
    quint8 l,h;
    QTime time = ui->timeEdit->time();
    l = time.second()%10;
    h = time.second()/10;
    h = h << 4;
    l |= h;
    array.append(l); //секунды

    l = time.minute()%10;
    h = time.minute()/10;
    h = h << 4;
    l |= h;
    array.append(l); //минуты

    l = time.hour()%10;
    h = time.hour()/10;
    h = h << 4;
    l |= h;
    array.append(l); //часы

    //array.append((char)0x00); //0 повторов
    return array;

}

void USKTimePacket::on_pushButtonOK_clicked()
{
    emit okButtonPushed();
}

#include "usktimepacket.h"
#include "ui_usktimepacket.h"
#include <QDebug>

USKTimePacket::USKTimePacket(QWidget *parent) :
    MyWidget(parent),
    ui(new Ui::USKTimePacket)
{
    ui->setupUi(this);
    on_pushButtonCurrentTime_clicked();
}

USKTimePacket::~USKTimePacket()
{
    delete ui;
}

void USKTimePacket::on_pushButtonCurrentTime_clicked()
{
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
}

QByteArray USKTimePacket::getData()
{
    QByteArray array;
    //array.append((char)0x05); //установлены флаги F4 и F6
    //array.append((char)0x40); //установлен флаг F1
    array.append((char)0x44); //установлены флаги F1 и F5
    array.append(0x07); //7 байт данных
    quint8 l,h;
    QTime time = ui->dateTimeEdit->time();
    QDate date = ui->dateTimeEdit->date();
    l = (date.year() % 100) % 10;
    h = (date.year() % 100) / 10;
    h = h << 4;
    l |= h;
    array.append(l);

    l = date.month() % 10;
    h = date.month() / 10;
    h = h << 4;
    l |= h;
    array.append(l);

    l = date.day() % 10;
    h = date.day() / 10;
    h = h << 4;
    l |= h;
    array.append(l);

    l = date.dayOfWeek();
    array.append(l);

    l = time.hour()%10;
    h = time.hour()/10;
    h = h << 4;
    l |= h;
    array.append(l); //часы

    l = time.minute()%10;
    h = time.minute()/10;
    h = h << 4;
    l |= h;
    array.append(l); //минуты

    l = time.second()%10;
    h = time.second()/10;
    h = h << 4;
    l |= h;
    array.append(l); //секунды

    //array.append((char)0x00); //0 повторов
    return array;
}

void USKTimePacket::on_pushButtonOK_clicked()
{
    emit okButtonPushed();
}

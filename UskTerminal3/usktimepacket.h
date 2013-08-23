#ifndef USKTIMEPACKET_H
#define USKTIMEPACKET_H

#include "mywidget.h"
#include <QWidget>
#include <QTime>
#include <QByteArray>

namespace Ui {
    class USKTimePacket;
}

class USKTimePacket : public MyWidget
{
    Q_OBJECT

public:
    explicit USKTimePacket(QWidget *parent = 0);
    ~USKTimePacket();
    QByteArray getData();

signals:
    void okButtonPushed();

private slots:
    void on_pushButtonCurrentTime_clicked();

    void on_pushButtonOK_clicked();

private:
    Ui::USKTimePacket *ui;
};

#endif // USKTIMEPACKET_H

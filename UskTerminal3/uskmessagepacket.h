#ifndef USKMESSAGEPACKET_H
#define USKMESSAGEPACKET_H

#include "mywidget.h"
#include <QWidget>
#include <QTextCodec>
#include <QByteArray>

namespace Ui {
    class USKMessagePacket;
}

class USKMessagePacket : public MyWidget
{
    Q_OBJECT

public:
    explicit USKMessagePacket(QWidget *parent = 0);
    ~USKMessagePacket();
    QByteArray getData();


signals:
    void okButtonPushed();

private slots:
    void on_pushButton_clicked();

private:
    Ui::USKMessagePacket *ui;
};

#endif // USKMESSAGEPACKET_H

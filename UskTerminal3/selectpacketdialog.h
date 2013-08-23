#ifndef SELECTPACKETDIALOG_H
#define SELECTPACKETDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QByteArray>

#include "mywidget.h"
#include "usktimepacket.h"
#include "uskmessagepacket.h"
#include "uskkpudata.h"

namespace Ui {
    class SelectPacketDialog;
}

class SelectPacketDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectPacketDialog(QWidget *parent = 0);
    ~SelectPacketDialog();
    QByteArray getData();
    void setPortIsOpen(bool isOpen);

signals:
    void cancelSendPacketButtonPushed();
    void sendPacketButtonPushed(QByteArray packet);

private slots:
    void onCurrenPacketTypeChanged(int index);
    void onSendPacketButtonPushed();

private:
    Ui::SelectPacketDialog *ui;
    MyWidget *lastWidget;
};

#endif // SELECTPACKETDIALOG_H

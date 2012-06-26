#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QLineEdit>
#include <QMessageBox>
#include <QCoreApplication>
#include <QQueue>
#include <QTextCodec>
#include <QByteArray>
#include <QFontMetrics>
#include <QTimer>
#include "mylineedit.h"
#include "abstractserial.h"
#include "serialdeviceenumerator.h"
#include "serialthread.h"
#include "selectpacketdialog.h"
#include "packetdecoder.h"
#include "packetinfowindows.h"

#define ColumnCount 0x100

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void updateDec(int column, int num);
    void updateHex(int column, int num);
    void updateBin(int column, int num);
    void updateChar(int column, int num);
    void updateCRC();
    void updateSendRegion();
    void setWin1251();
    void setUft8();
    void setGUIRtsCts(bool rts, bool cts);
    QString getHexString(QByteArray array);
    QString getDecString(QByteArray array);
    QString getBinString(QByteArray array);
    QString getCharString(QByteArray array);
    void fillSendRegionFromArray(QByteArray array);
    void addLogMessage(QString message, int type = 0);

private:
    Ui::MainWindow *ui;
    AbstractSerial *serialPort;
    SerialThread *serialThread;
    QQueue<char> queue;
    QByteArray array;
    QByteArray resRegArray;
    QTimer timer;
    PacketDecoder packetDecoder;
    PacketInfoWindows *packetInfoWindows;

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void onCellChanged(int row, int column);
    void onCellEdited(QString str);
    void onRtsChanged(bool status);
    void onTimerEvent();
    void onFocusChanged();
    void onIncludeCRCCheckBoxChanged();
    void OnTimeOutReached();
    void onOpenPortButtonPushed();
    void onClosePortButtonPushed();
    void onSendPacketButtonPushed();
    void onSendPacketButtonPushed(QByteArray packet);
    void onCancelSendPacketButtonPushed();
    void onClearSendRegionButtonPushed();
    void onClearReceiveRegionButtonPushed();
    void on_pushButtonPacket_clicked();
    void onEndTransmitPacket();
    void onTerminalModeChanged(bool terminalModeSelected);
    void onPacketInfoWindowsDestroyed();
    void onMenuViewAboutToShow();
    void onShowPacketInfoWindowMenu();
    void onEnableRepeatRacketButtonPushed();
    void onValueOfDelayBetweenSendPacketChanged(int delay);
    void onWrongResponseReceived(QByteArray response);
    void onDataReceived(QByteArray array);
    void onResponseReceived(QByteArray array);

};

#endif // MAINWINDOW_H

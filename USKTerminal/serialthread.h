#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QThread>
#include <QByteArray>
#include <QTime>
#include "abstractserial.h"
#include <QTcpSocket>

class SerialThread : public QThread
{
    Q_OBJECT
public:
    enum {waitData, continueWaitData, sendData, waitResponse, sendResponse, waitDecisionFromExternClass};
    explicit SerialThread(QObject *parent = 0);
    ~SerialThread();
    void run();
    void sendPacket(QByteArray array);
    void sendRespons(QByteArray array);
    void setSerialPortDriver(QIODevice *serial);
    void setRepeatPacketCount(int value);
    void setPacketDecoderMode(bool packerDecoderMode);
    void setDelayBetweenSendPacket(int delayInSec);
    void setUseRepeatPacket(bool use);
    void continueWaitingData();
    void continueWaitingResponse();
    void onErrorWhileReceiveResponse();
    void startWaitingData();
    void startWaitingDataWithDelay(int delay);
    void onCorrectResponseReceived();
    void onIncorrectResponseReceived();
    void continueSendPacket();
    int getCurrentSMStatus() {return status;}

private:

signals:
    void dataReceived(QByteArray array);
    void responseReceived(QByteArray array);
    void timeOutHappensWhileSendData();
    void timeOutHappensWhileSendResponse();
    void timeOutHappensWhileWaitingResponse();
    void timeOutHappensWhileWaitingData();
    void transmitionEnded();
    void errorWhileReceiveResponse(QByteArray response);
    void rtsChanged(bool status);
    void sendPacketSuccess();

public slots:

private:
    QIODevice *serialPort;
    QTcpSocket *socket;
    QByteArray array, tempArray;
    QByteArray incommingArray;
    QTime currentTime;
    int status;
    bool usePacketRepeat;
    bool usePackerDecoderMode;
    int repeatCount;
    int delayBetweenSendPacket;
    qint8 tryCount;


};

#endif // SERIALTHREAD_H

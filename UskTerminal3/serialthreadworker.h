#ifndef SERIALTHREADWORKER_H
#define SERIALTHREADWORKER_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QList>
#include <abstractserial.h>
class SerialThreadWorker : public QObject
{
    Q_OBJECT

    enum states {
        waitDataState,
        waitResponseState,
        waitDecisionFromExternalClassState,
        timeoutOnWaitDataState,
        timeoutOnWaitResponseState
    };

    struct Packet {
        QByteArray packet;
        QByteArray desctiption;
        int numCount;
    };

public:
    explicit SerialThreadWorker(QObject *parent = 0);
    ~SerialThreadWorker();

public Q_SLOTS:
    void sendPacket(const QByteArray &packet, const QString &description, const int &numberOfTryingToSend);
    void sendResponse(const QByteArray &response);

private Q_SLOTS:
    void onReadyRead();
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
    void onSocketError(QAbstractSocket::SocketError socketError);
    
signals:
    
public slots:

private:
    QTcpSocket *m_socket;
    AbstractSerial *m_serialPort;
    QTimer m_timerForTimeouts;
    states m_currentState;
    QList<Packet> m_listOfPacketToSend;
};

#endif // SERIALTHREADWORKER_H

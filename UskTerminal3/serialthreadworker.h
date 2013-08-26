#ifndef SERIALTHREADWORKER_H
#define SERIALTHREADWORKER_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QList>
#include "abstractserial.h"
#include "packetdecoder.h"
class SerialThreadWorker : public QObject
{
    Q_OBJECT

    enum states {
        waitDataState,
        continueWaitDataState,
        waitResponseState,
        continueWaitResponseState,
        waitDecisionFromExternalClassState,
        timeoutOnWaitDataState,
        timeoutOnWaitResponseState
    };



    struct Packet {
        QByteArray packet;
        QString desctiption;
        int numCount;
    };

public:

    enum modes {
        terminalMode,
        packetMode
    };

    enum connectionType {
        noneConnectionType,
        serialConnectionType,
        ethernetConnectionType
    };

public:
    explicit SerialThreadWorker(QObject *parent = 0);
    ~SerialThreadWorker();

private:
    void parseIncomingPacket(QIODevice * const device);
    void parseIncomingResponse(QIODevice * const device);
    QString getCharString(QByteArray array);
    QString getBinString(QByteArray array);
    QString getDecString(QByteArray array);
    QString getHexString(QByteArray array);

public Q_SLOTS:
    void setCurrentMode(int mode);
    void setDelayBetweenPacket(int ms);
    void sendPacket(const QByteArray &packet, const QString &description, const int &numberOfTryingToSend = 2);
    void sendResponse(const QByteArray &response);
    void openSerialPort(const QString &portName, const QString &baudrate, const QString &flowcontrol, const QString stopbits, const QString databits, const QString &parity);
    void openEthernet(const QString &addr, const int &port);
    bool isConnectedToUsk();
    void closeConnection();

private Q_SLOTS:
    void onReadyRead();
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
    void onSocketError(QAbstractSocket::SocketError socketError);
    void onTimeout();
    void onCheckPacket();
    
    
public slots:

signals:
    void connectedToUsk(bool connected, QString text);
    void dataArrived(const QByteArray &chunkData);
    void logMessage (const QString &message);
    void logMessageForParserWindows(const QString &message, const QString &color);


private:
    modes m_currentMode;
    connectionType m_currnetConnectionType;
    int m_delayBetweenPacket;
    int m_timeoutInMs;
    QTcpSocket *m_socket;
    AbstractSerial *m_serialPort;
    QTimer m_timerForTimeouts;
    PacketDecoder m_packetDecoder;
    QTimer m_timtrForCheckPackets;
    states m_currentState;
    Packet m_currentPacket;
    QByteArray m_currentDataChunk;
    QList<Packet> m_listOfPacketToSend;
};

#endif // SERIALTHREADWORKER_H

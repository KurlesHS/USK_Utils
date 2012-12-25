#ifndef SENDUSKV1WORKINGTHREAD_H
#define SENDUSKV1WORKINGTHREAD_H

#include <QObject>
#include <QHash>
#include <QDateTime>
#include <QTextCodec>
#include <QtAddOnSerialPort/serialport.h>
#include <QtAddOnSerialPort/serialportinfo.h>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

QT_USE_NAMESPACE_SERIALPORT

class SendUSKv1WorkingThread : public QObject
{
    Q_OBJECT


    enum uskState
    {
        uskWaitData,
        uskWaitResponse,
        uskWaitPeriod,
        uskWaitAfterSendCommand,
        uskWaitResendPrevCommand
    };

    struct uskPacket
    {
        QString description;
        QByteArray packet;
        bool waitResponse;
    };

    struct uskInfo
    {
        QString name;
        QString port;
        SerialPort *serialPort;
        QTimer *timer;
        QByteArray *buffer;
        uskState uskStatus;
        int num;
        bool isOpen;
        bool isBusy;
        bool isFaultly;
        bool isPresent;
        int numberOfAttemps;
        int currentNumberOfAttemps;
        QByteArray prevPacket;
        QString prevPacketDescription;
        bool prevPacketWaitResponse;
        QList <uskPacket> listOfPacketToSend;
    };

public:
    enum errorCodes
    {
        errorOpenPort,
        errorTimeoutWhileWaitData,
        errorTimeoutWhileWaitResponse,
        errorUskIsntResponse,
        errorUskInstPresent,
        errorUskWrongPacket
    };

    enum uskInfoPackets
    {
        packetUskReset,
        packetUskSettingTime,
        packetUskOn,
        packetUskErrorReceivingRS
    };

    explicit SendUSKv1WorkingThread(QObject *parent = 0);

public slots:
    bool addUsk(const QString &uskName, const QString &portName, int uskNum);
    void removeUsk(const QString &uskName);
    void openUsk(const QString &uskName);
    void closeUsk(const QString &uskName);
    void sendTime(const QString &uskName, const QDateTime &time);
    void resetUsk(const QString &uskName);
    void changeRelayStatus(const QString &uskName, const int &rayNum, const int &kpuNum, const int &sensorNum, const int &relayStatus);

signals:
    void error(const QString &uskName, int errorCode);
    void unknowCommand(const QString &uskName, const QString &command);
    void errorOnSendingCommand(const QString &uskName, const QString &commandDescription);
    void uskInfoPacketReceived(const QString &uskName, const int &infoPacket);
    void portIsOpen(const QString &uskName, const QString &portName);
    void portIsClose(const QString &uskName, const QString &portName);
    void uskReset(const QString &uskName);
    void uskIsPresent(const QString &uskName, const bool &present);
    void startSendingCommand(const QString &uskName, const QString &commandDescription);
    void detectedNewKpu(const QString &uskName, const int &rayNum, const int &kpuNum);
    void detectedDisconnetcedKpu(const QString &uskName, const int &rayNum, const int &kpuNum);
    void sensorChanged(const QString &uskName, const int &rayNum, const int &kpuNum, const int &sensorNum, const int &state);


private:
    QByteArray getSetTimePacket(const uskInfo *const ui, const QDateTime &time);
    QByteArray getResetUskPacket(const uskInfo *const ui);
    QByteArray getChangeKpuRelayStatePacket(const uskInfo *const ui, const int &rayNum, const int &kpuNum, const int &sensorNum, const int &state);
    void appendCrcToPacket(QByteArray &packet);
    QByteArray getFirstPartOfPacket(const ushort &uskNum, const quint64 &flags, const quint8 &priority);

private slots:
    void onReadyRead();
    void onTimer();
    void onTimer2();
    void onCheckBuffersTimer();

private:
    QHash<QString, uskInfo> m_hashOfUsk;
    static QTextCodec *m_codecWin1251;
    QTimer *m_checkBuffersTimer;
    
};

#endif // SENDUSKV1WORKINGTHREAD_H

#ifndef SENDUSKV1WORKINGTHREAD_H
#define SENDUSKV1WORKINGTHREAD_H

#include <QObject>
#include <QHash>
#include <QDateTime>
#include <QtAddOnSerialPort/serialport.h>
#include <QtAddOnSerialPort/serialportinfo.h>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

QT_USE_NAMESPACE_SERIALPORT

struct uskInfo
{
    QString name;
    QString port;
    SerialPort *serialPort;
    QTimer *timer;
    int num;
    bool isOpen;
    bool isBusy;
    bool isFaultly;
    bool isPresent;
};

enum errorCodes
{
    errorOpenPort,
    errorTimeout,
    errorUskIsntResponse,
    errorUskInstPresent
};

class SendUSKv1WorkingThread : public QObject
{
    Q_OBJECT
public:
    explicit SendUSKv1WorkingThread(QObject *parent = 0);

public slots:
    bool addUsk(const QString &uskName, const QString &portName, int uskNum);
    void openUsk(const QString &uskName);
    void closeUsk(const QString &uskName);
    void sendTime(const QString &uskName, const QDateTime &time);

signals:
    void error(const QString &uskName, int errorCode);
    void portIsOpen(const QString &uskName, const QString &portName);
    void uskReset(const QString &uskName);

private:
    QByteArray getSetTimePacket(const uskInfo *const ui, const QDateTime &time);
    QByteArray getResetUskPacket(const uskInfo *const ui);
    void appendCrcToPacket(QByteArray &packet);

private slots:
    void onReadyRead();

private:
    QHash<QString, uskInfo> m_hashOfUsk;
    
};

#endif // SENDUSKV1WORKINGTHREAD_H

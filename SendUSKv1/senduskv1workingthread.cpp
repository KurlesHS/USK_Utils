#include "senduskv1workingthread.h"
#include <QVariant>
#include <QTimer>
#include <QDebug>
SendUSKv1WorkingThread::SendUSKv1WorkingThread(QObject *parent) :
    QObject(parent)
{
}

bool SendUSKv1WorkingThread::addUsk(const QString &uskName, const QString &portName, int uskNum)
{
    // пустое имя не допустимо
    if (uskName.trimmed().isEmpty())
        return false;
    if (m_hashOfUsk.contains(uskName))
        return false;
    foreach (const uskInfo &ui, m_hashOfUsk)
    {
        if (ui.num == uskNum)
            return false;
    }
    uskInfo ui;
    ui.isBusy = true;
    ui.port = portName;
    ui.isFaultly = false;
    ui.isOpen = false;
    ui.isPresent = false;
    ui.name = uskName;
    ui.num = uskNum;
    ui.timer = new QTimer(this);
    ui.timer->setSingleShot(true);
    ui.serialPort = new SerialPort(portName, this);
    ui.serialPort->setProperty("uskName", uskName);
    m_hashOfUsk[uskName] = ui;
    connect(ui.serialPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    return true;
}

void SendUSKv1WorkingThread::openUsk(const QString &uskName)
{
    if (!m_hashOfUsk.contains(uskName))
    {
        emit error(uskName, errorUskIsntResponse);
        return;
    }
    uskInfo *pUskInfo = &m_hashOfUsk[uskName];
    if (pUskInfo->isOpen)
        return;
    pUskInfo->serialPort->setPort(pUskInfo->port);
    if (pUskInfo->serialPort->open(SerialPort::ReadWrite))
    {
        if (pUskInfo->serialPort->setDataBits(SerialPort::Data8) &&
                pUskInfo->serialPort->setRate(9600) &&
                pUskInfo->serialPort->setStopBits(SerialPort::OneStop) &&
                pUskInfo->serialPort->setFlowControl(SerialPort::NoFlowControl) &&
                pUskInfo->serialPort->setParity(SerialPort::NoParity))
        {
            emit portIsOpen(uskName, pUskInfo->port);
            //pUskInfo->serialPort->write(getSetTimePacket(pUskInfo, QDateTime::currentDateTime()));
            pUskInfo->serialPort->write(getResetUskPacket(pUskInfo));
            //TODO: проверка наличия УСК (установка времени)
        }
    } else
    {
        emit error(uskName, errorOpenPort);
    }
}

void SendUSKv1WorkingThread::closeUsk(const QString &uskName)
{
}

void SendUSKv1WorkingThread::sendTime(const QString &uskName, const QDateTime &time)
{

}



void SendUSKv1WorkingThread::onReadyRead()
{
    SerialPort *sp = qobject_cast<SerialPort*>(sender());
    if (!sp)
        return;
    QString uskName = sp->property("uskName").toString();
    // пустое имя - не допустимо;
    if (uskName.isEmpty())
        return;
    if (m_hashOfUsk.contains(uskName))
    {
        // есть такое УСК
        uskInfo *pUskInfo = &m_hashOfUsk[uskName];
        pUskInfo->isBusy = true;

    }
}


QByteArray SendUSKv1WorkingThread::getSetTimePacket(const uskInfo *const ui, const QDateTime &time)
{
    QByteArray res;
    res.append(static_cast<char>(0x00));
    res.append(static_cast<char>(ui->num));
    res.append(static_cast<char>(0x00));
    res.append(static_cast<char>(0x02));
    res.append(static_cast<char>(0x00));
    res.append(static_cast<char>(0x00));
    res.append(static_cast<char>(0x00));
    res.append(static_cast<char>(0x00));
    QString day = QString("%0").arg(time.date().day(), 2, 10, QChar('0'));
    QString month = QString("%0").arg(time.date().month(), 2, 10, QChar('0'));
    QString year = QString::number(time.date().year() % 10);
    QString hour = QString("%0").arg(time.time().hour(), 2, 10, QChar('0'));
    QString minute = QString("%0").arg(time.time().minute(), 2, 10, QChar('0'));
    QString second = QString("%0").arg(time.time().second(), 2, 10, QChar('0'));

    QString stringDate;
    stringDate.append(day).append('/').append(month).append('/').append(year).append(' ')
            .append(hour).append(':').append(minute).append(':').append(second);
    res.append(stringDate);
    qDebug() << "strdate" << stringDate;
    res.append(static_cast<char>(0x00));
    res.append(static_cast<char>(0x00));
    appendCrcToPacket(res);
    qDebug() << day << month << year << hour << minute << second;
    qDebug() << res.length() << ui->num << (int)res.at(res.length() - 1) << res;
    return res;
}

QByteArray SendUSKv1WorkingThread::getResetUskPacket(const uskInfo *const ui)
{
    QByteArray res;
    res.resize(26);
    res.fill(static_cast<char>(0x00));
    res[1] = static_cast<char>(ui->num);
    res[3] = 0x01;
    appendCrcToPacket(res);
    return res;
}

void SendUSKv1WorkingThread::appendCrcToPacket(QByteArray &packet)
{
    char crc = 0x00;
    for (int i = 0; i < packet.length(); ++i)
        crc += packet.at(i);
    packet.append(crc);
}

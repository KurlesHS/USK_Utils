#include "senduskv1workingthread.h"
#include <QVariant>
#include <QTimer>
#include <QDebug>

QTextCodec *SendUSKv1WorkingThread::m_codecWin1251 = QTextCodec::codecForName("Windows-1251");

SendUSKv1WorkingThread::SendUSKv1WorkingThread(QObject *parent) :
    QObject(parent)
{
    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(onTimer2()));
    //t->start(1000);

    m_checkBuffersTimer = new QTimer();
    connect(m_checkBuffersTimer, SIGNAL(timeout()), this, SLOT(onCheckBuffersTimer()));
    // период проверки выходных буферов
    m_checkBuffersTimer->start(200);

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
    ui.uskStatus = uskWaitData;
    ui.isBusy = true;
    ui.port = portName;
    ui.isFaultly = false;
    ui.isOpen = false;
    ui.isPresent = false;
    ui.name = uskName;
    ui.num = uskNum;
    ui.buffer = new QByteArray();
    ui.timer = new QTimer(this);
    ui.timer->setSingleShot(true);
    ui.timer->setProperty("uskName", uskName);
    ui.numberOfAttemps = 3;
    ui.serialPort = new QSerialPort(portName, this);
    ui.serialPort->setProperty("uskName", uskName);
    m_hashOfUsk[uskName] = ui;
    connect(ui.serialPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(ui.timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    return true;
}

void SendUSKv1WorkingThread::removeUsk(const QString &uskName)
{
    if (!m_hashOfUsk.contains(uskName))
        return;
    uskInfo *pUskInfo = &m_hashOfUsk[uskName];
    QString portName = pUskInfo->port;
    pUskInfo->timer->stop();
    bool isOpen = pUskInfo->serialPort->isOpen();
    if (isOpen)
        pUskInfo->serialPort->close();
    delete pUskInfo->timer;
    delete pUskInfo->buffer;
    delete pUskInfo->serialPort;
    m_hashOfUsk.remove(uskName);
    if (isOpen)
        emit portIsClose(uskName, portName);
}

void SendUSKv1WorkingThread::openUsk(const QString &uskName)
{
    if (!m_hashOfUsk.contains(uskName))
    {
        emit error(uskName, errorUskInstPresent);
        return;
    }
    uskInfo *pUskInfo = &m_hashOfUsk[uskName];
    if (pUskInfo->isOpen)
        return;
    pUskInfo->serialPort->setPort(pUskInfo->port);
    if (pUskInfo->serialPort->open(QSerialPort::ReadWrite))
    {
        if (pUskInfo->serialPort->setDataBits(QSerialPort::Data8) &&
                pUskInfo->serialPort->setBaudRate(9600) &&
                pUskInfo->serialPort->setStopBits(QSerialPort::OneStop) &&
                pUskInfo->serialPort->setFlowControl(QSerialPort::NoFlowControl) &&
                pUskInfo->serialPort->setParity(QSerialPort::NoParity))
        {
            emit portIsOpen(uskName, pUskInfo->port);
            pUskInfo->isBusy = false;
            //TODO: проверка наличия УСК (через команду сброс УСК)
        }
    } else
    {
        emit error(uskName, errorOpenPort);
    }
}

void SendUSKv1WorkingThread::closeUsk(const QString &uskName)
{
    // нет такого УСК
    if (!m_hashOfUsk.contains(uskName))
        return;
    uskInfo *pUskInfo = &m_hashOfUsk[uskName];
    pUskInfo->timer->stop();
    pUskInfo->serialPort->close();
    pUskInfo->isOpen = false;
    pUskInfo->isFaultly = false;
    pUskInfo->isPresent = false;
    pUskInfo->isBusy = true;
    pUskInfo->buffer->clear();
    emit portIsClose(pUskInfo->name, pUskInfo->port);
}

void SendUSKv1WorkingThread::sendTime(const QString &uskName, const QDateTime &time)
{
    if (m_hashOfUsk.contains(uskName))
    {
        uskInfo *pUskInfo = &m_hashOfUsk[uskName];
        uskPacket p;
        p.packet = getSetTimePacket(pUskInfo, time);
        p.description = trUtf8("Установка времени %1 на УСК \"%0\"").arg(uskName).arg(time.toString());
        p.waitResponse = true;
        pUskInfo->listOfPacketToSend.append(p);
    }
}

void SendUSKv1WorkingThread::resetUsk(const QString &uskName)
{
    if (m_hashOfUsk.contains(uskName))
    {
        uskInfo *pUskInfo = &m_hashOfUsk[uskName];
        uskPacket p;
        p.packet = getResetUskPacket(pUskInfo);
        p.description = trUtf8("Сброс УСК \"%0\"").arg(uskName);
        p.waitResponse = true;
        pUskInfo->listOfPacketToSend.append(p);
    }
}

void SendUSKv1WorkingThread::changeRelayStatus(const QString &uskName, const int &rayNum, const int &kpuNum, const int &sensorNum, const int &relayStatus)
{
    if (m_hashOfUsk.contains(uskName))
    {
        uskInfo *pUskInfo = &m_hashOfUsk[uskName];
        uskPacket p;
        p.packet = getChangeKpuRelayStatePacket(pUskInfo, rayNum, kpuNum, sensorNum, relayStatus);
        p.description = trUtf8("Изменение контактов реле на УСК \"%0\"").arg(uskName);
        p.waitResponse = false;
        pUskInfo->listOfPacketToSend.append(p);
    }
}

void SendUSKv1WorkingThread::changeVoltageStatus(const QString &uskName, const int numOutput, const bool &on)
{
    if (m_hashOfUsk.contains(uskName))
    {
        uskInfo *pUskInfo = &m_hashOfUsk[uskName];
        uskPacket p;
        p.packet = getVoltageStatusPacket(pUskInfo, numOutput, on);
        p.description = trUtf8("Изменение статуса НЧ выхода номер %1 на УСК \"%0\"").arg(uskName).arg(numOutput);
        p.waitResponse = false;
        pUskInfo->listOfPacketToSend.append(p);
    }
}

void SendUSKv1WorkingThread::onReadyRead()
{
    QSerialPort *sp = qobject_cast<QSerialPort*>(sender());
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
        if (pUskInfo->uskStatus == uskWaitPeriod)
            return;
        pUskInfo->timer->stop();
        pUskInfo->buffer->append(pUskInfo->serialPort->readAll());

        while (pUskInfo->buffer->length() >= 26)
        {
            QByteArray packet = pUskInfo->buffer->left(26);
            pUskInfo->buffer->remove(0, 26);
            pUskInfo->uskStatus = uskWaitData;

            // проверяем на корректность пакета постредством CRC
            char crc = 0;
            for (int i = 0; i < 25; ++i)
                crc += packet.at(i);
            if (crc != packet.at(25))
            {
                // crc не сошлось
                emit error(uskName, errorUskWrongPacket);
                pUskInfo->isPresent = false;
                continue;
            }
            // извлекаем полезную информацию
            pUskInfo->isPresent = true;
            short uskNum = (ushort)packet.at(0) + (ushort)packet.at(1) * 0x100;
            char u1 = packet.at(23);
            char u2 = packet.at(24);
            //char priority = packet.at(5);
            quint32 flags =
                    (quint32)packet.at(2) * 0x01 +
                    (quint32)packet.at(3) * 0x0100 +
                    (quint32)packet.at(4) * 0x010000 +
                    (quint32)packet.at(4) * 0x01000000;
            QByteArray cp1251Message = packet.mid(7, 16);
            QString textMessage = m_codecWin1251->toUnicode(cp1251Message);
            QString textMessageLover = textMessage.toLower();
            if (textMessageLover.trimmed() == trUtf8("включение уск"))
            {
                emit uskInfoPacketReceived(uskName, packetUskOn);

            } else if (textMessageLover.trimmed() == trUtf8("полный сброс уск"))
            {
                emit uskInfoPacketReceived(uskName, packetUskReset);

            } else if (textMessageLover.trimmed() == trUtf8("установка часов"))
            {
                emit uskInfoPacketReceived(uskName, packetUskSettingTime);
            } else if (textMessageLover.trimmed() == "ошибка приема rs")
            {
                emit uskInfoPacketReceived(uskName, packetUskErrorReceivingRS);
                emit errorOnSendingCommand(uskName, pUskInfo->prevPacketDescription);
                if (pUskInfo->currentNumberOfAttemps < pUskInfo->numberOfAttemps)
                {
                    // надо бы перепослать пакетик
                    pUskInfo->currentNumberOfAttemps++;
                    pUskInfo->uskStatus = uskWaitResendPrevCommand;
                    pUskInfo->timer->start(1000);
                }
            } else if (textMessageLover.mid(0, 2) == "l=" && textMessageLover.mid(4, 2) == "k=")
            {
                // изменение состояния контактов

                int rayNum = QString(textMessageLover.at(2)).toInt();
                int kpuNum = QString(textMessageLover.at(6)).toInt();
                /*
                qDebug() << trUtf8("изм. конт. КПУ %0 луч %1 пред %2 тек %3")
                            .arg(kpuNum).arg(rayNum).arg((uchar)u1, 8, 2, QChar('0')).arg((uchar)u2, 8, 2, QChar('0'));
                            */
                // начальный бит - нулевой (младший)
                char bit = 0x01;
                // пробегаемся по всем контактам
                for (int i = 0; i < 8; ++i)
                {
                    // если предыдущее и текущее состояние различаеся - информируем внешний мир об этом
                    if ((u1 ^ u2) & bit)
                        emit sensorChanged(uskName, rayNum, kpuNum,  i+ 1, u2 & bit ? 1 : 0);
                    // переходим к сл. датчику (биту)
                    bit <<= 1;
                }

            } else if (textMessageLover.left(9) == trUtf8("новый оу:"))
            {
                // обнаружено новое оконечное устройство (кпу)
                int rayNum = QString(textMessageLover.at(9)).toInt();
                int kpuNum = QString(textMessageLover.at(15)).toInt();
                emit detectedNewKpu(uskName, rayNum, kpuNum);
            } else
            {
                emit unknowCommand(uskName, textMessage);
            }
        }

        if (pUskInfo->buffer->length() != 0)
        {
            //что то приняли, но не до конца, пытаемся допринять.
            pUskInfo->timer->start(200);
        } else
            pUskInfo->isBusy = false;
    }
}

void SendUSKv1WorkingThread::onTimer()
{
    if (!sender())
        return;
    QString uskName = sender()->property("uskName").toString();
    if (uskName.isEmpty())
        return;
    if (m_hashOfUsk.contains(uskName))
    {
        uskInfo *pUskInfo = &m_hashOfUsk[uskName];
        pUskInfo->timer->stop();
        switch (pUskInfo->uskStatus)
        {
        case uskWaitData:
        {
            if (pUskInfo->buffer->length() >= 2)
            {
                emit error(uskName, errorTimeoutWhileWaitData);
                pUskInfo->uskStatus = uskWaitPeriod;
                pUskInfo->timer->start(1000);
            } else
                pUskInfo->buffer->clear();
        }
            break;
        case uskWaitResponse:
        {
            if (pUskInfo->buffer->length() == 0)
            {
                emit error(uskName, errorTimeoutWhileWaitResponse);
                if (pUskInfo->currentNumberOfAttemps < pUskInfo->numberOfAttemps)
                {
                    pUskInfo->uskStatus = uskWaitResendPrevCommand;
                    pUskInfo->currentNumberOfAttemps++;
                }
                else
                    pUskInfo->uskStatus = uskWaitPeriod;

                pUskInfo->timer->start(1000);
            } else if (pUskInfo->buffer->length() >= 2)
            {
                emit error(uskName, errorTimeoutWhileWaitResponse);
                pUskInfo->uskStatus = uskWaitPeriod;
                pUskInfo->timer->start(1000);
            } else
                pUskInfo->buffer->clear();
        }
            break;
        case uskWaitPeriod:
        {
            pUskInfo->uskStatus = uskWaitData;
            pUskInfo->buffer->clear();
            if (pUskInfo->serialPort->isOpen())
                pUskInfo->serialPort->readAll();
            pUskInfo->isBusy = false;
        }
            break;
        case uskWaitResendPrevCommand:
        {
            emit startSendingCommand(uskName, pUskInfo->prevPacketDescription);
            pUskInfo->serialPort->write(pUskInfo->prevPacket);

            pUskInfo->uskStatus = pUskInfo->prevPacketWaitResponse ? uskWaitResponse : uskWaitAfterSendCommand;
            pUskInfo->timer->start(1000);
        }
            break;
        case uskWaitAfterSendCommand:
        {
            pUskInfo->isBusy = false;
            pUskInfo->uskStatus = uskWaitData;
        }
        break;
        }
    }
}

void SendUSKv1WorkingThread::onTimer2()
{
    qDebug() << "onTimer2()";
}

void SendUSKv1WorkingThread::onCheckBuffersTimer()
{
    foreach(const QString &key, m_hashOfUsk.keys())
    {
        uskInfo &ui = m_hashOfUsk[key];
        if (!ui.isBusy)
        {
            if (!ui.listOfPacketToSend.isEmpty())
            {
                ui.isBusy = true;
                {
                    const uskPacket &packet = ui.listOfPacketToSend[0];
                    ui.prevPacket = packet.packet;
                    ui.prevPacketDescription = packet.description;
                    ui.currentNumberOfAttemps = 0;
                    ui.uskStatus = packet.waitResponse ? uskWaitResponse : uskWaitAfterSendCommand;
                    ui.serialPort->write(packet.packet);
                    emit startSendingCommand(ui.name, packet.description);
                    if (packet.waitResponse)
                    {
                        // отклик ждем 3 секунды
                        ui.timer->start(3000);
                    } else
                    {
                        // пауза между посылками команд - пол секунды
                        ui.timer->start(500);
                    }
                }
                ui.listOfPacketToSend.removeFirst();
            }
        }
    }
}

QByteArray SendUSKv1WorkingThread::getSetTimePacket(const uskInfo *const ui, const QDateTime &time)
{
    QByteArray res;
    res.append(getFirstPartOfPacket(ui->num, 2, 0));
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
    res.append(static_cast<char>(0x00));
    res.append(static_cast<char>(0x00));
    appendCrcToPacket(res);
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

QByteArray SendUSKv1WorkingThread::getChangeKpuRelayStatePacket(const uskInfo *const ui, const int &rayNum, const int &kpuNum, const int &sensorNum, const int &state)
{
    QByteArray res;
    res.append(getFirstPartOfPacket(ui->num, 0x00001000, 0));
    QString textCommand = trUtf8("КПУ %0/%1/%2/%3").arg(rayNum % 10).arg(kpuNum % 10).arg(sensorNum % 10).arg(state == 1 ? trUtf8("Вкл 0 ") : trUtf8("Выкл 0"));
    QByteArray encodedCommand = m_codecWin1251->fromUnicode(textCommand);
    res.append(encodedCommand);
    res.append(static_cast<char>(rayNum));
    res.append(static_cast<char>(kpuNum * 0x10 + state));
    appendCrcToPacket(res);
    QString str;
    for (int i = 0; i < res.length(); ++ i)
        str += trUtf8("%0 ").arg((uchar)res.at(i), 2, 16, QChar('0'));
    qDebug() << str;
    qDebug() << textCommand;
    return res;
}



void SendUSKv1WorkingThread::appendCrcToPacket(QByteArray &packet)
{
    char crc = 0x00;
    for (int i = 0; i < packet.length(); ++i)
        crc += packet.at(i);
    packet.append(crc);
}

QByteArray SendUSKv1WorkingThread::getFirstPartOfPacket(const ushort &uskNum, const quint64 &flags, const quint8 &priority = 0x00)
{
    QByteArray res;
    res.append(static_cast<char>(0x00));
    res.append(uskNum & 0xff);
    res.append((uskNum >> 8) & 0xff);
    res.append(flags & 0xff);
    res.append((flags >> 8) & 0xff);
    res.append((flags >> 16) & 0xff);
    res.append((flags >> 24) & 0xff);
    res.append(priority);
    return res;
}

QByteArray SendUSKv1WorkingThread::getVoltageStatusPacket(const uskInfo * const ui, const int &numOutput, const bool &on)
{
    quint64 flag;
    switch (numOutput) {
    case 2:
        flag = on ? 0x00002000 : 0x00004000;
        break;
    default:
        flag = on ? 0x00000020 : 0x00000100;
        break;
    }
    QByteArray res = getFirstPartOfPacket(ui->num, flag, 0);
    res.append("                ");
    res.append((char)0x00);
    res.append((char)0x00);
    appendCrcToPacket(res);
    return res;
}

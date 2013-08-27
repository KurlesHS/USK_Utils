#include "serialthreadworker.h"
#include <QHostAddress>
#include "mywidget.h"

#define CURRENT_TIME QTime::currentTime().toString("hh:mm:ss")

SerialThreadWorker::SerialThreadWorker(QObject *parent) :
    QObject(parent),
    m_currentMode(terminalMode),
    m_currnetConnectionType(noneConnectionType),
    m_delayBetweenPacket(200),
    m_timeoutInMs(5000),
    m_timerForCheckPackets(this),
    m_timerForTimeouts(this)

{
    m_socket = new QTcpSocket(this);
    m_serialPort = new AbstractSerial(this);
    m_timerForTimeouts.setSingleShot(true);
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    connect(m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
    connect(m_socket, SIGNAL(connected()),
            this, SLOT(onSocketConnected()));
    connect(m_socket, SIGNAL(disconnected()),
            this, SLOT(onSocketDisconnected()));
    connect(&m_timerForTimeouts, SIGNAL(timeout()), this, SLOT(onTimeout()));
    connect(&m_timerForCheckPackets, SIGNAL(timeout()), this, SLOT(onCheckPacket()));
    m_currentState = waitDataState;
}

SerialThreadWorker::~SerialThreadWorker()
{
    qDebug() << Q_FUNC_INFO;
    delete m_socket;
    delete m_serialPort;
}

void SerialThreadWorker::parseIncomingPacket(QIODevice * const device)
{
    m_packetDecoder.parsePacket(m_currentDataChunk);
    int state = m_packetDecoder.getStatePacket();
    switch (state) {
    case PacketDecoder::IncorrcetPacket:
    {
        // приняли некорректный пакет - игнорируем его
        m_currentDataChunk.clear();
        emit logMessage(trUtf8("Принят некорректный пакет данных"));
        m_currentState = waitDataState;
        m_timerForCheckPackets.start(m_delayBetweenPacket);
    }
        break;
    case PacketDecoder::IncompletePacket:
    {
        m_currentState = continueWaitResponseState;
        // приняли неполный пакет - даем еще немного времени на то,
        // что бы допринять его.
        m_timerForTimeouts.start(m_timeoutInMs);
    }
        break;
    case PacketDecoder::CorrectPacket:
    {
        // приняли корректный пакет - выводим его в окно разбора пактов
        QString string = trUtf8("%0: Принят пакет, его данные: ").arg(QTime::currentTime().toString("hh:mm:ss"));
        emit logMessageForParserWindows(string, "red");
        string = trUtf8("<br/>Hex: %0<br/>Dec: %1<br/>Char: %2")
                      .arg(getHexString(m_currentDataChunk))
                      .arg(getDecString(m_currentDataChunk))
                      .arg(getCharString(m_currentDataChunk));
        emit logMessageForParserWindows(string, "gray");
        string = trUtf8("<br/>Номер УСК: %0").arg(m_packetDecoder.getUSKNum());
        emit logMessageForParserWindows(string, "darkred");
        emit logMessageForParserWindows(trUtf8("<br/>Установленные флаги: <br/>"), "black");
        foreach(const QString &flagName, m_packetDecoder.getFlagNames())
        {
            emit logMessageForParserWindows(flagName, "black");
        }

        QBitArray flags = m_packetDecoder.getFlags();
        if (flags.testBit(PacketDecoder::F05)) //есть ли массив данных
        {
            emit logMessageForParserWindows(trUtf8("<br/>Присутствует массив данных:"), "black");
            emit logMessageForParserWindows(trUtf8("<br/>Hex: %0").arg(getHexString(m_packetDecoder.getArray())), "black");
            emit logMessageForParserWindows(trUtf8("Dec: %0").arg(getDecString(m_packetDecoder.getArray())), "black");
            emit logMessageForParserWindows(trUtf8("Char: %0").arg(getCharString(m_packetDecoder.getArray())), "black");
        }

        if ((flags.testBit(PacketDecoder::F04) && !flags.testBit(PacketDecoder::F03))
                || (!flags.testBit(PacketDecoder::F04) && flags.testBit(PacketDecoder::F03)))
        {
            //присутствует строка символов
            emit logMessageForParserWindows(trUtf8("<br/>Присутствует текстовая строка: %0").arg(m_packetDecoder.getTextMessage()), "black");
        }

        if (flags.testBit(PacketDecoder::F06) && flags.testBit(PacketDecoder::F05 && m_packetDecoder.getArray().length() >= 3) ) //команда от КПУ
        {
            int commad = (uchar)m_packetDecoder.getArray().at(2);
            emit logMessageForParserWindows(trUtf8("<br/>Установлены флаги f5, f6"), "black");
            emit logMessageForParserWindows(trUtf8("Команда от КПУ: %0").arg(m_packetDecoder.getCommand(commad)), "black");
            switch (commad)
            {
            case 0x01221:
                break;
            case 0x0c:
                //обнаружен кпу
            {
                int type;
                int kpuNum;
                int state;
                int softVersion;
                int hardVersion;
                quint64 serialNumber;
                m_packetDecoder.getNewKPUData(type, kpuNum, state, softVersion, hardVersion, serialNumber);
                emit logMessageForParserWindows(trUtf8("<br/>Номер КПУ: %0;<br/>Тип КПУ:%1;<br/>Сосотояние контактов:%2"
                                                  /*"<br/>Версия ПО:%3<br>Версия платы:%4"
                                                  "<br/>Серийный номер:%5"*/)
                                                .arg(kpuNum)
                                                .arg(type)
                                                .arg(getBinString(QByteArray().append((char)state))), "black");
                                           //.arg(softVersion)
                                           //.arg(hardVersion)
                                           //.arg(serialNumber), "black");
            }
                break;
            case 0x0d:
                //отключен/неисправен КПУ
                emit logMessageForParserWindows(trUtf8("<br/>Номер КПУ:%0").arg(m_packetDecoder.getKPUNum()), "black");
                break;
            case 0x00:
                // изменение состояния датчиков КПУ
                emit logMessageForParserWindows(trUtf8("<br/>Номер КПУ:%0"
                                                  "<br/>Тип КПУ:%1"
                                                  "<br/>Состояние контактов:%2"
                                                  "<br/>Предыдущее состояние контактов:%3")
                                           .arg(m_packetDecoder.getKPUNum())
                                           .arg(m_packetDecoder.getKPUType())
                                           .arg(getBinString(QByteArray().append((char)m_packetDecoder.getKPUState())))
                                           .arg(getBinString(QByteArray().append((char)m_packetDecoder.getPrevKPUState()))), "black");
                break;
            default:
                break;
            }
        }
    }
        break;
    case PacketDecoder::UnknownPacket:
    {
        emit logMessage(trUtf8("принят неизвестный пакет!"));
        QString string = trUtf8("%0: Принят неизвестный пакет, его данные: ").arg(CURRENT_TIME);
        emit logMessageForParserWindows(string, "red");
        string = trUtf8("<br/>Hex: %0<br/>Dec: %1<br/>Char: %2")
                      .arg(getHexString(m_currentDataChunk))
                      .arg(getDecString(m_currentDataChunk))
                      .arg(getCharString(m_currentDataChunk));
        emit logMessageForParserWindows(string, "gray");
    }
    default:
        break;
    }

    // посылаем отклик в нужном случае
    switch (state) {
    case PacketDecoder::CorrectPacket:
    case PacketDecoder::UnknownPacket:
    {
        emit logMessageForParserWindows(trUtf8("<br/>------------Конец пакета--------------<br/>"), "red");
        QByteArray response;
        int numUsk = m_packetDecoder.getUSKNum();
        response.append(MyWidget::prepareComplexNumber(numUsk));
        response.append((char)0x00);
        quint8 crc = 0;
        foreach (char byte, response)
            crc += (quint8)byte;
        response.append((char)crc);
        device->write(response);
        emit logMessageForParserWindows(trUtf8("%0 Отсылаем стандартный отклик:").arg(CURRENT_TIME), "darkred");
        QString string = trUtf8("<br/>Hex: %0<br/>Dec: %1<br/>Char: %2")
                      .arg(getHexString(response))
                      .arg(getDecString(response))
                      .arg(getCharString(response));
        emit logMessageForParserWindows(string, "gray");
        emit logMessageForParserWindows(trUtf8("<br/>------------Конец отклика--------------<br/>"), "red");
        m_timerForCheckPackets.start(m_delayBetweenPacket);
        m_currentDataChunk.clear();

    }
        break;
    default:
        break;
    }
}

void SerialThreadWorker::parseIncomingResponse(QIODevice * const device)
{
    QByteArray chunkData = device->readAll();
    emit dataArrived(chunkData);
    m_currentDataChunk.append(chunkData);
    m_packetDecoder.parseResponse(m_currentDataChunk);
    int state = m_packetDecoder.getStatePacket();
    switch(state) {
    case PacketDecoder::IncompletePacket:
    {
        m_currentState = continueWaitResponseState;
        m_timerForTimeouts.start(m_timeoutInMs);
    }
        break;
    case PacketDecoder::IncorrcetPacket:
    {
        emit logMessage(trUtf8("Принят ошибочный отклик от УСК"));
        emit logMessageForParserWindows(trUtf8("<br/>%0: Принят ошибочный отклик от УСК<br/>").arg(CURRENT_TIME), "red");
        m_currentDataChunk.clear();
        m_currentState = waitDataState;
        m_timerForCheckPackets.start(m_delayBetweenPacket);
    }
        break;
    case PacketDecoder::CorrectPacket:
    {
        int numUsk = m_packetDecoder.getUSKNum();
        emit logMessage(trUtf8("Принят отклик от УСК №%0").arg(numUsk));
        emit logMessageForParserWindows(trUtf8("<br/>%0: Принят отклик от УСК №%1<br/>").arg(CURRENT_TIME).arg(numUsk), "darkred");
        quint16 modules = m_packetDecoder.getModules();
        QByteArray bmod;
        bmod.append(modules % 0x100);
        bmod.append(modules / 0x100);
        emit logMessageForParserWindows(trUtf8("Байты модулей: %0").arg(getBinString(bmod)), "darkred");
        m_currentState = waitDataState;
        m_timerForCheckPackets.start(m_delayBetweenPacket);
        m_currentDataChunk.clear();
    }
        break;
    }
}

void SerialThreadWorker::setCurrentMode(int mode)
{
    m_currentMode = static_cast<modes>(mode);
    m_currentState = waitDataState;
}

void SerialThreadWorker::setDelayBetweenPacket(int ms)
{
    m_delayBetweenPacket= ms;
}

void SerialThreadWorker::sendPacket(const QByteArray &packet, const QString &description, const int &numberOfTryingToSend)
{
    Packet p;
    p.desctiption = description;
    p.numCount = numberOfTryingToSend;
    p.packet = packet;
    m_queueOfPacketToSend.enqueue(p);
}

void SerialThreadWorker::openSerialPort(const QString &portName, const QString &baudrate, const QString &flowcontrol, const QString stopbits, const QString databits, const QString &parity)
{
    if (m_currnetConnectionType == noneConnectionType) {
        QString errorText;
        if (!m_serialPort->open(QIODevice::ReadWrite)) {
            errorText = trUtf8("Ошибка открытия последовательного порта %").arg(portName);
        } else if (!m_serialPort->setBaudRate(baudrate)) {
            errorText = trUtf8("Ошибка инициализации скорости последовательного порта %0 на значение %1").arg(portName).arg(baudrate);
        } else if (!m_serialPort->setFlowControl(flowcontrol)) {
            errorText = trUtf8("Ошибка инициализации управления потока последовательного порта %0 значением %1").arg(portName).arg(flowcontrol);
        } else if (!m_serialPort->setStopBits(stopbits)) {
            errorText = trUtf8("Ошибка установки stop bits последовательного порта %0 значением %1").arg(portName).arg(stopbits);
        } else if (!m_serialPort->setDataBits(databits)) {
            errorText = trUtf8("Ошибка установки data bits последовательного порта %0 значением %1").arg(portName).arg(databits);
        } else if (!m_serialPort->setParity(parity)) {
            errorText = trUtf8("Ошибка установки parity последовательного порта %0 значением %1").arg(portName).arg(parity);
        }
        if (errorText.isEmpty()) {
            emit connectedToUsk(true, trUtf8("Соеденено с последовательным портом %0 установлено").arg(portName));
        } else {
            emit connectedToUsk(false, errorText);
            m_serialPort->reset();
            m_serialPort->flush();
            m_serialPort->close();
            m_currnetConnectionType = noneConnectionType;
        }
    } else {
        emit connectedToUsk(false, trUtf8("Соединение к последновательному порту или сервером уже устновлено"));
    }
}

void SerialThreadWorker::openEthernet(const QString &addr, const int &port)
{
    if (m_currnetConnectionType == noneConnectionType && m_socket->state() == QAbstractSocket::UnconnectedState) {
        m_socket->connectToHost(QHostAddress(addr), port);
        m_currnetConnectionType = ethernetConnectionType;
        m_timerForCheckPackets.start(m_delayBetweenPacket);
    } else {
        emit connectedToUsk(false, trUtf8("Соединение к последновательному порту или сервером уже устновлено"));
    }
}

bool SerialThreadWorker::isConnectedToUsk()
{
    return m_currnetConnectionType == serialConnectionType || m_currnetConnectionType == ethernetConnectionType;
}

void SerialThreadWorker::closeConnection()
{
    switch (m_currnetConnectionType) {
    case ethernetConnectionType:
    {
           m_socket->abort();
    }
        break;
    case serialConnectionType:
    {
        m_currnetConnectionType = noneConnectionType;
        emit connectedToUsk(false, trUtf8("Соединение с поселдовательным портом %0 разорвано").arg(m_serialPort->deviceName()));
        m_serialPort->reset();
        m_serialPort->close();
    }
        break;
    default:
        break;
    }
}

void SerialThreadWorker::onReadyRead()
{
    QIODevice *device = qobject_cast<QIODevice*>(sender());
    if (device) {
        QByteArray packet = device->readAll();
        if (packet.isEmpty()) {
            return;
        }
        emit dataArrived(packet);
        if (m_currentMode == terminalMode) {
            return;
        }
        m_timerForTimeouts.stop();
        m_timerForCheckPackets.stop();
        m_currentDataChunk.append(packet);
        switch (m_currentState) {
        case waitDataState:
        case continueWaitDataState:
        {
            parseIncomingPacket(device);
        }
            break;
        case waitResponseState:
        case continueWaitResponseState:
        {
            parseIncomingResponse(device);
        }
            break;
        default:
            break;
        }
    }
}

void SerialThreadWorker::onSocketConnected()
{
    emit connectedToUsk(true, trUtf8("Соединение с сервером %1 установлено").arg(m_socket->peerAddress().toString()));
}

void SerialThreadWorker::onSocketDisconnected()
{
    m_currnetConnectionType = noneConnectionType;
    emit connectedToUsk(false, trUtf8("Coединение с сервером %0 прервано").arg(m_socket->peerAddress().toString()));
}

void SerialThreadWorker::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    Q_UNUSED(socketState)
}

void SerialThreadWorker::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    m_currnetConnectionType = noneConnectionType;
    emit connectedToUsk(false, trUtf8("Ошибка сокета: %0").arg(m_socket->errorString()));
}

void SerialThreadWorker::onTimeout()
{
    m_timerForTimeouts.stop();
    switch(m_currentState) {
    case waitDataState:
    case continueWaitDataState:
    {
        emit logMessage(trUtf8("Таймаут при получении пакета от УСК"));
        emit logMessageForParserWindows(trUtf8("%0: Таймаут при ожидании отклика от УСК").arg(CURRENT_TIME), "red");
    }
        break;
    case waitResponseState:
    case continueWaitResponseState:
    {
        emit logMessage(trUtf8("Таймаут при ожидании отклика от УСК"));
        emit logMessageForParserWindows(trUtf8("%0: Таймаут при ожидании отклика от УСК").arg(CURRENT_TIME), "red");
    }
        break;
    default:
        break;
    }
    m_currentState = waitDataState;
    m_timerForCheckPackets.start(m_delayBetweenPacket);
}

void SerialThreadWorker::onCheckPacket()
{
    if (!m_queueOfPacketToSend.isEmpty()) {
        m_timerForCheckPackets.stop();
        QIODevice *dev = nullptr;
        if (m_serialPort->isOpen()) {
            dev = m_serialPort;
        } else if (m_socket->isOpen()) {
            dev = m_socket;
        } else {
            return;
        }
        Packet p = m_queueOfPacketToSend.dequeue();
        m_currentPacket = p;
        if (p.packet.isEmpty()) {
            return;
        }
        dev->write(p.packet);
        if (m_currentMode == packetMode)
            m_timerForTimeouts.start(m_timeoutInMs);
        else
            m_timerForCheckPackets.start(m_delayBetweenPacket);
        m_currentState = waitResponseState;
    }
}

QString SerialThreadWorker::getHexString(QByteArray array)
{
    QString res;
    if (array.isEmpty()) return res;
    for (int i=0;i < array.count(); ++i) {
        quint8 num = quint8(array.at(i));
        QString hex = QString::number(num, 16);
        hex = hex.toUpper();
        num <= 15 ? hex = "0" + hex + " " : hex += " ";
        res.append(hex);
    }
    return res;
}

QString SerialThreadWorker::getDecString(QByteArray array)
{
    QString res;
    if (array.isEmpty()) return res;
    for (int i=0;i < array.count(); ++i) {
        quint8 num = quint8(array.at(i));
        QString dec = QString::number(num);
        dec.append(" ");
        res.append(dec);
    }
    return res;
}

QString SerialThreadWorker::getBinString(QByteArray array)
{
    QString res;
    if (array.isEmpty()) return res;
    for (int i=0;i < array.count(); ++i) {
        quint8 num = quint8(array.at(i));
        quint8 mask = 0x80;
        QString bin;
        bin.clear();
        for (int i = 0;i <= 7 ;++i){
            mask & num ? bin += "1" : bin += "0";
            mask = mask >> 1;
        }
        bin.append(" ");
        res.append(bin);
    }
    return res;
}

QString SerialThreadWorker::getCharString(QByteArray array)
{

    QString chars, res;
    if (array.isEmpty()) return res;
    for (int i=0;i < array.count(); ++i) {
        char num = char(array.at(i));

        chars = QString::fromAscii(&num, 1);
        chars.append(" ");
        res.append(chars);
    }
    return res;
}

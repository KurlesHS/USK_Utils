#include "serialthread.h"

#include <QApplication>
SerialThread::SerialThread(const type &t, QObject *parent) :
    QThread(parent),
    currentType(t),
    serialPort(0)
{

    status = waitData;
    repeatCount = 1;
    tryCount = 2;
    usePacketRepeat = false;
    currentTime.start();
}

SerialThread::~SerialThread()
{

    if (isRunning()) {
        exit();
        if (!wait(2000))
            terminate();
    }
}

void SerialThread::setRepeatPacketCount(int value)
{
    repeatCount = value;
}

void SerialThread::setPacketDecoderMode(bool packerDecoderMode)
{
    usePackerDecoderMode = packerDecoderMode;
}

void SerialThread::setDelayBetweenSendPacket(int delayInSec)
{
    delayBetweenSendPacket = delayInSec;
}

void SerialThread::setUseRepeatPacket(bool use)
{
    usePacketRepeat = use;
}

void SerialThread::setSerialPortDriver(QIODevice *serial){
    serialPort = serial;
}

void SerialThread::run()
{
    if (currentType == typeEthernet) {
        QTcpSocket *socket = new QTcpSocket();
        qDebug() << hostAddress << portNumber;
        socket->connectToHost(hostAddress, portNumber);
        connect(socket, SIGNAL(connected()), this, SIGNAL(connected()));
        connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(error(QAbstractSocket::SocketError)));
        serialPort = socket;
        status = waitData;

    }
    m_terminate = false;
    incommingArray.clear();
    msleep(1);
    while (true){
        QApplication::processEvents();
        if (m_terminate) {
            if (currentType == typeEthernet)
            {
                QTcpSocket* socket = (QTcpSocket*)serialPort;
                socket->abort();
                QApplication::processEvents();
                delete serialPort;
                return;
            }
        }
        switch(status){
        //Ожидание данных
        case waitData:{
            //Есть данные в порту - читаем их
            if (serialPort->bytesAvailable())
            {
                incommingArray.append(serialPort->readAll());
                //информируем внешний класс о принятых данных
                status = waitDecisionFromExternClass;
                emit dataReceived(incommingArray);
                //ждем решения от внешнего класса:
                //пакет вылидный - шлем отклик,
                //пакет не полный - продолжаем чтение
                //ошибочный пакет - курим оперделенное время, сбрасываем порт и снова на чтение данных
                break;
            }
            //ждем прихода данных
            msleep(10);
        }
            break;
        //отправка пакета
        case sendData:{
            //шлем пакет
            setTerminationEnabled(false);
            serialPort->write(array);
            qDebug() << "in SM:" << array;
            setTerminationEnabled(true);
            //сбрасываем таймер
            currentTime.restart();
            incommingArray.clear();
            //и в зависимости от текущего режима программы
            //переключаемся на ожидание отклика или очередного пакета данных
            if (usePackerDecoderMode)
                status = waitResponse;
            else
                status = waitData;
        }
            break;

        //ожидание отклика
        case waitResponse:{
            //Есть данные в порту - читаем их
            if (serialPort->bytesAvailable())
            {
                //добавляем их в массив
                incommingArray.append(serialPort->readAll());
                //информируем внешний класс о принятии отклика
                status = waitDecisionFromExternClass;
                emit responseReceived(incommingArray);
                //ждем решения от внешнего класса:
                //отклик валидный - переходим на ожидание данных
                //отклик не полный - продолжаем чтение
                //ошибочный отклик - курим оперделенное время, сбрасываем порт и пытаемся второй раз послать пакет
                //если и второй раз ошибка - информируем об ошибке и на ожидание пакета
                break;
            }
            if (currentTime.elapsed() >= 500)
            {
                --tryCount;
                if (tryCount)
                    //есть еще одна попытка
                {
                    array = tempArray;
                    serialPort->reset();
                    incommingArray.clear();
                    msleep(200);
                    status = sendData;
                    break;
                }
                //попытки кончились - информируем внешний класс о таймауте при приеме отклика
                emit timeOutHappensWhileWaitingResponse();
                incommingArray.clear();

                /*
                //если нужно повторить посылку пакета
                qDebug() << "RepeatCount:" << repeatCount;
                if (repeatCount - 1) {
                    --repeatCount;
                    array = tempArray;
                    serialPort->reset();
                    incommingArray.clear();
                    msleep(200);
                    tryCount = 2;
                    status = sendData;
                    break;
                }
                */
                if (usePacketRepeat)
                {
                    sleep(delayBetweenSendPacket);
                    array = tempArray;
                    serialPort->reset();
                    incommingArray.clear();
                    tryCount = 2;
                    status = sendData;
                    break;
                }
                //и переключаемся на прием пакета
                startWaitingDataWithDelay(200);
            }
        }
            break;

        //посылка отклика
        case sendResponse:{
            //тут уже от нас ничего не зависит, посылаем отклик
            qDebug() << trUtf8("Посылка отклика") << array;
            setTerminationEnabled(false);
            serialPort->write(array);
            setTerminationEnabled(true);
            //курим 200мс
            //и в любом случае переходим на ожиадние данных
            startWaitingDataWithDelay(200);
        }
            break;

        case waitDecisionFromExternClass:
            msleep(1);
            break;
        //продолжаем ждать пакет
        case continueWaitData:
        {
            if (serialPort->bytesAvailable())
            {
                incommingArray.append(serialPort->readAll());
                //информируем внешний класс о принятых данных
                status = waitDecisionFromExternClass;
                emit dataReceived(incommingArray);
                //ждем решения от внешнего класса:
                //пакет вылидный - шлем отклик,
                //пакет не полный - продолжаем чтение
                //ошибочный пакет - курим оперделенное время, сбрасываем порт и снова на чтение данных
                break;
            }
            //тут уже надо на тайминги смотреть
            if (currentTime.elapsed() >=500)
            {
                status = waitData;
                currentTime.restart();
                incommingArray.clear();
                emit timeOutHappensWhileWaitingData();
                break;
            }
            //ждем прихода данных
            msleep(10);
        }
            break;
        }
    }
}

void SerialThread::sendPacket(QByteArray arr)
{
    array = arr;
    tryCount = 2;
    tempArray = arr;
    currentTime.restart();
    status = sendData;
}

void SerialThread::sendRespons(QByteArray array)
{
    this->array = array;
    tempArray = array;
    currentTime.restart();
    status = sendResponse;
}

void SerialThread::setNetworkSettings(const QString &host, const int &port)
{
    portNumber = port;
    hostAddress = host;
}

void SerialThread::continueWaitingData()
{
    status = continueWaitData;
    currentTime.restart();
}

void SerialThread::continueWaitingResponse()
{
    status = waitResponse;
    currentTime.restart();
}

void SerialThread::onErrorWhileReceiveResponse()
{
    --tryCount;
    if (tryCount)
        //есть еще одна попытка
    {
        array = tempArray;
        serialPort->reset();
        msleep(200);
        status = sendData;
        return;
    }
    //попытки кончились - информируем внешний класс об ошибке
    emit errorWhileReceiveResponse(incommingArray);
    incommingArray.clear();
    //если активирован режим повтора пакетов
    if (usePacketRepeat)
    {
        sleep(delayBetweenSendPacket);
        array = tempArray;
        serialPort->reset();
        incommingArray.clear();
        tryCount = 2;
        status = sendData;
        return;
    }

    //иначе переключаемся на прием пакета
    startWaitingDataWithDelay(200);
}

void SerialThread::startWaitingData()
{
    incommingArray.clear();
    status = waitData;
    array.clear();
    currentTime.restart();
    serialPort->reset();

}

void SerialThread::startWaitingDataWithDelay(int delay)
{
    msleep(delay);
    startWaitingData();
}

void SerialThread::onCorrectResponseReceived()
{

    if (repeatCount)
    {
        --repeatCount;
        msleep(200);
        serialPort->reset();
        status = sendData;
        array = tempArray;
        return;
    }
    emit sendPacketSuccess();
    startWaitingDataWithDelay(200);
}

void SerialThread::onIncorrectResponseReceived()
{
    --tryCount;
    if (tryCount)
        //есть еще одна попытка
    {
        array = tempArray;
        serialPort->reset();
        incommingArray.clear();
        msleep(200);
        status = sendData;
        return;
    }

    incommingArray.clear();
    //если нужно повторить посылку пакета
    if (repeatCount) {
        --repeatCount;
        array = tempArray;
        serialPort->reset();
        incommingArray.clear();
        msleep(200);
        tryCount = 2;
        status = sendData;
        return;
    }
    //и переключаемся на прием пакета
    startWaitingDataWithDelay(200);
}

void SerialThread::continueSendPacket()
{
    if (repeatCount) {
        --repeatCount;
        array = tempArray;
        serialPort->reset();
        incommingArray.clear();
        msleep(200);
        tryCount = 2;
        status = sendData;
        return;
    }
    startWaitingDataWithDelay(200);

}

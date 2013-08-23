#include "serialthreadworker.h"

SerialThreadWorker::SerialThreadWorker(QObject *parent) :
    QObject(parent)
{
    m_socket = new QTcpSocket();
    m_serialPort = new AbstractSerial();
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
    m_currentState = waitDataState;
}

SerialThreadWorker::~SerialThreadWorker()
{
    delete m_socket;
    delete m_serialPort;
}

void SerialThreadWorker::sendPacket(const QByteArray &packet, const QString &description, const int &numberOfTryingToSend)
{
    Packet p;
    p.desctiption = description;
    p.numCount = numberOfTryingToSend;
    p.packet = packet;
    m_listOfPacketToSend.append(p);
}

void SerialThreadWorker::sendResponse(const QByteArray &response)
{

}

void SerialThreadWorker::onReadyRead()
{

}

void SerialThreadWorker::onSocketConnected()
{

}

void SerialThreadWorker::onSocketDisconnected()
{

}

void SerialThreadWorker::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{

}

void SerialThreadWorker::onSocketError(QAbstractSocket::SocketError socketError)
{

}

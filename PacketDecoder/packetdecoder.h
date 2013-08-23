#ifndef PACKETDECODER_H
#define PACKETDECODER_H

#include <QByteArray>
#include <QBitArray>
#include <QStringList>
#include <QMap>
#include <QList>
#include <QDateTime>

//оперделения соответствия флагов
/*
#define F01 6
#define F02 5
#define F03 4
#define F04 3
#define F05 2
#define F06 1
#define F07 0
#define F08 13
#define F09 12
#define F10 11
#define F11 10
#define F12 9
#define F13 8
#define F14 7
*/
class PacketDecoder
{
public:

    enum Flags {
        F01 = 6,
        F02 = 5,
        F03 = 4,
        F04 = 3,
        F05 = 2,
        F06 = 1,
        F07 = 0,
        F08 = 13,
        F09 = 12,
        F10 = 11,
        F11 = 10,
        F12 = 9,
        F13 = 8,
        F14 = 7
    };

    enum StatePacket {
        CorrectPacket, //правильный пакет/ответ
        IncorrcetPacket, //неправиьный пакет/ответ
        IncompletePacket, //неполный пакет/ответ
        ResponsePacket, // отклик
        UnknownPacket //не получилось расшифровать пакет
    };

    enum ResponseType {
        StandartResponse, //стандартный ответ
        TimeSettingResponse //ответ на команду установки времени
    };

    enum AvailableOptions {

    };

    PacketDecoder();
    PacketDecoder(QByteArray packet);


    static QByteArray getSendTimeCommand(int uskNum, const QDateTime &time);
    static QByteArray getSendTextMessageCommand(const int &uskNum, const int &msgType, const QString &message);
    static QByteArray getChangeRelayContactsCommand(const int &uskNum, const int &kpuNum, const int &kpuType,
                             quint8 contacts, quint8 mask);
    static QByteArray getSendByteArrayOnKpuCommand(const int &uskNum, const int &kpuNum, int kpuType,
                            const QByteArray &byteArray);
    static QByteArray getResetUskCommand(const int &uskNum);

    //парсинг пакета
    void parsePacket(QByteArray packet);
    //парсинг ответа, type - обычный ответ, или ответ на команду установки времени
    void parseResponse(QByteArray response, ResponseType type = StandartResponse);
    //проверка пакета/ответа на валидность
    StatePacket getStatePacket();
    //
    QMap <int, QBitArray> getKpuStates();
    //номер уск, от которого пришел пакет или ответ
    int getUSKNum();
    //включенные флаги
    QBitArray getFlags();
    //список включенных флагов в текстовом представлении
    QStringList getFlagNames();
    //текстовое сообщение
    QString getTextMessage();
    //температура (ответ на команду установки времени)
    int getTemperature();
    //4 напряжения (ответ на команду установки времени)
    QByteArray getVoltages();
    //рабочие модули (ответ на команду установки времени)
    QBitArray getWorkingModules();
    //массив данных
    QByteArray getArray();
    //текстовое представление команды по ее номеру
    QString getCommand(int command);
    //текущая команда
    int getCommand(){return standartEvent;}
    //при обнаружении нового КПУ
    void getNewKPUData (int &type, int &kpuNum, int &state, int &softVersion, int &hardVersion, quint64 &serialNumber);
    int getKPUNum() {return kpuNum;}
    int getKPUType() {return kpuType;}
    int getKPUState() {return kpuState;}
    int getPrevKPUState() {return kpuPrevState;}

private:
    static int parseComplexNumber(quint64 &res, QByteArray array, quint64 index = 0);
    static QByteArray prepareComplexNumber(quint64 number);
    static void addCRCToPacket(QByteArray &packet);
    void initPacketDecoder();

    int typeOfError;
    quint64 USKNum; //номер УСК
    QBitArray flags; //флаги побитово
    quint64 uint64Flags; //флаги одним числом
    QBitArray workingModules;
    QByteArray dataArray;
    QMap<int, QString> nameOfFlags; //названия флагов
    QStringList nameOfCommand;
    StatePacket statePacket; //текущее состояние пакета
    QMap <int, QBitArray> kpuStatesMap;
    int kpuType, kpuNum, kpuState, kpuPrevState, kpuSoftVersion, kpuHardVersion;
    quint64 kpuSerialNumber;
    QString textMessage;
    quint16 modules;
    int standartEvent;
    int USKTemperature;
    QByteArray USKVoltages;
};

#endif // PACKETDECODER_H

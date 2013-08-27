#include "packetdecoder.h"
#include <QDebug>
#include <QDateTime>
#include <QTextCodec>

PacketDecoder::PacketDecoder()
{
    initPacketDecoder();
}

PacketDecoder::PacketDecoder(QByteArray packet)
{
    initPacketDecoder();
    parsePacket(packet);
}

void PacketDecoder::initPacketDecoder()
{
    nameOfFlags[6] =  QObject::trUtf8("F1 (загрузка времени)");
    nameOfFlags[5] =  QObject::trUtf8("F2 (Флаг тестовой команды)");
    nameOfFlags[4] =  QObject::trUtf8("F3 (Вывод 16 символов в строку ЖКИ)");
    nameOfFlags[3] =  QObject::trUtf8("F4 (Вывод 32 символов в стоку ЖКИ)");
    nameOfFlags[2] =  QObject::trUtf8("F5 (Присутствие массива данных)");
    nameOfFlags[1] = QObject::trUtf8("F6 (Обмен данными по шине КПУ)");
    nameOfFlags[0] = QObject::trUtf8("F7 (Приняты данные от канала FSK)");
    nameOfFlags[13] = QObject::trUtf8("F8 (Reserved)");
    nameOfFlags[12] = QObject::trUtf8("F9 (Reserved)");
    nameOfFlags[11] = QObject::trUtf8("F10 (Reserved)");
    nameOfFlags[10] = QObject::trUtf8("F11 (Reserved)");
    nameOfFlags[9] = QObject::trUtf8("F12 (Reserved)");
    nameOfFlags[8] =  QObject::trUtf8("F13 (Reserved)");
    nameOfFlags[7] = QObject::trUtf8("F14 (Reserved)");

    nameOfKpuCommand.clear();
    //nameOfKpuCommand.append(QObject::trUtf8("Сброс УСК"));
    nameOfKpuCommand[0x0c] = QObject::trUtf8("Обнаружен КПУ");
    nameOfKpuCommand[0x0d] = QObject::trUtf8("Неисправен/отключен КПУ");
    nameOfKpuCommand[0x00] = QObject::trUtf8("Изменение состояний портов КПУ");

    statePacket = IncompletePacket;
}

QByteArray PacketDecoder::prepareComplexNumber(quint64 number)
{

    QByteArray res;
    quint8 n;
    while (true){
        n= number & 0xff;
        if (number >= 0x80) res.append(n | 0x80);
        else {res.append(n); break;}
        number = number >> 7;
    }
    return res;
}

void PacketDecoder::parsePacket(QByteArray array)
{
    flags.clear();
    workingModules.clear();
    dataArray.clear();
    textMessage.clear();
    statePacket = IncompletePacket;
    standartEvent = -1;
    quint64 numUsk, _flags;
    qint8 crc = 0;
    for (int i=0; i < array.length(); ++i)
        crc += array.at(i);
    int len = parseComplexNumber(numUsk, array);

    // не полный адресс УСК - продолжаем четние
    if (!len) {
        statePacket = IncompletePacket;
        return;
    }
    USKNum = numUsk;
    // не правильный формат числа - точно принят не правильный пакет
    // оповещаем об этом пользователя
    // и возвращаемся в цикл опроса

    if (len >= 0xff){
        statePacket = IncorrcetPacket;
        return;
    }
    array.remove(0, len);
    len = parseComplexNumber(_flags, array);
    if (!len) {
        statePacket = IncompletePacket;
        return;
    }
    if (len >= 0xff){
        statePacket = IncorrcetPacket;
        return;
    }
    this->flags = QBitArray(64);
    uint64Flags = _flags;
    quint64 bit = 0x01;
    for (int i=0;i < 64; ++i)
    {
        this->flags.setBit(i, _flags & bit);
        bit = bit << 1;
    }
    array.remove(0,len);
    len = array.length();
    if (flags.testBit(F03) && !flags.testBit(F04))
    {
        //Установлен флаг F3, но сброшен F4
        //значит присутствует 16-ти байтное сообщение
        if (len < 16)
        {
            statePacket = IncompletePacket;
            return;
        }
        QTextCodec* prevCodec = QTextCodec::codecForCStrings();
        QTextCodec *cyrillicCodec = QTextCodec::codecForName("Windows-1251");
        QTextCodec::setCodecForCStrings(cyrillicCodec);
        textMessage = QString::fromAscii(array.data(), 16);
        QTextCodec::setCodecForCStrings(prevCodec);
        array.remove(0,16);
    }
    if (!flags.testBit(F03) && flags.testBit(F04))
    {
        //Установлен флаг F4, но сброшен F3
        //значит присутствует 32-ти байтное сообщение
        if (len < 32)
        {
            statePacket = IncompletePacket;
            return;
        }
        QTextCodec* prevCodec = QTextCodec::codecForCStrings();
        QTextCodec *cyrillicCodec = QTextCodec::codecForName("Windows-1251");
        QTextCodec::setCodecForCStrings(cyrillicCodec);
        textMessage = QString::fromAscii(array.data(), 32);
        QTextCodec::setCodecForCStrings(prevCodec);
        array.remove(0,32);
    }

    if (array.length() <= 0) {
        statePacket = IncompletePacket;
        return;
    }
    //если установлен флаг F5 извлекаем массив данных
    if (flags.testBit(F05)) {
        len = array.at(0);
        array.remove(0,1);
        if (array.length() < (len + 1))
        {
            statePacket = IncompletePacket;
            return;
        }

        if (array.length() > (len + 1))
        {
            statePacket = IncorrcetPacket;
            return;
        }
        if (crc) {
            // беда с крк
            statePacket = IncorrcetPacket;
            return;
        }
        //удаляем крк из пакета
        array.remove(array.length() - 1, 1);
    } else {
        //если флат F5 не установлен, то конец пакета, и у нас остался
        //только 1 байт - байт крк
        if (array.length() > 1)
        {
            //слишком большой пакет
            statePacket = IncorrcetPacket;
            return;
        }
        array.clear();
    }

    // до сюда доходим если только пакет, переданый нам, полностью валидный.
    // flags - флаги, numUSK - номер УСК, array - переданый нам массив данных, без числа повторов и крк.
    statePacket = CorrectPacket;
    standartEvent = -1;
    if (flags.testBit(F05)) dataArray = array;
    else dataArray = QByteArray();

    //проверка на приход команды от КПУ
    if (flags.testBit(F05) && flags.testBit(F06) && dataArray.length() >= 3)
    {

        int command = (uchar)array.at(2);
        int typeKpu = (uchar)array.at(1);
        int numKpu = (uchar)array.at(0);
        array.remove(0,3);
        standartEvent = command;
        switch(command){
        case 0x0111:
            // сброс УСК
        {

        }
            break;
        case 0x0c:
            // обнаружен кпу

            // номер КПУ (1 байт)
            // тип КПУ (1 байт)
            // команда (1)
            // состояние контактов (?1 байт), может быть зависимость от типа КПУ

        {

            if (array.length() != 1){
                // что то не так с пакетом!
                // его длинна должна быть равной 1 байтам!
                statePacket = UnknownPacket;
                return;
            }
            kpuType = typeKpu;
            kpuNum = numKpu;
            kpuState = (uchar)array.at(0);
            //kpuSoftVersion = (uchar) array.at(3);
            //kpuHardVersion = (uchar) array.at(4);
            //kpuSerialNumber =  array.at(5) +
            //        (array.at(6) << 8) +
            //        (array.at(7) << 16) +
            //        (array.at(8) << 24);
            return;

        }
            break;
        case 0x0d:
            // неисправен/отключен КПУ
            // номер КПУ (1 байт)
        {
            if (array.length() != 0)
            {
                // что то не так с пакетом!
                // его длинна должна быть равной 1 байту!
                statePacket = UnknownPacket;
                return;
            }
            kpuNum = numKpu;

        }
            break;
        case 0x00:
            // изменение состояния датчиков КПУ

            // номер КПУ (1 байт)
            // тип КПУ (1 байт)
            // команда (0)
            // текущее состояние датчиков
            // предыдущее состояние датчиков
        {
            if (array.length() != 2)
            {
                // что то не так с пакетом!
                // его длинна должна быть равной 4 байтам!
                statePacket = UnknownPacket;
                return;
            }
            kpuType = typeKpu;
            kpuNum = numKpu;
            kpuState = (uchar)array.at(0);
            kpuPrevState = (uchar)array.at(1);
            return;
        }
            break;
        default:
            statePacket = UnknownPacket;
            break;
        }
    }
}

void PacketDecoder::parseResponse(QByteArray response, PacketDecoder::ResponseType type)
{
    statePacket = IncompletePacket;
    // пока 2 типа - ответ на установку времени
    // и просто ответ: номерУСК, 0x00, и крк
    switch (type){
    case TimeSettingResponse: /*ожидаем ответ на время
      1: адрес уск
      2: битовый массив рабочих модулей
      3: байт температуры воздуха
      4: 4 байта замера напряжений
      5: CRC
    */
    {
        QByteArray tempArray;
        tempArray.append(response);
        quint64 numUSK, len;
        int numByte;
        numByte = parseComplexNumber(numUSK, response,0);
        if (!numByte) {
            statePacket = IncompletePacket;
            return;
        }
        //если не правильный формат составного числа - значит явно ошибка
        if (numByte == 0xff) {
            statePacket = IncorrcetPacket;
            return;
        }
        USKNum = numUSK;
        response.remove(0,numByte);
        workingModules.clear();
        workingModules.resize(64);
        quint8 numBit = 0;
        quint8 c1,b;
        while (true){
            if (response.isEmpty()){
                statePacket = IncompletePacket;
                return;
            }
            b = response.at(0);
            response.remove(0,1);
            c1 = b & 0x7f;
            for (int i=0; i<=6; ++i){
                quint8 temp = c1;
                temp = temp >> i;
                temp &= 0x01;
                workingModules.setBit(numBit,temp);
                ++numBit;
            }
            if (b & 0x80) continue;
            break;
        }

        //если оставшийся пакет < 6 байт, то недоприняли
        //eсли > 6 байт - то уже не верный пакет

        if (response.length() < 6) {
            statePacket = IncompletePacket;
            return;
        }

        if (response.length() > 6) {
            //ошибка!!!
            statePacket = IncorrcetPacket;
            return;
        }

        //здесь если приняли весь пакет полностью, ура!
        USKTemperature = (quint8)response.at(0);
        response.remove(0,1);
        USKVoltages = response.left(4);
        len = tempArray.length();
        quint8 crc_cur = 0;
        for (quint64 i=0; i < len; ++i)
        {
            crc_cur += tempArray.at(i);
        }
        if (!crc_cur) {
            //ура!!! все сошлось!!!
            statePacket = CorrectPacket;
            return;
        }
        statePacket = IncorrcetPacket;
        return;
    }
    break;

    case StandartResponse: /*ожидаем простой ответ
      1: адрес уск
      2: два байта, побитовое поле модулей, принявших команду
      3: CRC
    */
    {
        QByteArray tempArray;
        tempArray.append(response);
        quint64 numUSK, len;
        int numByte;
        numByte = parseComplexNumber(numUSK, response,0);
        if (!numByte) {
            statePacket = IncompletePacket;
            return;
        }
        //если не правильный формат составного числа - значит явно ошибка
        if (numByte == 0xff) {
            statePacket = IncorrcetPacket;
            return;
        }
        response.remove(0,numByte);

        if (response.length() < 4) {
            statePacket = IncompletePacket;
            return;
        }

        if (response.length() > 4) {
            //ошибка!!!
            statePacket = IncorrcetPacket;
            return;
        }

        if (response.at(0) != 0)
        {
            statePacket = IncorrcetPacket;
            return;
        }

        //quint8 crc = array.at(1);
        len = tempArray.length();
        quint8 crc_cur = 0;
        for (quint64 i=0; i < len; ++i)
        {
            crc_cur += tempArray.at(i);
        }
        if (!crc_cur) {
            //ура!!! все сошлось!!!
            statePacket = CorrectPacket;
            USKNum = numUSK;
            modules = response.at(1);
            modules += (uchar)response.at(2) * 0x100;
            return;
        }

        statePacket = IncorrcetPacket;
        return;
    }
    break;
    default:
        break;
    }
}

PacketDecoder::StatePacket PacketDecoder::getStatePacket()
{
    return statePacket;
}

int PacketDecoder::getUSKNum()
{
    return (int)USKNum;
}

QBitArray PacketDecoder::getFlags()
{
    return flags;
}

QStringList PacketDecoder::getFlagNames()
{
    QStringList rv;
    for (int i = 0;i < 14; ++i)
        if (flags.testBit(i))
            rv.append(nameOfFlags.value(i));
    return rv;
}

QString PacketDecoder::getTextMessage()
{
    return textMessage;
}

int PacketDecoder::getTemperature()
{
    return USKTemperature;
}

QByteArray PacketDecoder::getVoltages()
{
    return USKVoltages;
}

QBitArray PacketDecoder::getWorkingModules()
{
    return workingModules;
}

quint16 PacketDecoder::getModules() const
{
    return modules;
}

QByteArray PacketDecoder::getArray()
{
    return dataArray;
}

QString PacketDecoder::getCommand(int command)
{
    return nameOfKpuCommand.value(command, QObject::trUtf8("Неизвестная комада"));
}

void PacketDecoder::getNewKPUData(int &type, int &kpuNum, int &state, int &softVersion, int &hardVersion, quint64 &serialNumber)
{
    type = kpuType;
    kpuNum = this->kpuNum;
    state = kpuState;
    softVersion = kpuSoftVersion;
    hardVersion = kpuHardVersion;
    serialNumber = kpuSerialNumber;
}

int PacketDecoder::parseComplexNumber(quint64 &res, QByteArray array, quint64 index)
{
    if (array.isEmpty()) return 0;
    quint64 temp;
    res = 0;
    quint8 iterat = 0;
    while (true)
    {
        if (iterat == 9) return 0xff;
        if ((array.count() - 1) < index) return 0;
        temp = array.at(index);
        temp &= 0x7f;
        temp = temp << (7 * iterat);
        res += temp;
        if (array.at(index) & 0x80) {
            ++index;
            ++iterat;
            continue;
        } else break;
    }
    return iterat+1;

}

void PacketDecoder::addCRCToPacket(QByteArray &packet)
{
    quint8 res = 0;
    for (int i=0;i<packet.size();++i) res += packet[i];
    res = 0 - res;
    packet.append(res);
}

QByteArray PacketDecoder::getSendTimeCommand(int uskNum, const QDateTime &time)
{
    QByteArray array;
    array.append(prepareComplexNumber(uskNum)); //Номер устройства;
    array.append((char)0x40); //установлен флаг F1
    quint8 l,h;
    l = time.time().second()%10;
    h = time.time().second()/10;
    h = h << 4;
    l |= h;
    array.append(l); //секунды

    l = time.time().minute()%10;
    h = time.time().minute()/10;
    h = h << 4;
    l |= h;
    array.append(l); //минуты

    l = time.time().hour()%10;
    h = time.time().hour()/10;
    h = h << 4;
    l |= h;
    array.append(l); //часы
    addCRCToPacket(array);
    return array;
}

QByteArray PacketDecoder::getSendTextMessageCommand(const int &uskNum, const int &msgType, const QString &message)
{
    QByteArray array;
    if (msgType > 1) return array;
    array.append(prepareComplexNumber(uskNum)); //Номер устройства;
    char flags;

    //перед отправкой текста !!!НЕОБХОДИМО!!! привести его в win1251
    //реализуем с помощью QTextCodec
    QTextCodec* prevCodec = QTextCodec::codecForCStrings();
    QTextCodec *cyrillicCodec = QTextCodec::codecForName("Windows-1251");
    QTextCodec::setCodecForCStrings(cyrillicCodec);
    int lenmessage = message.length();
    switch (msgType){
    case 0:
    {
        flags = 0x10; //0x10 байт
        array.append(flags);
        char *buff = new char[0x10];
        for (int i=0; i<0x10; ++i)
            *(buff+i) = 0x20;
        if (lenmessage > 0x10) lenmessage = 0x10;
        memcpy(buff, message.toAscii().data(),lenmessage);
        array.append(buff,0x10);
        delete buff;
    }
        break;
    case 1:

    {
        flags = 0x08; //0x20 байт
        array.append(flags);
        char *buff = new char[0x20];
        for (int i=0; i<0x20; ++i)
            *(buff+i) = 0x20;
        if (lenmessage > 0x20) lenmessage = 0x20;
        memcpy(buff, message.toAscii().data(),lenmessage);
        array.append(buff,0x20);
        delete buff;
    }
        break;
    }
    QTextCodec::setCodecForCStrings(prevCodec);
    addCRCToPacket(array);
    return array;
}

QByteArray PacketDecoder::getChangeRelayContactsCommand(const int &uskNum, const int &kpuNum, const int &kpuType, quint8 contacts, quint8 mask)
{
    QByteArray array;
    array.append(prepareComplexNumber(uskNum));
    array.append(0x06); // F5, F6 флаги
    array.append(0x04); // 4 байта данных
    array.append(kpuType); //тип КПУ
    array.append((char)kpuNum); //номер КПУ
    array.append(0x03); // команда на включение КПУ
    array.append((char)contacts); // состояние контактов
    addCRCToPacket(array);
    return array;
}

QByteArray PacketDecoder::getSendByteArrayOnKpuCommand(const int &uskNum, const int &kpuNum, int kpuType, const QByteArray &byteArray)
{
    QByteArray array;
    array.append(prepareComplexNumber(uskNum));
    array.append(0x06); // F5 и F6
    array.append((char)(byteArray.length() + 3)); //длинна массива
    array.append(byteArray); // сам массив, кот. нужно передать.
    addCRCToPacket(array);
    return array;
}

QByteArray PacketDecoder::getResetUskCommand(const int &uskNum)
{
    QByteArray array;
    array.append(prepareComplexNumber(uskNum));
    array.append(0x18);
    array.append((char)0x00);
    addCRCToPacket(array);
    return array;
}

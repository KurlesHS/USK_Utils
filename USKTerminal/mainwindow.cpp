#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    packetInfoWindows = 0;
    setWindowTitle(trUtf8("USK Terminal v2.06 alpha"));
    ui->tableWidget->verticalHeader()->setVisible(true);
    ui->tableWidget->horizontalHeader()->setVisible(true);

    QFont font = ui->tableWidget->horizontalHeader()->font();
    font.setBold(true);
    resRegArray.clear();
    ui->tableWidget->horizontalHeader()->setFont(font);
    ui->pushButtonClosePort->setEnabled(false);
    ui->pushButtonOpenPort->setEnabled(true);
    ui->pushButtonSendPacket->setEnabled(false);
    ui->pushButtonCancelSendPacket->setEnabled(false);

    SerialDeviceEnumerator *sde = SerialDeviceEnumerator::instance();
    QStringList listOfSerialPort = sde->devicesAvailable();
    if (listOfSerialPort.isEmpty()) {
        QMessageBox::critical(this,"Ошибка!!!", "Не присутствует ни одного СОМ порта в системе!!!");
        QCoreApplication::exit(0);
    }
    serialThread = 0;
    onEnableRepeatRacketButtonPushed();
    connect(ui->checkBoxCRC,SIGNAL(clicked()),SLOT(onIncludeCRCCheckBoxChanged()));

    connect(ui->pushButtonClosePort,SIGNAL(clicked()),this,SLOT(onClosePortButtonPushed()));
    connect(ui->pushButtonOpenPort,SIGNAL(clicked()),this,SLOT(onOpenPortButtonPushed()));
    connect(ui->pushButtonSendPacket,SIGNAL(clicked()),this,SLOT(onSendPacketButtonPushed()));
    connect(ui->pushButtonClearEditRegion,SIGNAL(clicked()),this,SLOT(onClearSendRegionButtonPushed()));
    connect(ui->pushButtonClearReceiveRegion,SIGNAL(clicked()),this,SLOT(onClearReceiveRegionButtonPushed()));
    connect(ui->pushButtonCancelSendPacket, SIGNAL(clicked()), this, SLOT(onCancelSendPacketButtonPushed()));

    connect(ui->radioButtonTerminalMode, SIGNAL(toggled(bool)), this, SLOT(onTerminalModeChanged(bool)));

    connect(&timer, SIGNAL(timeout()),this, SLOT(onTimerEvent()));

    connect(ui->menuView, SIGNAL(aboutToShow()), this, SLOT(onMenuViewAboutToShow()));
    connect(ui->actionShowParserWindows, SIGNAL(changed()), this, SLOT(onShowPacketInfoWindowMenu()));

    connect(ui->pushButtonEnableRepeatPacket,  SIGNAL(clicked()), this, SLOT(onEnableRepeatRacketButtonPushed()));
    connect(ui->spinBoxRepeatPacketDelay, SIGNAL(valueChanged(int)), this, SLOT(onValueOfDelayBetweenSendPacketChanged(int)));


    timer.setParent(this);
    timer.start(20);
    ui->groupBoxRepeatPacket->setEnabled(false);

    ui->SerialPortComboBox->addItems(listOfSerialPort);
    serialPort = new AbstractSerial(this);
    ui->flowContorlComboBox->addItems(serialPort->listFlowControl());
    ui->flowContorlComboBox->setCurrentIndex(1);
    ui->baudRateComboBox->addItems(serialPort->listBaudRate());
    ui->baudRateComboBox->setCurrentIndex(7);
    ui->parityComboBox->addItems(serialPort->listParity());
    ui->parityComboBox->setCurrentIndex(1);
    ui->stopBitsComboBox->addItems(serialPort->listStopBits());
    ui->stopBitsComboBox->setCurrentIndex(1);
    ui->dataBitsComboBox->addItems(serialPort->listDataBits());
    ui->dataBitsComboBox->setCurrentIndex(4);
    //int column = ui->tableWidget->columnCount();
    int column = ColumnCount;
    ui->tableWidget->setColumnCount(column);
    for (int i=0; i < column; ++i)
    {

        MyLineEdit *edit = new MyLineEdit(this);

        ui->tableWidget->horizontalHeader()->resizeSection(i,ui->tableWidget->fontMetrics().width("000000000")+2);
        ui->tableWidget->model()->setHeaderData(i, Qt::Horizontal,QString::number(i));
        QRegExp rx("^[0-9]{3}$");
        QValidator *validator =new QRegExpValidator (rx,this);
        edit->setValidator(validator);
        edit->setProperty("row", quint16(0));
        edit->setProperty("column", quint16(i));
        edit->setProperty("prevText", QString("0"));
        edit->setText("0");
        ui->tableWidget->setCellWidget(0, i, edit);
        connect(edit,SIGNAL(textEdited(QString)),this,SLOT(onCellEdited(QString)));
        connect(edit,SIGNAL(focusOut()),this,SLOT(onFocusChanged()));

        edit = new MyLineEdit(this);
        rx.setPattern("^[0-9a-fA-f]{2}$");
        validator = new QRegExpValidator(rx,this);
        edit->setValidator(validator);
        edit->setProperty("row", quint16(1));
        edit->setProperty("column", quint16(i));
        edit->setProperty("prevText", QString("00"));
        edit->setText("00");
        ui->tableWidget->setCellWidget(1, i, edit);
        connect(edit,SIGNAL(textEdited(QString)),this,SLOT(onCellEdited(QString)));
        connect(edit,SIGNAL(focusOut()),this,SLOT(onFocusChanged()));

        edit = new MyLineEdit(this);
        rx.setPattern("^[01]{8}$");
        validator = new QRegExpValidator(rx,this);
        edit->setValidator(validator);
        edit->setProperty("row", quint16(2));
        edit->setProperty("column", quint16(i));
        edit->setProperty("prevText", QString("00000000"));
        edit->setText("00000000");
        ui->tableWidget->setCellWidget(2, i, edit);
        connect(edit,SIGNAL(textEdited(QString)),this,SLOT(onCellEdited(QString)));
        connect(edit,SIGNAL(focusOut()),this,SLOT(onFocusChanged()));

        edit = new MyLineEdit(this);
        rx.setPattern("^.{1}$");
        validator = new QRegExpValidator(rx,this);
        edit->setValidator(validator);
        edit->setProperty("row", quint16(3));
        edit->setProperty("column", quint16(i));
        edit->setProperty("prevText", QString(""));
        edit->setText("");
        ui->tableWidget->setCellWidget(3, i, edit);
        connect(edit,SIGNAL(textEdited(QString)),this,SLOT(onCellEdited(QString)));
        connect(edit,SIGNAL(focusOut()),this,SLOT(onFocusChanged()));

        edit = new MyLineEdit(this);
        rx.setPattern("^[01]{1}$");
        validator = new QRegExpValidator(rx,this);
        edit->setValidator(validator);
        edit->setProperty("row", quint16(4));
        edit->setProperty("column", quint16(i));
        edit->setProperty("prevText", QString("0"));
        edit->setText("0");
        ui->tableWidget->setCellWidget(4, i, edit);
        connect(edit,SIGNAL(textEdited(QString)),this,SLOT(onCellEdited(QString)));
        connect(edit,SIGNAL(focusOut()),this,SLOT(onFocusChanged()));
    }
    QSettings setting(QString("USKTerminal.ini"),  QSettings::IniFormat);
    setting.beginGroup("MainSettings");
    if (setting.contains("PacketData"))
    {
        QByteArray data = setting.value("PacketData").toByteArray();
        if (data.size()) {
            if (data.size() > ColumnCount) data.resize(ColumnCount);
            for (int i = 0; i < column; ++i)
            {
                if (i >= data.size()) break;
                updateDec(i, (uchar)data.at(i));
                updateHex(i, (uchar)data.at(i));
                updateBin(i, (uchar)data.at(i));
                updateChar(i, (uchar)data.at(i));
            }
            MyLineEdit* le = qobject_cast<MyLineEdit*>(ui->tableWidget->cellWidget(4,data.size() - 1));
            le->setText("1");
            updateSendRegion();
        }
    }
    if (setting.contains("PortName"))
    {
        int index = ui->SerialPortComboBox->findText(setting.value("PortName").toString());
        if (index >= 0) ui->SerialPortComboBox->setCurrentIndex(index);
        else ui->SerialPortComboBox->lineEdit()->setText(setting.value("PortName").toString());
    }
    if (setting.contains("IncludeCRC"))
        ui->checkBoxCRC->setChecked(setting.value("IncludeCRC").toBool());
    if (setting.contains("CRCMethod"))
        ui->checkBoxCRCMethod->setChecked(setting.value("CRCMethod").toBool());
    if (setting.contains("BaudRate"))
        ui->baudRateComboBox->setCurrentIndex(setting.value("BaudRate").toInt());
    if (setting.contains("FlowControm"))
        ui->flowContorlComboBox->setCurrentIndex(setting.value("FlowControm").toInt());
    if (setting.contains("Parity"))
        ui->parityComboBox->setCurrentIndex(setting.value("Parity").toInt());
    if (setting.contains("StopBits"))
        ui->stopBitsComboBox->setCurrentIndex(setting.value("StopBits").toInt());
    if (setting.contains("DataBits"))
        ui->dataBitsComboBox->setCurrentIndex(setting.value("DataBits").toInt());

    setting.endGroup();
    delete serialPort;
    serialPort = 0;
}

MainWindow::~MainWindow()
{
    QSettings setting(QString("USKTerminal.ini"),  QSettings::IniFormat);
    setting.beginGroup("MainSettings");
    if (ui->checkBoxCRC->isChecked())
    {
        array.truncate(array.size()-1);
    }
    setting.setValue("PacketData", array);
    setting.setValue("PortName", ui->SerialPortComboBox->currentText());
    setting.setValue("IncludeCRC", ui->checkBoxCRC->isChecked());
    setting.setValue("CRCMethod", ui->checkBoxCRCMethod->isChecked());
    setting.setValue("BaudRate", ui->baudRateComboBox->currentIndex());
    setting.setValue("FlowControm", ui->flowContorlComboBox->currentIndex());
    setting.setValue("Parity", ui->parityComboBox->currentIndex());
    setting.setValue("StopBits", ui->stopBitsComboBox->currentIndex());
    setting.setValue("DataBits", ui->dataBitsComboBox->currentIndex());
    setting.endGroup();
    if (serialThread) {
        serialThread->terminate();
        serialThread->wait();
        delete serialThread;
        serialThread = 0;
    }

    if (serialPort){
        if (serialPort->isOpen())
        {
            serialPort->close();
            delete serialPort;
            serialPort = 0;
        }
    }

    if (packetInfoWindows)
    {
        packetInfoWindows->close();
        packetInfoWindows = 0;
    }
    delete ui;
}

void MainWindow::OnTimeOutReached()
{
    addLogMessage(trUtf8("Таймаут!"));
    ui->pushButtonSendPacket->setEnabled(true);
}

//слот для обработки поступивших данных
void MainWindow::onDataReceived(QByteArray array)
{
    //если активен режим терминала - все просто,
    //добвавляем к уже существующему массиву данных
    //новополученные данные
    if (ui->radioButtonTerminalMode->isChecked())
    {
        resRegArray.append(array);
        ui->lineEditResBin->setText(getBinString(resRegArray));
        ui->lineEditResDec->setText(getDecString(resRegArray));
        ui->lineEditResHex->setText(getHexString(resRegArray));
        ui->lineEditResChar->setText(getCharString(resRegArray));
        serialThread->startWaitingData();
        return;
    }

    //попытка отпарсить пришедший пакет
    packetDecoder.parsePacket(array);
    int state = packetDecoder.getStatePacket();
    switch (state)
    {
    //неполный пакет - продолжаем жадть остаток пакета
    case PacketDecoder::IncompletePacket:
    {
        serialThread->continueWaitingData();
        return;
    }
        break;
    //неверный пакет - курим 200мс и начинаем ждать
    //новый пакет данных
    case  PacketDecoder::IncorrcetPacket:
    {
        addLogMessage(trUtf8("Принят ошибочный пакет!"));
        resRegArray.append(array);
        ui->lineEditResBin->setText(getBinString(resRegArray));
        ui->lineEditResDec->setText(getDecString(resRegArray));
        ui->lineEditResHex->setText(getHexString(resRegArray));
        ui->lineEditResChar->setText(getCharString(resRegArray));
        serialThread->startWaitingDataWithDelay(200);

        return;
    }
        break;
    //пакет валидный: начинаем разбирать его
    case PacketDecoder::CorrectPacket:
    {
        //выведем его на экран
        resRegArray.append(array);
        ui->lineEditResBin->setText(getBinString(resRegArray));
        ui->lineEditResDec->setText(getDecString(resRegArray));
        ui->lineEditResHex->setText(getHexString(resRegArray));
        ui->lineEditResChar->setText(getCharString(resRegArray));
        addLogMessage(trUtf8("Корректрый пакет!"));
        //пока без полной разборки
        //ToDo: добавить полный парсинг пакета

        //...

        if (packetInfoWindows)
        {
            QString string = trUtf8("%0: Принят пакет, его данные:").arg(QTime::currentTime().toString("hh:mm:ss"));
            packetInfoWindows->addText(string, "RED");
            string = trUtf8("<br/>Hex: %0<br/>Dec: %1<br/>Char: %2").arg(getHexString(array)).arg(getDecString(array)).arg(getCharString(array));
            packetInfoWindows->addText(string, "gray");
            string = trUtf8("<br/>Номер УСК: %0").arg(packetDecoder.getUSKNum());
            packetInfoWindows->addText(string, "darkred");
            packetInfoWindows->addText(trUtf8("<br/>Установленные флаги: <br/>"), "black");
            foreach(QString flagName, packetDecoder.getFlagNames())
            {
                packetInfoWindows->addText(flagName, "black");
            }

            QBitArray flags = packetDecoder.getFlags();
            if (flags.testBit(PacketDecoder::F05)) //есть ли массив данных
            {
                packetInfoWindows->addText(trUtf8("<br/>Присутствует массив данных:"));
                packetInfoWindows->addText(trUtf8("<br/>Hex: %0").arg(getHexString(packetDecoder.getArray())));
                packetInfoWindows->addText(trUtf8("Dec: %0").arg(getDecString(packetDecoder.getArray())));
                packetInfoWindows->addText(trUtf8("Char: %0").arg(getCharString(packetDecoder.getArray())));
            }

            if ((flags.testBit(PacketDecoder::F04) && !flags.testBit(PacketDecoder::F03))
                    || (!flags.testBit(PacketDecoder::F04) && flags.testBit(PacketDecoder::F03)))
            {
                //присутствует строка символов
                packetInfoWindows->addText(trUtf8("<br/>Присутствует текстовая строка: %0").arg(packetDecoder.getTextMessage()));
            }

            if (flags.testBit(PacketDecoder::F03) && flags.testBit(PacketDecoder::F04) && flags.testBit(PacketDecoder::F05)) //стандартная команда
            {
                packetInfoWindows->addText(trUtf8("<br/>Установлены флаги f3, f4, f5, f6"));
                packetInfoWindows->addText(trUtf8("Команда от КПУ: %0").arg(packetDecoder.getCommand(packetDecoder.getArray().at(0))));
                switch (packetDecoder.getArray().at(0))
                {
                case 0:
                    break;
                case 1:
                    //обнаружен кпу
                {
                    int type;
                    int kpuNum;
                    int state;
                    int softVersion;
                    int hardVersion;
                    quint64 serialNumber;
                    packetDecoder.getNewKPUData(type, kpuNum, state, softVersion, hardVersion, serialNumber);
                    packetInfoWindows->addText(trUtf8("<br/>Номер КПУ: %0;<br/>Тип КПУ:%1;<br/>Сосотояние контактов:%2"
                                                      "<br/>Версия ПО:%3<br>Версия платы:%4"
                                                      "<br/>Серийный номер:%5")
                                               .arg(kpuNum)
                                               .arg(type)
                                               .arg(state)
                                               .arg(softVersion)
                                               .arg(hardVersion)
                                               .arg(serialNumber));
                }
                    break;
                case 2:
                    //отключен/неисправен КПУ
                    packetInfoWindows->addText(trUtf8("<br/>Номер КПУ:%0").arg(packetDecoder.getKPUNum()));
                    break;
                case 3:
                    // изменение состояния датчиков КПУ
                    packetInfoWindows->addText(trUtf8("<br/>Номер КПУ:%0"
                                                      "<br/>Тип КПУ:%1"
                                                      "<br/>Состояние контактов:%2"
                                                      "<br/>Предыдущее состояние контактов:%3")
                                               .arg(packetDecoder.getKPUNum())
                                               .arg(packetDecoder.getKPUType())
                                               .arg(packetDecoder.getKPUState())
                                               .arg(packetDecoder.getPrevKPUState()));
                    break;
                default:
                    break;
                }
            }
            packetInfoWindows->addText(trUtf8("<br/>------------Конец пакета--------------<br/>"), "red");
        }

        QByteArray response;

        int numUsk = packetDecoder.getUSKNum();
        response.append(MyWidget::prepareComplexNumber(numUsk));
        response.append((char)0x00);
        quint8 crk = 0;
        foreach (char byte, response)
            crk += (quint8)byte;
        response.append((char)crk);
        qDebug() << "отклик перед отправкой:" << response;
        serialThread->sendRespons(response);
    }
        break;
    case PacketDecoder::UnknownPacket:
    {

        //выведем его на экран
        resRegArray.append(array);
        ui->lineEditResBin->setText(getBinString(resRegArray));
        ui->lineEditResDec->setText(getDecString(resRegArray));
        ui->lineEditResHex->setText(getHexString(resRegArray));
        ui->lineEditResChar->setText(getCharString(resRegArray));
        addLogMessage(trUtf8("неизвестный пакет!"));


        //...

        QByteArray response;

        int numUsk = packetDecoder.getUSKNum();
        response.append(MyWidget::prepareComplexNumber(numUsk));
        response.append((char)0x00);
        quint8 crk = 0;
        foreach (char byte, response)
            crk += (quint8)byte;
        response.append((char)crk);
        serialThread->sendRespons(response);
    }
        break;
    }
}

//слот для обработки отклика от УСК
void MainWindow::onResponseReceived(QByteArray array)
{
    packetDecoder.parseResponse(array);
    int state = packetDecoder.getStatePacket();
    bool isStandartResponse = true;
    if (state == PacketDecoder::IncorrcetPacket)
    {
        //проверяем на валидность отклика
        //если отклик не валидный, то пробуем парсить ответ на
        //команду установки времени
        isStandartResponse = false;
        packetDecoder.parseResponse(array, PacketDecoder::TimeSettingResponse);
        state = packetDecoder.getStatePacket();
    }
    switch(state)
    {
    case PacketDecoder::IncorrcetPacket:
    {
        //ошибочный отклик:
        serialThread->onIncorrectResponseReceived();
        addLogMessage(trUtf8("Неверный отклик"));

        return;
    }
        break;
    case PacketDecoder::IncompletePacket:
    {
        serialThread->continueWaitingResponse();
    }
        break;
    case PacketDecoder::CorrectPacket:
    {
        serialThread->onCorrectResponseReceived();
        addLogMessage(trUtf8("Получен %1 от УСК №%0").arg(packetDecoder.getUSKNum())
                      .arg(isStandartResponse ? trUtf8("простой отклик") : trUtf8("отклик на команду установки времени")));

        //заполним данные
        resRegArray.append(array);
        ui->lineEditResBin->setText(getBinString(resRegArray));
        ui->lineEditResDec->setText(getDecString(resRegArray));
        ui->lineEditResHex->setText(getHexString(resRegArray));
        ui->lineEditResChar->setText(getCharString(resRegArray));
        serialThread->onCorrectResponseReceived();
    }
        break;
    }
}

void MainWindow::onClearReceiveRegionButtonPushed()
{
    ui->lineEditResBin->setText("");
    ui->lineEditResDec->setText("");
    ui->lineEditResHex->setText("");
    ui->lineEditResChar->setText("");
    resRegArray.clear();
}

void MainWindow::onClearSendRegionButtonPushed()
{
    int countColumn = ui->tableWidget->columnCount();
    for (int i = 0; i < countColumn; ++i){
        updateDec(i,0);
        updateHex(i,0);
        updateBin(i,0);
        updateChar(i,0);
        MyLineEdit* le = qobject_cast<MyLineEdit*>(ui->tableWidget->cellWidget(4,i));
        if (le) le->setText("0");
    }
    updateSendRegion();
}

void MainWindow::fillSendRegionFromArray(QByteArray array)
{
    if (array.count() == 0 ) return;
    int countColumn = ui->tableWidget->columnCount();
    for (int i = 0; i < countColumn; ++i){
        updateDec(i,0);
        updateHex(i,0);
        updateBin(i,0);
        updateChar(i,0);
        MyLineEdit* le = qobject_cast<MyLineEdit*>(ui->tableWidget->cellWidget(4,i));
        if (le) le->setText("0");
    }

    if (array.count() > (countColumn - 1)) array.resize(countColumn - 1);
    for (int i = 0; i < array.count(); ++i)
    {
        quint8 num = array.at(i);
        updateDec(i,num);
        updateHex(i,num);
        updateBin(i,num);
        updateChar(i,num);
    }
    MyLineEdit* le = qobject_cast<MyLineEdit*>(ui->tableWidget->cellWidget(4,array.count() - 1));
    if (le) le->setText("1");


    updateSendRegion();

}

void MainWindow::addLogMessage(QString message, int type)
{
    message.prepend(QTime::currentTime().toString("[hh:mm:ss] "));
    if (type) message.prepend("<FONT color=\"#3366FF\">");
    else message.prepend("<FONT color=\"#CC33CC\">");
    message.append("</FONT>");
    ui->textEditLogEvents->append(message);

}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    if (packetInfoWindows)
    {
        delete packetInfoWindows;
        packetInfoWindows = 0;
    }
    QWidget::closeEvent(ev);
}

void MainWindow::onClosePortButtonPushed()
{
    ui->pushButtonOpenPort->setEnabled(true);
    ui->pushButtonClosePort->setEnabled(false);
    ui->SerialPortComboBox->setEnabled(true);
    ui->pushButtonSendPacket->setEnabled(false);
    ui->pushButtonCancelSendPacket->setEnabled(false);
    ui->baudRateComboBox->setEnabled(true);
    ui->flowContorlComboBox->setEnabled(true);
    ui->dataBitsComboBox->setEnabled(true);
    ui->stopBitsComboBox->setEnabled(true);
    ui->parityComboBox->setEnabled(true);


    serialThread->terminate();
    serialThread->wait();
    serialPort->close();
    delete serialPort;
    delete serialThread;
    serialPort = 0;
    serialThread = 0;
}

void MainWindow::onOpenPortButtonPushed()
{

    if (serialPort) return;

    if (serialThread) return;

    serialPort = new AbstractSerial();
    serialPort->setDeviceName(ui->SerialPortComboBox->currentText());
    if (!serialPort->open(AbstractSerial::ReadWrite)){
        setUft8();
        QMessageBox::warning(this,"Не получилось открыть", ui->SerialPortComboBox->currentText().append(" порт."));
        setWin1251();
        delete serialPort;
        serialPort = 0;
        return;
    }

    serialPort->setBaudRate(ui->baudRateComboBox->currentText());
    serialPort->setDataBits(ui->dataBitsComboBox->currentText());
    serialPort->setFlowControl(ui->flowContorlComboBox->currentText());
    serialPort->setStopBits(ui->stopBitsComboBox->currentText());
    serialPort->setParity(ui->parityComboBox->currentText());

    ui->pushButtonOpenPort->setEnabled(false);
    ui->pushButtonClosePort->setEnabled(true);
    ui->pushButtonSendPacket->setEnabled(true);
    ui->pushButtonCancelSendPacket->setEnabled(true);
    ui->SerialPortComboBox->setEnabled(false);
    ui->baudRateComboBox->setEnabled(false);
    ui->flowContorlComboBox->setEnabled(false);
    ui->dataBitsComboBox->setEnabled(false);
    ui->stopBitsComboBox->setEnabled(false);
    ui->parityComboBox->setEnabled(false);
    serialThread = new SerialThread;
    serialThread->setSerialPortDriver(serialPort);
    //serialPort->moveToThread(serialThread);

    connect(serialThread,SIGNAL(dataReceived(QByteArray)),this,SLOT(onDataReceived(QByteArray)));
    connect(serialThread, SIGNAL(responseReceived(QByteArray)), this, SLOT(onResponseReceived(QByteArray)));
    connect(serialThread,SIGNAL(timeOutHappensWhileWaitingResponse()),this,SLOT(OnTimeOutReached()));
    connect(serialThread, SIGNAL(timeOutHappensWhileWaitingData()), this, SLOT(OnTimeOutReached()));
    connect(serialThread, SIGNAL(sendPacketSuccess()), this, SLOT(onEndTransmitPacket()));




    serialThread->start();
}

void MainWindow::onSendPacketButtonPushed()
{
    if (!serialThread) return;
    if (array.isEmpty()) return;
    serialThread->setPacketDecoderMode(ui->radioButtonParserPacketMode->isChecked());
    serialThread->setUseRepeatPacket(ui->pushButtonEnableRepeatPacket->isChecked());
    serialThread->setDelayBetweenSendPacket(ui->spinBoxRepeatPacketDelay->value());
    //if (ui->radioButtonParserPacketMode->isChecked()) ui->pushButtonSendPacket->setEnabled(false);
    serialThread->sendPacket(array);
    qDebug(array);
}

void MainWindow::onSendPacketButtonPushed(QByteArray packet)
{
    if (!packet.size()) return;
    fillSendRegionFromArray(packet);
    onSendPacketButtonPushed();
}

void MainWindow::onCancelSendPacketButtonPushed()
{
    if (!serialThread) return;
    serialThread->setUseRepeatPacket(false);
}

void MainWindow::onCellChanged(int row, int column)
{
    return;
}

void MainWindow::onRtsChanged(bool status)
{
    //ui->radioButtonRTS->setChecked(status);
}

void MainWindow::onIncludeCRCCheckBoxChanged()
{
    updateSendRegion();
}

void MainWindow::onFocusChanged()
{
    MyLineEdit *le, *le2;
    le = qobject_cast<MyLineEdit*>(sender());
    if (!le) return;
    le2 = qobject_cast<MyLineEdit*>(ui->tableWidget->cellWidget(0,le->property("column").toInt()));
    int num = le2->text().toInt();
    switch (le->property("row").toInt()){
    case 0:
        updateDec(le->property("column").toInt(), num);
        break;
    case 1:
        updateHex(le->property("column").toInt(), num);
        break;
    case 2:
        updateBin(le->property("column").toInt(), num);
        break;
    case 3:
        updateChar(le->property("column").toInt(), num);
        break;
    case 4:
        if (le->text() == "") le->setText("0");
        break;
    default:
        break;
    }
}

void MainWindow::onCellEdited(QString str)
{
    if (!sender()) return;
    int tableColumnCount = ui->tableWidget->columnCount();
    MyLineEdit* lineedit = qobject_cast<MyLineEdit*>(sender());
    int row = lineedit->property("row").toInt();
    int column = lineedit->property("column").toInt();
    if (row == 1) str = str.toUpper();
    quint16 num = 0;
    if (row == 0){
        int cur = lineedit->cursorPosition();
        num = str.toInt();
        if (num >= 256) {
            str = lineedit->property("prevText").toString();
            --cur;
            lineedit->setText(str);
            lineedit->setCursorPosition(cur);
        }
        lineedit->setProperty("prevText", str);
    }
    if (row == 1) lineedit->setText(str);

    switch (row){
    case 0:
        num = str.toInt();
        break;
    case 1:
        num = str.toInt(0,16);
        break;
    case 2:
        num = str.toInt(0,2);
        break;
    case 3:
        num = (unsigned char)str.at(0).toAscii();
        break;
    default:
        num = 0;
        break;
    }

    switch (row){
    case 0:{
        updateHex(column,num);
        updateBin(column,num);
        updateChar(column,num);
    }
    break;

    case 1:{
        updateDec(column,num);
        updateBin(column,num);
        updateChar(column,num);
    }
    break;

    case 2:{
        updateDec(column,num);
        updateHex(column,num);
        updateChar(column,num);
    }
    break;

    case 3:{
        updateDec(column,num);
        updateHex(column,num);
        updateBin(column,num);
    }
    break;

    default:
        break;

    }

    if (row == 4) {
        if (str == "1")
            for (int i = 0; i < tableColumnCount; ++i)
            {
                MyLineEdit* le = qobject_cast<MyLineEdit*>(ui->tableWidget->cellWidget(4,i));
                le->setText("0");
            }
        MyLineEdit* le = qobject_cast<MyLineEdit*>(ui->tableWidget->cellWidget(4,column));
        le->setText(str);
    }
    updateSendRegion();
}

QString MainWindow::getHexString(QByteArray array)
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

QString MainWindow::getDecString(QByteArray array)
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

QString MainWindow::getBinString(QByteArray array)
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

QString MainWindow::getCharString(QByteArray array)
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


void MainWindow::updateSendRegion()
{
    int tableColumnCount = ui->tableWidget->columnCount();
    queue.clear();
    array.clear();
    for (int i = 0; i < tableColumnCount; ++i){
        MyLineEdit* le = qobject_cast<MyLineEdit*>(ui->tableWidget->cellWidget(4,i));
        if (le->text() == "1") {
            quint8 crc = 0;
            char c;
            for (int i2 = 0; i2 < tableColumnCount;++i2)
            {
                QLineEdit* le = qobject_cast<MyLineEdit*>(ui->tableWidget->cellWidget(0,i2));
                le->text() == "" ? c = 0 : c = le->text().toInt();
                crc += c;
                queue.enqueue(c);
                array.append(c);
                le = qobject_cast<MyLineEdit*>(ui->tableWidget->cellWidget(4,i2));
                if (le->text() == "1") break;
            }
            if (ui->checkBoxCRC->isChecked())
            {
                if (!ui->checkBoxCRCMethod->isChecked())
                    crc = 0x00 - crc;
                queue.enqueue(crc);
                array.append(crc);
            }
            QString str = QString::number(crc,16);
            str = str.toUpper();
            if (crc <= 15) str = "0" + str;
            ui->lineEditSendCRC->setText(str);
            break;
        }
    }

    if (queue.isEmpty())
    {
        ui->lineEditSendBin->setText("");
        ui->lineEditSendChar->setText("");
        ui->lineEditSendCRC->setText("");
        ui->lineEditSendDec->setText("");
        ui->lineEditSendHex->setText("");
        return;
    }
    ui->lineEditSendHex->setText(getHexString(array));
    ui->lineEditSendBin->setText(getBinString(array));
    ui->lineEditSendDec->setText(getDecString(array));
    ui->lineEditSendChar->setText(getCharString(array));
}

void MainWindow::onTimerEvent()
{
    if (!serialPort) {
        setGUIRtsCts(false, false);
        return;
    }
    if (!serialPort->isOpen()){
        setGUIRtsCts(false, false);
        return;
    }
    int line = serialPort->lineStatus();
    setGUIRtsCts(line & AbstractSerial::LineRTS, line & AbstractSerial::LineCTS);
    if (serialThread)
    {
        ui->labelStatus->setText(trUtf8("статус: %0").arg(serialThread->getCurrentSMStatus()));
    }


}

void  MainWindow::setGUIRtsCts(bool rts, bool cts)
{
    ui->labelRTS->setEnabled(rts);
    ui->labelCTS->setEnabled(cts);
}

void MainWindow::updateDec(int column, int num)
{
    MyLineEdit* lineEdit = qobject_cast<MyLineEdit*>(ui->tableWidget->cellWidget(0,column));
    if (!lineEdit) return;
    QString str = QString::number(num);
    lineEdit->setProperty("prevText",str);
    lineEdit->setText(str);
}

void MainWindow::updateHex(int column, int num)
{
    MyLineEdit* lineEdit = qobject_cast<MyLineEdit*>(ui->tableWidget->cellWidget(1,column));
    if (!lineEdit) return;
    QString str = QString::number(num,16);
    str = str.toUpper();
    if (num <= 15) str = "0" + str;
    lineEdit->setProperty("prevText",str);
    lineEdit->setText(str);
}

void MainWindow::updateBin(int column, int num)
{
    MyLineEdit* lineEdit = qobject_cast<MyLineEdit*>(ui->tableWidget->cellWidget(2,column));
    if (!lineEdit) return;
    QString str;
    int mask = 0x80;
    for (int i = 0;i <= 7 ;++i){
        mask & num ? str += "1" : str += "0";
        mask = mask >> 1;
    }
    lineEdit->setProperty("prevText",str);
    lineEdit->setText(str);
}

void MainWindow::updateChar(int column, int num)
{
    MyLineEdit* lineEdit = qobject_cast<MyLineEdit*>(ui->tableWidget->cellWidget(3,column));
    if (!lineEdit) return;
    char c = (char) num;
    QString str = QString::fromAscii(&c, 1);
    lineEdit->setProperty("prevText",str);
    lineEdit->setText(str);
}

void MainWindow::setWin1251()
{
    QTextCodec *cyrillicCodec = QTextCodec::codecForName("Windows-1251");
    QTextCodec::setCodecForCStrings(cyrillicCodec);
}

void MainWindow::setUft8()
{
    QTextCodec *cyrillicCodec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForCStrings(cyrillicCodec);
}

void MainWindow::on_pushButtonPacket_clicked()
{
    SelectPacketDialog dlg;
    connect(&dlg, SIGNAL(cancelSendPacketButtonPushed()), this, SLOT(onCancelSendPacketButtonPushed()));
    connect(&dlg, SIGNAL(sendPacketButtonPushed(QByteArray)), this, SLOT(onSendPacketButtonPushed(QByteArray)));
    dlg.setPortIsOpen(serialThread);
    if (!dlg.exec()) return;
    QByteArray tempArray = dlg.getData();
    if (tempArray.size() > ColumnCount) tempArray.resize(ColumnCount);
    fillSendRegionFromArray(tempArray);
}

void MainWindow::onEndTransmitPacket()
{
    ui->pushButtonSendPacket->setEnabled(true);
}

void MainWindow::onTerminalModeChanged(bool terminalModeSelected)
{
    ui->groupBoxRepeatPacket->setEnabled(!terminalModeSelected);
    if (serialThread)
        serialThread->setPacketDecoderMode(ui->radioButtonParserPacketMode->isChecked());
    if (terminalModeSelected)
    {
        if (packetInfoWindows)
        {
            packetInfoWindows->close();
            packetInfoWindows = 0;
        }
    } else
    {
        if (!packetInfoWindows)
        {
            packetInfoWindows = new PacketInfoWindows;
            connect(packetInfoWindows, SIGNAL(destroyed()), this, SLOT(onPacketInfoWindowsDestroyed()));
            packetInfoWindows->show();
        }
    }
}

void MainWindow::onPacketInfoWindowsDestroyed()
{
    packetInfoWindows = 0;
}

void MainWindow::onMenuViewAboutToShow()
{
    ui->actionShowParserWindows->setChecked(packetInfoWindows);
    ui->actionShowParserWindows->setEnabled(ui->radioButtonParserPacketMode->isChecked());
}

void MainWindow::onShowPacketInfoWindowMenu()
{
    if (ui->actionShowParserWindows->isChecked())
    {
        if (packetInfoWindows) return;
        packetInfoWindows = new PacketInfoWindows;
        packetInfoWindows->show();
    } else
    {
        if (!packetInfoWindows) return;
        delete packetInfoWindows;
        packetInfoWindows = 0;
    }
    QVector<QString> v;
    foreach(QString str, v)
    {
        qDebug() << str;
    }

}

void MainWindow::onEnableRepeatRacketButtonPushed()
{
    ui->spinBoxRepeatPacketDelay->setEnabled(ui->pushButtonEnableRepeatPacket->isChecked());
    QString textOnButton;
    ui->pushButtonEnableRepeatPacket->isChecked() ? textOnButton = trUtf8("Вкл.") : textOnButton = trUtf8("Выкл.");
    ui->pushButtonEnableRepeatPacket->setText(textOnButton);
    if (!serialThread) return;
    serialThread->setUseRepeatPacket(ui->pushButtonEnableRepeatPacket->isChecked());
    serialThread->setDelayBetweenSendPacket(ui->spinBoxRepeatPacketDelay->value());
}

void MainWindow::onValueOfDelayBetweenSendPacketChanged(int delay)
{
    if (!serialThread) return;
    serialThread->setDelayBetweenSendPacket(delay);
}


void MainWindow::onWrongResponseReceived(QByteArray response)
{
    //здесь QByteArray с неверный откликом
    //после 2-х попыток
}

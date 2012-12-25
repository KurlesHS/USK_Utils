#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "senduskv1workingthread.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    td = new SendUSKv1WorkingThread(this);
    connect(td, SIGNAL(detectedDisconnetcedKpu(QString,int,int)), SLOT(onDetectedDisconnetcedKpu(QString,int,int)));
    connect(td, SIGNAL(detectedNewKpu(QString,int,int)),SLOT(onDetectedNewKpu(QString,int,int)));
    connect(td, SIGNAL(error(QString,int)), SLOT(onError(QString,int)));
    connect(td, SIGNAL(errorOnSendingCommand(QString,QString)), SLOT(onErrorOnSendingCommand(QString,QString)));
    connect(td, SIGNAL(portIsClose(QString,QString)), SLOT(onPortIsClose(QString,QString)));
    connect(td, SIGNAL(portIsOpen(QString,QString)), SLOT(onPortIsOpen(QString,QString)));
    connect(td, SIGNAL(startSendingCommand(QString,QString)), SLOT(onStartSendingCommand(QString,QString)));
    connect(td, SIGNAL(unknowCommand(QString,QString)), SLOT(onUnknowCommand(QString,QString)));
    connect(td, SIGNAL(uskInfoPacketReceived(QString,int)), SLOT(onUskInfoPacketReceived(QString,int)));
    connect(td, SIGNAL(uskIsPresent(QString,bool)), SLOT(onUskIsPresent(QString,bool)));
    connect(td, SIGNAL(uskReset(QString)), SLOT(onUskReset(QString)));
    connect(td, SIGNAL(sensorChanged(QString,int,int,int,int)), SLOT(onSensorChanged(QString,int,int,int,int)));

    connect(ui->pushButtonSendTime, SIGNAL(clicked()), SLOT(onSendTimeButtonPushed()));
    connect(ui->pushButtonMessageOnScreen, SIGNAL(clicked()), SLOT(onSendMessageButtonPushed()));
    connect(ui->pushButtonResetUSK, SIGNAL(clicked()), SLOT(onResetUskButtonPushed()));
    connect(ui->pushButtonChangeRelay, SIGNAL(clicked()), SLOT(onChangeRelaysOnKpuButtonPushed()));

    for (int i = 1; i <= 4; ++i)
        ui->comboBoxRayNum->addItem(trUtf8("Луч №%0").arg(i));
    for (int i = 1; i <= 9; ++i)
        ui->comboBoxKpuNum->addItem(trUtf8("КПУ №%0").arg(i));
    for (int i = 1; i <= 8; ++i)
        ui->comboBoxRelayNum->addItem(trUtf8("Датчик №%0").arg(i));
    ui->comboBoxRelayState->addItem(trUtf8("Лог. \"0\""));
    ui->comboBoxRelayState->addItem(trUtf8("Лог. \"1\""));

#ifdef Q_OS_WIN
    td->addUsk("11", "COM1", 11);
#elif defined Q_OS_LINUX
    td->addUsk("11", "ttyS0", 11);
#endif
    td->openUsk("11");

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onError(const QString &uskName, int errorCode)
{
    QString message;
    SendUSKv1WorkingThread::errorCodes ec = static_cast<SendUSKv1WorkingThread::errorCodes>(errorCode);
    switch(ec)
    {
    case SendUSKv1WorkingThread::errorOpenPort:
        message = trUtf8("Ошибка при открытии порта (%0)").arg(uskName);
        break;
    case SendUSKv1WorkingThread::errorTimeoutWhileWaitData:
        message = trUtf8("Таймаут при ожидании данных пришел слишком маленький пакет (%0)").arg(uskName);
        break;
    case SendUSKv1WorkingThread::errorTimeoutWhileWaitResponse:
        message = trUtf8("Таймаут при ожидании отклика (скорей всего УСК не подключен к порту) (%0)").arg(uskName);
        break;
    case SendUSKv1WorkingThread::errorUskInstPresent:
        message = trUtf8("УСК %0 отсутствует").arg(uskName);
        break;
    case SendUSKv1WorkingThread::errorUskIsntResponse:
        message = trUtf8("Уск %0 не отвечает").arg(uskName);
        break;
    case SendUSKv1WorkingThread::errorUskWrongPacket:
        message = trUtf8("От УСК %0 пришел не верный пакет (не сошлась контрольная сумма)");
        break;
    }
    addMessage(message);
}

void MainWindow::onUnknowCommand(const QString &uskName, const QString &command)
{
    QString message = trUtf8("Пришла неизветная модулю команда (%0) от УСК %1 ").arg(command).arg(uskName);
    addMessage(message);
}

void MainWindow::onErrorOnSendingCommand(const QString &uskName, const QString &commandDescription)
{
    QString message = trUtf8("Ошибка при посылке команды %0 на УСК %1").arg(commandDescription).arg(uskName);
    addMessage(message);
}

void MainWindow::onUskInfoPacketReceived(const QString &uskName, const int &infoPacket)
{
    QString message;
    SendUSKv1WorkingThread::uskInfoPackets uip = static_cast<SendUSKv1WorkingThread::uskInfoPackets>(infoPacket);
    switch(uip)
    {
    case SendUSKv1WorkingThread::packetUskErrorReceivingRS:
        message = trUtf8("Ошибка при посылке команды на УСК %1").arg(uskName);
        break;
    case SendUSKv1WorkingThread::packetUskOn:
        message = trUtf8("Включение УСК %0").arg(uskName);
        break;
    case SendUSKv1WorkingThread::packetUskReset:
        message = trUtf8("Полный сброс УСК %0").arg(uskName);
        break;
    case SendUSKv1WorkingThread::packetUskSettingTime:
        message = trUtf8("Комманда установки времени на УСК %0 выполнена успешно").arg(uskName);
        break;
    }
    addMessage(message);
}

void MainWindow::onPortIsOpen(const QString &uskName, const QString &portName)
{
    addMessage(trUtf8("Порт %0 открыт УСК %1").arg(portName).arg(uskName));
}

void MainWindow::onPortIsClose(const QString &uskName, const QString &portName)
{
    addMessage(trUtf8("Порт %0 закрыт УСК %1").arg(portName).arg(uskName));
}

void MainWindow::onUskReset(const QString &uskName)
{
    addMessage(trUtf8("Сброс УСК %0").arg(uskName));
}

void MainWindow::onUskIsPresent(const QString &uskName, const bool &present)
{
    addMessage(trUtf8("УСК %0 %1").arg(uskName).arg(present ? "присутствует" : "отсутствует"));
}

void MainWindow::onStartSendingCommand(const QString &uskName, const QString &commandDescription)
{
    addMessage(trUtf8("Начало отсылки команды %1 на УСК %0").arg(uskName).arg(commandDescription));
}

void MainWindow::onDetectedNewKpu(const QString &uskName, const int &rayNum, const int &kpuNum)
{
    addMessage(trUtf8("Обнаружено КПУ №%0 на луче №%1 на УСК %2").arg(kpuNum).arg(rayNum).arg(uskName));
}

void MainWindow::onDetectedDisconnetcedKpu(const QString &uskName, const int &rayNum, const int &kpuNum)
{
    addMessage(trUtf8("Обнаружено отключение КПУ №%0 на луче №%1 на УСК %2").arg(kpuNum).arg(rayNum).arg(uskName));
}

void MainWindow::onSensorChanged(const QString &uskName, const int &rayNum, const int &kpuNum, const int &sensorNum, const int &state)
{
    addMessage(trUtf8("Изменение состоняния контактов на УСК %0, луч №%1, КПУ№%2, номер датчика %4 состояние %3")
               .arg(uskName).arg(rayNum).arg(kpuNum).arg(state == 0 ? trUtf8("лог. ноль") : trUtf8("лог. еденица")).arg(sensorNum));
}

void MainWindow::onSendTimeButtonPushed()
{
    td->sendTime("11", QDateTime::currentDateTime());
}



void MainWindow::onSendMessageButtonPushed()
{
}

void MainWindow::onResetUskButtonPushed()
{
    td->resetUsk("11");
}

void MainWindow::onChangeRelaysOnKpuButtonPushed()
{
    td->changeRelayStatus("11", ui->comboBoxRayNum->currentIndex() + 1,
                          ui->comboBoxKpuNum->currentIndex() + 1,
                          ui->comboBoxRelayNum->currentIndex() + 1,
                          ui->comboBoxRelayState->currentIndex());
}

void MainWindow::addMessage(const QString &message)
{
    QString res = trUtf8("%0: %1").arg(QDateTime::currentDateTime().toString()).arg(message);
    ui->textEdit->append(res);
}

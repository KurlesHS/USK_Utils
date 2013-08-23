#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class SendUSKv1WorkingThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onError(const QString &uskName, int errorCode);
    void onUnknowCommand(const QString &uskName, const QString &command);
    void onErrorOnSendingCommand(const QString &uskName, const QString &commandDescription);
    void onUskInfoPacketReceived(const QString &uskName, const int &infoPacket);
    void onPortIsOpen(const QString &uskName, const QString &portName);
    void onPortIsClose(const QString &uskName, const QString &portName);
    void onUskReset(const QString &uskName);
    void onUskIsPresent(const QString &uskName, const bool &present);
    void onStartSendingCommand(const QString &uskName, const QString &commandDescription);
    void onDetectedNewKpu(const QString &uskName, const int &rayNum, const int &kpuNum);
    void onDetectedDisconnetcedKpu(const QString &uskName, const int &rayNum, const int &kpuNum);
    void onSensorChanged(const QString &uskName, const int &rayNum, const int &kpuNum, const int &sensorNum, const int &state);

    void onSendTimeButtonPushed();
    void onSendMessageButtonPushed();
    void onResetUskButtonPushed();
    void onChangeRelaysOnKpuButtonPushed();
    void onLfButtonPushed();



    void addMessage(const QString &message);
    
private:
    Ui::MainWindow *ui;
    SendUSKv1WorkingThread *td;
};

#endif // MAINWINDOW_H

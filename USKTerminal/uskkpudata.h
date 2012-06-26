#ifndef USKKPUDATA_H
#define USKKPUDATA_H

#include "mywidget.h"
#include "qhexedit/qhexedit.h"


namespace Ui {
class USKKPUData;
}

class USKKPUData : public MyWidget
{
    Q_OBJECT
    
public:
    explicit USKKPUData(QWidget *parent = 0);
    ~USKKPUData();
    QByteArray getData();

private:
    quint16 crc16_modbus(const QByteArray &array);

signals:
    void okButtonPushed();

private slots:
    void onSizeOfDataChanged(int newSize);
    void onOkButtonPushed();
    void onModbusButtonPushed();
    void on_pushButtonOk_clicked();


private:
    Ui::USKKPUData *ui;
};

#endif // USKKPUDATA_H

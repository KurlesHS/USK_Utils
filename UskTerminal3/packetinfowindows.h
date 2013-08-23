#ifndef PACKETINFOWINDOWS_H
#define PACKETINFOWINDOWS_H

#include <QDialog>

namespace Ui {
class PacketInfoWindows;
}

class PacketInfoWindows : public QDialog
{
    Q_OBJECT
    
public:
    explicit PacketInfoWindows(QWidget *parent = 0);
    ~PacketInfoWindows();
    void addText(QString text, QString color = "BLACK");
    
private:
    Ui::PacketInfoWindows *ui;
};

#endif // PACKETINFOWINDOWS_H

#include "packetinfowindows.h"
#include "ui_packetinfowindows.h"

#include <QDebug>

PacketInfoWindows::PacketInfoWindows(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PacketInfoWindows)
{
    setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(trUtf8("Разбор пакетов"));
    ui->textEdit->setReadOnly(true);
}

PacketInfoWindows::~PacketInfoWindows()
{
    delete ui;
    qDebug() << "Destroy PacketInfoWindows";
}

void PacketInfoWindows::addText(QString text, QString color)
{
    text.prepend(QString("<FONT color=\"%0\">").arg(color));
    text.append("</FONT>");
    ui->textEdit->append(text);
}

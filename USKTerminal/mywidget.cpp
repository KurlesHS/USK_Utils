#include "mywidget.h"


MyWidget::MyWidget(QWidget *parent) :
    QWidget(parent)
{
}

void MyWidget::setWin1251()
{
    QTextCodec *cyrillicCodec = QTextCodec::codecForName("Windows-1251");
    QTextCodec::setCodecForCStrings(cyrillicCodec);
}

void MyWidget::setUtf8()
{
    QTextCodec *cyrillicCodec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForCStrings(cyrillicCodec);
}

QByteArray MyWidget::prepareComplexNumber(quint64 number)
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

#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>
#include <QByteArray>
#include <QTextCodec>

class MyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyWidget(QWidget *parent = 0);
    virtual QByteArray getData() = 0;
    static QByteArray prepareComplexNumber(quint64 number);
    static void setUtf8();
    static void setWin1251();

signals:

public slots:

};

#endif // MYWIDGET_H

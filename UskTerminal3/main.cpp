#include <QtGui/QApplication>
#include <QTextCodec>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QTextCodec *cyrillicCodec = QTextCodec::codecForName("UTF-8");
    QTextCodec *cyrillicCodec2 = QTextCodec::codecForName("Windows-1251");
    QTextCodec::setCodecForTr(cyrillicCodec);
    QTextCodec::setCodecForLocale(cyrillicCodec);
    QTextCodec::setCodecForCStrings(cyrillicCodec2);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

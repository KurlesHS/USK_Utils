#include "mainwindow.h"
#include <QApplication>
#include <QAudioFormat>
#include <QFile>
#include <QAudioOutput>
#include <QSound>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    return a.exec();
}

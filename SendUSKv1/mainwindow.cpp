#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "senduskv1workingthread.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SendUSKv1WorkingThread *td = new SendUSKv1WorkingThread(this);
    td->addUsk("11", "COM1", 11);
    td->openUsk("11");
}

MainWindow::~MainWindow()
{
    delete ui;
}

#-------------------------------------------------
#
# Project created by QtCreator 2012-12-21T12:26:58
#
#-------------------------------------------------

QT       += core gui

RCC_DIR = $$PWD/../build/senduskv1/rcc
UI_DIR = $$PWD/../build/senduskv1/ui
MOC_DIR = $$PWD/../build/senduskv1/moc
DESTDIR = $$PWD/../app

CONFIG(debug, debug|release):{
DEFINES+=DEBUG
OBJECTS_DIR = $$PWD/../build/senduskv1/debug/obj
} else: {
OBJECTS_DIR = $$PWD/../build/senduskv1/release/obj
}
CONFIG(debug, debug|release):DEFINES += DEBUG


greaterThan(QT_MAJOR_VERSION, 4) {QT += serialport}
else:
{
    system("echo QTSERIALPORT_PROJECT_ROOT = $$PWD/../qtserialport > $$OUT_PWD/.qmake.cache")
    system("echo QTSERIALPORT_BUILD_ROOT = $$PWD/../qtserialport >> $$OUT_PWD/.qmake.cache")
    include($$QTSERIALPORT_PROJECT_ROOT/src/serialport/qt4support/serialport.prf)
}


TARGET = SendUSKv1
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    senduskv1workingthread.cpp

HEADERS  += mainwindow.h \
    senduskv1workingthread.h

FORMS    += mainwindow.ui

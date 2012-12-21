#-------------------------------------------------
#
# Project created by QtCreator 2012-11-14T15:10:21
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

RCC_DIR = $$PWD/../build/pippip/rcc
UI_DIR = $$PWD/../build/pippip/ui
MOC_DIR = $$PWD/../build/pippip//moc
DESTDIR = $$PWD/../app

CONFIG(debug, debug|release):{
DEFINES+=DEBUG
OBJECTS_DIR = $$PWD/../build/pippip/debug/obj
} else: {
OBJECTS_DIR = $$PWD/../build/pippip/release/obj
}
CONFIG(debug, debug|release):DEFINES += DEBUG
TARGET = pipPip
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    data/1.wav \
    data/0.wav \
    data/empty.wav

RESOURCES += \
    pipip.qrc

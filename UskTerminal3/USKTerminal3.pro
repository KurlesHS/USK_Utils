#-------------------------------------------------
#
# Project created by QtCreator 2011-06-29T21:31:50
#
#-------------------------------------------------

QT       += core gui network

RCC_DIR = $$PWD/../build/rcc
UI_DIR = $$PWD/../build/ui
MOC_DIR = $$PWD/../build//moc
DESTDIR = $$PWD/../app

CONFIG(debug, debug|release):{
DEFINES+=DEBUG
OBJECTS_DIR = $$PWD/../build/debug/obj
} else: {
OBJECTS_DIR = $$PWD/../build/release/obj
}
CONFIG(debug, debug|release):DEFINES += DEBUG


TARGET = USKTerminal3
TEMPLATE = app

INCLUDEPATH += $$PWD/../PacketDecoder
INCLUDEPATH += $$PWD

SOURCES += main.cpp\
        mainwindow.cpp \
    mylineedit.cpp \
    serialthread.cpp \
    usktimepacket.cpp \
    uskmessagepacket.cpp \
    selectpacketdialog.cpp \
    mywidget.cpp \
    packetinfowindows.cpp \
    uskkpudata.cpp \
    qhexedit/xbytearray.cpp \
    qhexedit/qhexedit_p.cpp \
    qhexedit/qhexedit.cpp \
    qhexedit/commands.cpp \
    $$PWD/../PacketDecoder/packetdecoder.cpp \
    serialthreadworker.cpp

HEADERS  += mainwindow.h \
    mylineedit.h \
    serialthread.h \
    usktimepacket.h \
    uskmessagepacket.h \
    selectpacketdialog.h \
    mywidget.h \
    packetinfowindows.h \
    uskkpudata.h \
    qhexedit/xbytearray.h \
    qhexedit/qhexedit_p.h \
    qhexedit/qhexedit.h \
    qhexedit/commands.h \
    $$PWD/../PacketDecoder/packetdecoder.h \
    serialthreadworker.h
FORMS    += mainwindow.ui \
    usktimepacket.ui \
    uskmessagepacket.ui \
    selectpacketdialog.ui \
    packetinfowindows.ui \
    uskkpudata.ui

#загрузка и подключение библиотек, необходимых для QSerialDevice
#в случае чего просто скопировать и вставить в .pro файл нового проекта.

include(../qserialdevice/src/qserialdevice/qserialdevice.pri)
include(../qserialdevice/src/qserialdeviceenumerator/qserialdeviceenumerator.pri)
unix:include(../qserialdevice/src/unix/ttylocker.pri)
win32 {
    LIBS += -lsetupapi -luuid -ladvapi32
}
unix:!macx {
    LIBS += -ludev
}

#конец загрузки и подключения.

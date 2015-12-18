#-------------------------------------------------
#
# Project created by QtCreator 2015-12-10T14:55:26
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TypeAssisClient
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    tcpclientsocket.cpp \
    login.cpp

HEADERS  += widget.h \
    getdatathread.h \
    receivethread.h \
    tcpclientsocket.h \
    login.h

FORMS    += widget.ui \
    login.ui

RESOURCES += \
    myapp.qrc

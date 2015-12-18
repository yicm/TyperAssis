#-------------------------------------------------
#
# Project created by QtCreator 2015-12-09T14:20:10
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TypeAssisServer
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    NoFocusDelegate.cpp \
    server.cpp \
    tcpclientsocket.cpp

HEADERS  += widget.h \
    NoFocusDelegate.h \
    server.h \
    tcpclientsocket.h \
    receivethread.h \
    getdatathread.h

FORMS    += widget.ui

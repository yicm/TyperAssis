#ifndef RECEIVETHREAD_H
#define RECEIVETHREAD_H

#include <QThread>
#include "tcpclientsocket.h"

class ReceiveThread : public QThread
{
    Q_OBJECT
public:
    ReceiveThread();
    ReceiveThread(QString,QString,QString);
    ~ReceiveThread();
    void run();
public:
    TcpClientSocket * client;

public slots:
    void updateQueueData(QString,int);
    void closeWidgetSlot();
    void updateClientDataSlot(QString);
};

#endif // RECEIVETHREAD_H

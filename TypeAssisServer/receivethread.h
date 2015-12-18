#ifndef RECEIVETHREAD_H
#define RECEIVETHREAD_H

#include <QThread>
#include <server.h>

class ReceiveThread : public QThread
{
    Q_OBJECT
public:
    ReceiveThread();
    ReceiveThread(int,int);
    ~ReceiveThread();
    void run();
private:
    Server * server;
    int maxConnectNum;

public slots:
    void updateQueueData(QString,int,quint16,QHostAddress);
};

#endif // RECEIVETHREAD_H

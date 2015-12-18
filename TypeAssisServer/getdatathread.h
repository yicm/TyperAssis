#ifndef GETDATATHREAD_H
#define GETDATATHREAD_H

#include "receivethread.h"

class GetDataThread : public QThread
{
    Q_OBJECT
public:
    GetDataThread();
    GetDataThread(int,int);
    void run();
signals:
    void dataGeted(QVariantMap);
private:
    int port;
    int maxConnectNum;
};

#endif // GETDATATHREAD_H

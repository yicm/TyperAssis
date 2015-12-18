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
    //数据已获取到了，向UI层发送获取到数据
    void dataGeted(QVariantMap);
private:
};

#endif // GETDATATHREAD_H

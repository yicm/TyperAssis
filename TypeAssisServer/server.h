#ifndef SERVER_H
#define SERVER_H

#include <QWidget>
#include <QtNetwork>
#include "tcpclientsocket.h"

class Server : public QTcpServer
{
    //提供对象的信号槽机制，qmake
    Q_OBJECT

public:
    Server(QObject *parent=0,int port=0);
    //用来保存每一个客户连接的TcpClientSocket
    QList<TcpClientSocket*>tcpClientSocketList;
public:
    //最后离线，向其他客户端发送LEAVE
    void updateClientsData(QString&,QHostAddress,int);

signals:
    void updateServer(QString,int,quint16,QHostAddress);
public slots:
    void updateClients(QString,int,quint16,QHostAddress);
    void slotDisconnected(int);

protected:
    void incomingConnection(int socketDescriptor);
};

#endif // SERVER_H

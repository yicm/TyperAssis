#ifndef TCPCLIENTSOCKET_H
#define TCPCLIENTSOCKET_H

#include <QWidget>
#include <QtNetwork>

#define SWITCH 0

class TcpClientSocket:public QTcpSocket
{
    Q_OBJECT

public:
    TcpClientSocket(QObject *parent=0);
    ~TcpClientSocket();
signals:
    void updateClients(QString,int,quint16,QHostAddress);
    void disconnected(int);
protected slots:
    void dataReceived();
    void slotDisconnected();
};
#endif

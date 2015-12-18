#ifndef TCPCLIENTSOCKET_H
#define TCPCLIENTSOCKET_H

#include <QWidget>
#include <QtNetwork>

#define  SWITCH  0


class TcpClientSocket:public QTcpSocket
{
    Q_OBJECT

public:
    TcpClientSocket();
    TcpClientSocket(QObject *parent=0);
    TcpClientSocket(QString,QString,QString);
    ~TcpClientSocket();
    void disconnected();
private:
   QString nickName;
   QHostAddress *serverIP;
signals:
    void updateClients(QString,int);
protected slots:
    void slotConnected();
    void dataReceived();
};
#endif

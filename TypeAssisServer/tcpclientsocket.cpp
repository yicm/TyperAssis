#include "tcpclientsocket.h"


TcpClientSocket::TcpClientSocket(QObject *parent)
{
    connect(this,SIGNAL(readyRead()),this,SLOT(dataReceived()));
    connect(this,SIGNAL(disconnected()),this,SLOT(slotDisconnected()));
}

TcpClientSocket::~TcpClientSocket()
{

}

//有数据到来
void TcpClientSocket::dataReceived()
{
    while(bytesAvailable()>0){
        char buf[1024];
        int length = bytesAvailable();
        this->read(buf,length);
        QString msg = buf;

        //向服务层发送接收到的数据
        emit updateClients(msg,length,this->peerPort(),this->peerAddress());
    }
}

void TcpClientSocket::slotDisconnected()
{
    if(SWITCH)qDebug()<< "tcpclientsocket slotDiaconnected";
    //有用户已断开了连接，向服务层发送断开连接信号
    emit disconnected(this->socketDescriptor());
}

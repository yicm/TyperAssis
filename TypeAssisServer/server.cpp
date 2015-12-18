#include"server.h"


Server::Server(QObject *parent,int port):QTcpServer(parent)
{
    //在指定端口对任意的地址进行监听
    listen(QHostAddress::Any,port);
}

//有TCP请求会促发该函数,然后新建一个client socket,并处理该socket
void Server::incomingConnection(int socketDescriptor)
{
    if(SWITCH)qDebug() << "client incomingConnection";
    TcpClientSocket *tcpClientSocket = new TcpClientSocket(this);

    //处理该新的client socket,包括该客户端的数据更新和断开连接
    connect(tcpClientSocket,SIGNAL(updateClients(QString,int,quint16,QHostAddress)),
            this,SLOT(updateClients(QString,int,quint16,QHostAddress)));
    connect(tcpClientSocket,SIGNAL(disconnected(int)),this,SLOT(slotDisconnected(int)));
    if (!tcpClientSocket->setSocketDescriptor(socketDescriptor)){
        qDebug()<< "setSocketDescriptor error";
        //错误处理
        //do someting ...
        return;
    }

    //将tcpClientSocket加入保存列表
    tcpClientSocketList.append(tcpClientSocket);
}

//向未断开连接的所有客户端发送该断开连接的客户
void Server::updateClientsData(QString &msg,QHostAddress address,int clientNum)
{
    tcpClientSocketList.removeAt(clientNum);

    QString handleMsg = msg.left(msg.length()-1) + ",\"rip\":\""+address.toString()+"\"}";
    if(SWITCH)qDebug()<< "[LEAVEMSG]: "<< handleMsg;
    //实现信息"广播"
    for(int i=0;i<tcpClientSocketList.count();i++){
        QTcpSocket *item = tcpClientSocketList.at(i);
        if(item->write(handleMsg.toUtf8(),handleMsg.length()) != handleMsg.length()){
            continue;
        }
    }
}

//将任意客户端发来的信息进行转播
void Server::updateClients(QString msg,int length,quint16 port,QHostAddress address)
{
    //向主窗口发送(信号)数据，传递至UI层
    emit updateServer(msg,length,port,address);
    QString handleMsg = msg.left(length-1) + ",\"rip\":\""+address.toString()+"\"}";
    if(SWITCH)qDebug()<< "handleMsg: "<< handleMsg;
    //实现信息"广播"
    for(int i=0;i<tcpClientSocketList.count();i++){
        QTcpSocket *item = tcpClientSocketList.at(i);
        if(item->write(handleMsg.toUtf8(),handleMsg.length()) != handleMsg.length()){
            continue;
        }
    }
}

//从tcpClientSocketList列表中将断开连接的TcpClientSocket对象删除掉
void Server::slotDisconnected(int descriptor)
{
    if(SWITCH)qDebug()<< "slotDisConnected";

    //向UI发送json数据
    QJsonObject json;
    json.insert("Header","LEAVE");          //DATA:打字数据，MSG:弹幕信息；CHAT:私聊；LIVE:离开
    json.insert("name", "");                //
    json.insert("speed", 0);                //打字速度
    json.insert("accuracy","");             //正确率
    json.insert("time",0);                  //在线时间(s)
    json.insert("to","");                   //与谁私聊，另开端口
    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);

    QString msg = "{\"Header\":\"LEAVE\",\"accuracy\":\"\",\"name\":\"\",\"speed\":\"\",\"time\":\"0\",\"to\":\"\"}";

    for(int i=0;i<tcpClientSocketList.count();i++){
        QTcpSocket *item=tcpClientSocketList.at(i);
        if(item->socketDescriptor()==descriptor){                        
            //发送完断开连接数据后，删除该连接
            updateClientsData(json_str,item->peerAddress(),i);

            emit updateServer(json_str,json_str.length(),
                              item->peerPort(),item->peerAddress());
            return;
        }
    }

    return;
}

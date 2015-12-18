#include "widget.h"
#include "ui_widget.h"

#include <QMouseEvent>
#include <QMessageBox>
#include <QQueue>

#include "NoFocusDelegate.h"


QVector<QVariantMap> queueData;
QSemaphore toReceive;
QSemaphore toGetData;

QWaitCondition buffer_full;
QWaitCondition buffer_empty;
QMutex     mutex;
QMutex     mutex2;


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    //UI初始化
    initUi();
    //服务器创建按钮与槽函数绑定
    connect(ui->createServerButton,SIGNAL(clicked()),this,SLOT(slotCreateServer()));
}

Widget::~Widget()
{
    delete ui;
}

//-----------------------------------------------------------
//服务器主界面初始化部分
//-----------------------------------------------------------
void Widget::initUi()
{
    //获取最小化、关闭按钮图标
    QPixmap minPix  = style()->standardPixmap(QStyle::SP_TitleBarMinButton);
    QPixmap closePix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
    QPixmap maxPix = style()->standardPixmap(QStyle::SP_TitleBarMaxButton);

    //设置最小化、关闭按钮图标
    ui->minButton->setIcon(minPix);
    ui->maxButton->setIcon(maxPix);
    ui->closeButton->setIcon(closePix);

    ui->minButton->setToolTip(tr("最小化"));
    ui->maxButton->setToolTip(tr("最大化"));
    ui->closeButton->setToolTip(tr("关闭"));
    //窗口鼠标拖动相关
    //设置主窗口无边框
    this->setWindowFlags(Qt::FramelessWindowHint);
    //设置在不按鼠标的情况下也触发鼠标移动事件
    this->setMouseTracking(true);
    //鼠标左键按下标记
    isLeftPressed = false;
    //标记鼠标左击时的位置
    curPos=0;
    //tablewidget初始化
    ui->tableWidget->setColumnCount(TABWIDGET_COLUMN);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setGridStyle(Qt::DotLine);
    ui->tableWidget->horizontalHeader()->setDisabled(true);
    ui->tableWidget->verticalHeader()->setDisabled(true);
    //设置tablewidget等宽
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //设置为鼠标选中为选中一行
    //ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->horizontalHeader()->setFixedHeight(35);
    //设置表头
    QStringList header;
    header << tr("用户名") << tr("IP") << tr("端口") << tr("打字速度(字符/s)") << tr("正确率(%)") << tr("在线时间(s)");
    ui->tableWidget->setHorizontalHeaderLabels(header);
    //设置表头字体
    QFont font = ui->tableWidget->horizontalHeader()->font();
    font.setBold(true);
    ui->tableWidget->horizontalHeader()->setFont(font);
    ui->tableWidget->setStyleSheet("selection-background-color:lightblue;");
    ui->tableWidget->horizontalHeader()->setHighlightSections(false);
    ui->tableWidget->setFrameShape(QFrame::NoFrame);
    //设置垂直表头是否显示列数
    //ui->tableWidget->verticalHeader()->hide();
    //去除单元格之间选中时的虚框
    ui->tableWidget->setItemDelegate(new NoFocusDelegate());
    ui->tableWidget->setMouseTracking(true);
    //tableWidget的一些信号槽连接
    /*
    connect(ui->tableWidget,SIGNAL(itemEntered(QTableWidgetItem*)),
            this,SLOT(itemEnteredHover(QTableWidgetItem *)));

    connect(ui->tableWidget,SIGNAL(itemClicked(QTableWidgetItem*)),
            this,SLOT(itemIsClicked(QTableWidgetItem *)));

    connect(ui->tableWidget, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(sdlog2Decode(int,int)));
    connect(ui->tableWidget->horizontalHeader(),SIGNAL(sectionClicked(int)),
            this, SLOT(onHeaderClicked(int)));
    */
}

//-----------------------------------------------------------
//窗口移动、缩放处理部分
//-----------------------------------------------------------
//鼠标按下事件
void Widget::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        this->isLeftPressed=true;
        QPoint temp=event->globalPos();
        pLast=temp;
        curPos=countFlag(event->pos(),countRow(event->pos()));
        event->ignore();
    }
}

//鼠标释放事件
void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    if(isLeftPressed)
        isLeftPressed=false;
    //恢复鼠标指针形状
    QApplication::restoreOverrideCursor();
    event->ignore();
}

//鼠标双击事件
void Widget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        if(windowState()!=Qt::WindowFullScreen)
            //双击窗口全屏
            setWindowState(Qt::WindowFullScreen);
        //恢复正常模式
        else setWindowState(Qt::WindowNoState);
    }
    event->ignore();
}

//鼠标移动时间处理
void Widget::mouseMoveEvent(QMouseEvent *event)
{
    int poss = countFlag(event->pos(),countRow(event->pos()));

    setCursorType(poss);
    if(isLeftPressed)
    {
        QPoint ptemp=event->globalPos();
        ptemp=ptemp-pLast;
        //移动窗口
        if(curPos == 22)
        {
            ptemp=ptemp+pos();
            move(ptemp);
        }
        else
        {
            QRect wid=geometry();

            switch(curPos)
            {
                case 11:wid.setTopLeft(wid.topLeft()+ptemp);break;          //左上角
                case 13:wid.setTopRight(wid.topRight()+ptemp);break;        //右上角
                case 31:wid.setBottomLeft(wid.bottomLeft()+ptemp);break;    //左下角
                case 33:wid.setBottomRight(wid.bottomRight()+ptemp);break;  //右下角
                case 12:wid.setTop(wid.top()+ptemp.y());break;              //中上角
                case 21:wid.setLeft(wid.left()+ptemp.x());break;            //中左角
                case 23:wid.setRight(wid.right()+ptemp.x());break;          //中右角
                case 32:wid.setBottom(wid.bottom()+ptemp.y());break;        //中下角
            }
            setGeometry(wid);
        }
        //更新位置
        pLast=event->globalPos();
    }
    event->ignore();
}

//计算鼠标在哪一列和哪一行
int Widget::countFlag(QPoint p,int row)
{
    if(p.y()< MARGIN)
        return 10+row;
    else if(p.y()>this->height() - MARGIN)
        return 30+row;
    else
        return 20+row;
}

//根据鼠标所在位置改变鼠标指针形状
void Widget::setCursorType(int flag)
{
    Qt::CursorShape cursor = Qt::ArrowCursor;
    switch(flag)
    {
        case 11:
        case 33:
        cursor = Qt::SizeFDiagCursor;break;
        case 13:
        case 31:
        cursor = Qt::SizeBDiagCursor;break;
        case 21:
        case 23:
        cursor = Qt::SizeHorCursor;break;
        case 12:
        case 32:
        cursor = Qt::SizeVerCursor;break;
        case 22:
        cursor = Qt::ArrowCursor;break;
        default:
        //恢复鼠标指针形状
        //QApplication::restoreOverrideCursor();
        break;
    }
    setCursor(cursor);
}

//计算在哪一列
int Widget::countRow(QPoint p)
{
    return (p.x()<MARGIN)?1:(p.x()>(this->width()-MARGIN)?3:2);
}

//-----------------------------------------------------------
//最大小、关闭按钮槽函数
//-----------------------------------------------------------
void Widget::on_minButton_clicked()
{
    this->setWindowState(Qt::WindowMinimized);
}

void Widget::on_maxButton_clicked()
{
    if(windowState() == Qt::WindowMaximized){
        this->setWindowState(Qt::WindowNoState);
    }
    else this->setWindowState(Qt::WindowMaximized);
}

void Widget::on_closeButton_clicked()
{
    this->close();
}
//-----------------------------------------------------------
//表格列表用户处理：增 删 改
//-----------------------------------------------------------
//新增一个用户到tableWidget
void Widget::addANewUser(QString *data)
{    
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    for(int i = 0; i < (ui->tableWidget->columnCount()); ++i){
        QTableWidgetItem *item = new QTableWidgetItem(data[i]);
        item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        item->setFlags(item->flags());
        ui->tableWidget->setItem(row,i,item);
    }
}

//更新tableWidget中某个用户的数据
void Widget::updateUserData(QString *data)
{
    int row = ui->tableWidget->rowCount();
    if(row > 0){
      int i = 0;
      for(i = 0;i < row; ++i){
            QString strText = ui->tableWidget->item(i,1)->text();
            if(*(data+1) == strText){
                for(int j = 0;j < (ui->tableWidget->columnCount());++j){
                    QTableWidgetItem *item = new QTableWidgetItem(data[j]);
                    item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
                    item->setFlags(item->flags());
                    ui->tableWidget->setItem(i,j,item);
                }
            }
        }
    }
    else{
        ui->tableWidget->insertRow(row);
        for(int i = 0; i < (ui->tableWidget->columnCount()); ++i){
            QTableWidgetItem *item = new QTableWidgetItem(data[i]);
            item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            item->setFlags(item->flags());
            ui->tableWidget->setItem(row,i,item);
        }
    }

    ui->tableWidget->sortItems( 3, Qt::DescendingOrder);
}

//删除tableWidget中的某个用户
void Widget::deleteUser(QString *data)
{
    for(int i = 0;i < ui->tableWidget->rowCount(); ++i){
        if(SWITCH)qDebug()<< "i=" << i;
        //【这一个问题没有解决：tableWidget的用户列表数目跟实际的在线用户数目跟不上同步】
        if(ui->tableWidget->item(i,1) == NULL){
            //没有处理该信息，重新将该信息加到缓冲区中
            if(SWITCH)qDebug()<< "The ui tableWidget item is NULL";
            return ;
        }
        QString strText = ui->tableWidget->item(i,1)->text();
        if(data[1] == strText){
            ui->tableWidget->removeRow(i);
            ui->tableWidget->update();
        }
    }
}

//-----------------------------------------------------------
//创建TCP服务器、更新服务器端的数据
//-----------------------------------------------------------
//创建TCP服务器按钮槽函数
void Widget::slotCreateServer()
{
    QString portStr = ui->lineEditPort->text();
    QString maxConnectNumStr = ui->lineEditMaxConnect->text();

    //输入的文本限制处理
    if(portStr.size() < 3 || maxConnectNumStr.size() <= 0){
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setStyleSheet("background:#BDB76B;color:#000080;font-size:20px;font-weight:bold;");
        msgBox.setIcon(QMessageBox::Critical);       
        msgBox.setText("端口号错误或连接数目错误！请检查！");
        msgBox.exec();
        return ;
    }
    port = portStr.toInt();
    maxConnectNum = maxConnectNumStr.toInt();
    if(SWITCH)if(SWITCH)qDebug() << port << " " << maxConnectNum;
    //通过线程方式，创建接收数据线程
    receiveThread = new ReceiveThread(port,maxConnectNum);
    toReceive.release(maxConnectNum);

    //开启接收数据线程，将接收到的数据放入到队列缓冲区中
    receiveThread->start();
    //创建获取数据线程，从队列缓冲区中每次取一个数据处理啊
    getDataThread = new GetDataThread(port,maxConnectNum);
    connect(getDataThread,SIGNAL(dataGeted(QVariantMap)),
            this,SLOT(getDataSlot(QVariantMap)));
    //开启获取数据线程
    getDataThread->start();
    ui->createServerButton->setEnabled(false);
}

//获取数据槽函数，通过获取数据线程中信号，来接收该获取的数据
void Widget::getDataSlot(QVariantMap jsonMap)
{
    //数据类型
    QString header = jsonMap["Header"].toString();
    QString jsonArray[10] = {};
    //客户昵称
    jsonArray[0] = jsonMap["name"].toString();
    //客户端ip地址
    jsonArray[1] = jsonMap["ip"].toString();
    //端口
    jsonArray[2] = jsonMap["port"].toString();
    //speed：打字速度
    jsonArray[3] = jsonMap["speed"].toString();
    //accuracy：正确率
    jsonArray[4] = jsonMap["accuracy"].toString();
    //time:在线时间
    jsonArray[5] = jsonMap["time"].toString();

    //各种类信息处理
    //打字用户登录
    if(header == "LOGIN"){
        addANewUser(jsonArray);
    }
    //打字用户数据包
    else if(header == "DATA"){

        updateUserData(jsonArray);
    }
    //打字用户离线
    else if(header == "LEAVE"){
        deleteUser(jsonArray);
    }
    //打字用户发送了弹幕
    else if(header == "MSG"){

    }
    //打字用户与人私聊
    else if(header == "CHAT"){

    }
}

//-----------------------------------------------------------
//ReceiveThread
//-----------------------------------------------------------
ReceiveThread::ReceiveThread()
{

}

ReceiveThread::ReceiveThread(int port,int maxConnectNum)
{
    this->maxConnectNum = maxConnectNum;
    //接收数据线程开启网络连接
    server = new Server(this,port);

    queueData.clear();

    server->setMaxPendingConnections(maxConnectNum);
    //绑定数据连接和更新缓冲区槽函数
    connect(server,SIGNAL(updateServer(QString,int,quint16,QHostAddress)),
            this,SLOT(updateQueueData(QString,int,quint16,QHostAddress)));
}

ReceiveThread::~ReceiveThread()
{
    delete server;
}

//等待数据更新
void ReceiveThread::run()
{
    this->exec();
}

//数据更新，放入缓冲区中
void ReceiveThread::updateQueueData(QString msg,int len,quint16 port,QHostAddress address)
{
    if(SWITCH)if(SWITCH)qDebug() << "------.left(length)-----------";
    if(SWITCH)if(SWITCH)qDebug() << msg.left(len);
    if(SWITCH)if(SWITCH)qDebug() << address.toString();

    //超过了设置的最大连接数
    if(toGetData.available() == maxConnectNum){
        mutex.lock();
        buffer_full.wait(&mutex);
        mutex.unlock();
    }

    toReceive.acquire();
    mutex2.lock();
    //json解析
    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(msg.left(len).toUtf8(),&jsonError);

    if(jsonError.error == QJsonParseError::NoError){
        if(jsonResponse.isObject()){            
            QJsonObject obj = jsonResponse.object();
            QVariantMap jsonMap = obj.toVariantMap();
            jsonMap.insert("port",port);
            jsonMap.insert("ip",address.toString());
            //放入缓冲区中
            queueData.push_back(jsonMap);
        }
    }
    if(SWITCH)qDebug()<< "[R]queueData.size=" << queueData.size();

    mutex2.unlock();
    toGetData.release();
    buffer_empty.wakeAll();
}
//-----------------------------------------------------------
//GetDataThread
//-----------------------------------------------------------
GetDataThread::GetDataThread()
{

}

GetDataThread::GetDataThread(int port,int maxConnectNum)
{
    this->port = port;
    this->maxConnectNum = maxConnectNum;
}

//获取数据线程
void GetDataThread::run()
{
    if(SWITCH)qDebug()<< "GetDataThread is running...";
    while(1){
        if(toReceive.available() == maxConnectNum){
            mutex.lock();
            buffer_empty.wait(&mutex);
            mutex.unlock();
        }
        toGetData.acquire();
        mutex2.lock();
        if(SWITCH)qDebug()<< "g mutex2.lock()";

        if(queueData.size() > 0){
            QVariantMap msg = queueData.last();
            QVector<QVariantMap>::iterator iter = queueData.end();
            --iter;
            queueData.erase(iter);
            //将获取的数据通过信号槽机制传送到UI
            emit dataGeted(msg);
        }
        mutex2.unlock();
        toReceive.release();
        buffer_full.wakeAll();

        if(SWITCH)qDebug()<< "[G]queueData.size=" << queueData.size();
    }
}

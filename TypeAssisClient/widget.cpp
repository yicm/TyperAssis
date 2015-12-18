#include "widget.h"
#include "ui_widget.h"

#include <QMouseEvent>
#include <QPoint>
#include <qDebug>
#include <QDesktopWidget>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>


QSemaphore toReceive;
QSemaphore toGetData;
QQueue<QVariantMap> queueData;
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
    //定时器初始化
    TimerInit();
    //导入打字文章
    ShowFileTextInit(":/files/TEST.txt");
    //未登录之前隐藏主界面
    this->hide();

    //新建一个登录
    login = new Login();
    login->show();
    connect(login,SIGNAL(loginSendSetData(QString,QString,QString)),this,
            SLOT(widgetReceiveSetData(QString,QString,QString)));
    connect(login,SIGNAL(loginSendEventData(int)),this,
            SLOT(widgetReceiveEventData(int)));
}

Widget::~Widget()
{
    if(eventData != -1){
        delete receiveThread;
        delete getDataThread;
    }
    delete login;
    delete ui;
}

void Widget::initUi()
{
    //关闭主界面事件数据 ECS  Close 初始化
    eventData = 0;
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
    //this->setWindowIcon(QIcon(":/images/icon/proicon.png"));
    int x = (QApplication::desktop()->width() - this->width())/2;
    int y = (QApplication::desktop()->height() - 30 - this->height())/2;
    this->move(x,y);
    //设置在不按鼠标的情况下也触发鼠标移动事件
    this->setMouseTracking(true);
    //鼠标左键按下标记
    isLeftPressed = false;
    //标记鼠标左击时的位置
    curPos=0;
    //打字部分初始化
    ct = 0;                                         //初始化ct计数
    CurrentFocus = 1;                               //默认初始聚焦在view1上
    BackSpaceFg = 0;
    BackSpaceStatus = 0;
    CorrectNum = 0;                                 //正确个数
    ErrorNum = 0;                                   //错误个数
    AllNum = 0;                                     //总个数 = 正确个数+错误个数
    TimeCount = 0;
    isSetChar = false;

    ui->view1->installEventFilter(this);            //在窗体上为view1安装过滤器
    ui->view3->installEventFilter(this);            //在窗体上为view3安装过滤器
    ui->view5->installEventFilter(this);            //在窗体上为view5安装过滤器
    ui->view7->installEventFilter(this);            //在窗体上为view7安装过滤器
    ui->view9->installEventFilter(this);            //在窗体上为view9安装过滤器
    //默认不能使能，处理光标问题
    ui->view3->setEnabled(false);
    ui->view5->setEnabled(false);
    ui->view7->setEnabled(false);
    ui->view9->setEnabled(false);
}

//-----------------------------------------------------------
//登录槽函数处理
//-----------------------------------------------------------
void Widget::widgetReceiveSetData(const QString &ip,const QString &port,const QString &nickName)
{
    login->hide();
    this->show();
    this->nickName = nickName;
    //创建接收数据线程
    receiveThread = new ReceiveThread(ip,port,nickName);
    //当打字数据更新时，发送数据更新新号和执行相关槽函数
    connect(this,SIGNAL(updateClientDataSignal(QString)),receiveThread,SLOT(updateClientDataSlot(QString)));
    //当窗口关闭时，发送离线信号给服务端
    connect(this,SIGNAL(closeSignal()),receiveThread,SLOT(closeWidgetSlot()));

    toReceive.release(BUFFER_SIZE);

    receiveThread->start();
    //创建获取数据线程
    getDataThread = new GetDataThread();
    connect(getDataThread,SIGNAL(dataGeted(QVariantMap)),
            this,SLOT(getDataSlot(QVariantMap)));
    getDataThread->start();
}

//EventData接收槽函数
void Widget::widgetReceiveEventData(const int &eventData)
{
    if(SWITCH)qDebug()<< eventData;
    this->eventData = eventData;
    if(eventData == -1){
        if(SWITCH)qDebug() << "eventData = -1";
        this->close();
        return ;
    }

}

//-----------------------------------------------------------
//打字练习部分设置
//-----------------------------------------------------------
void Widget::TimerInit()
{
    ui->lcdNumber->setDigitCount(8);
    ui->lcdNumber->setSegmentStyle(QLCDNumber::Flat);

    QTimer *timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timerHandleSlot()));
    //三秒钟刷新一次
    timer->start(3000);
}

//设置主界面背景
void Widget::SetProBackground(const QString path)
{
    //this->setStyleSheet("background-image:url(:/images/background/login.jpg);"); //样式表设置背景图片
    QPalette pal;
    QPixmap pixmap(path);
    pal.setBrush(QPalette::Window,QBrush(pixmap));
    setPalette(pal);
}

//定时器溢出槽函数
void Widget::timerHandleSlot()
{
    TimeCount += 3;
    QDateTime time=QDateTime::currentDateTime();
    QString text=time.toString("hh:mm:ss");
    ui->lcdNumber->display(text);                       //时间
    QString str = tr("%1").arg(CorrectNum);             //正确个数
    ui->lineEdit0->setText(str);
    str = tr("%1").arg(ErrorNum);                       //错误个数
    ui->lineEdit1->setText(str);

    QString accuracyStr;
    if(AllNum != 0){                                    //正确率
        accuracyStr = str.setNum(((float)CorrectNum/(float)AllNum)*100,'g',4);
    }
    else {
        accuracyStr = "0.00";
    }
    ui->lineEdit2->setText(str);
    QString speedStr = tr("%1").arg(CorrectNum*60/(TimeCount));
    ui->lineEdit3->setText(speedStr);

    //发信更新数据信号到接收数据线程，通过线程的client socket将数据传送到服务器上
    QJsonObject json;
    json.insert("Header","DATA");                       //DATA:打字数据，MSG:弹幕信息；CHAT:私聊；LOGIN:登录
    json.insert("name", nickName);                      //My name
    json.insert("to","");                               //与谁私聊，另开端口
    json.insert("speed", speedStr);                     //打字速度
    json.insert("accuracy",accuracyStr);                //正确率
    json.insert("time",QString::number(TimeCount));     //在线时间(s)
    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);
    emit updateClientDataSignal(json_str);
}

//-----------------------------------------------------------
//文章操作相关
//-----------------------------------------------------------
//文本截取，比对剩余文本
int Widget::ShowFileTextInit(const QString textpath)
{
    QFile file(textpath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if(SWITCH)qDebug()<<"Can't open the file!"<<endl;
        return -1;
    }

    Content = QString(file.readAll());                  //获取文件文本字符串
    Content = Content.simplified();                     //去掉\r\n，并用空格代替
    Length = Content.length();                          //获取文本字符串总长度
    RestContent = Content;                              //初始时RestContent = Content;
    RestLength = Length;                                //初始剩余长度，不然是随机值。
    RestContent = ViewGetTextFromContent(RestContent,Length);

    return 1;
}

//lineEdit从String中获取文本并填充文本框
//cont:文本
//len:文本当前长度
//返回截取后的文本RestContent
QString Widget::ViewGetTextFromContent(QString cont,unsigned long len)
{
    //文本长度小于了文本框所需要的字符个数，则需要从头开始，初始化文本内容
    if(RestLength <= LINE_NUMBER*5){        
        RestLength = Length;                            //重新赋值原文本长度
        if(SWITCH)qDebug()<<RestLength;
        if(SWITCH)qDebug()<<"The file's content is too short!![Restart again.]";
        return NULL;                                    //已经没有文字可显示了,then...
    }
    RestLength = len - LINE_NUMBER*5;                   //每一次执行，减少LINE_NUMBER*5长度
    //view0
    QString tempstr = cont.left(LINE_NUMBER);           //取前LINE_NUMBER个字符
    cont =  cont.mid(LINE_NUMBER);                      //从第LINE_NUMBER个字符开始取data字符串，一直到到结尾
    ui->view0->setText(tempstr);                        //写入文本
    //view2
    tempstr = cont.left(LINE_NUMBER);
    cont = cont.mid(LINE_NUMBER);
    ui->view2->setText(tempstr);
    //view4
    tempstr = cont.left(LINE_NUMBER);
    cont = cont.mid(LINE_NUMBER);
    ui->view4->setText(tempstr);
    //view6
    tempstr = cont.left(LINE_NUMBER);
    cont = cont.mid(LINE_NUMBER);
    ui->view6->setText(tempstr);
    //view8
    tempstr = cont.left(LINE_NUMBER);
    cont = cont.mid(LINE_NUMBER);

    //聚焦到第一个文本输入框中
    ui->view8->setText(tempstr);
    ui->view1->setFocus();

    return cont;
}

//切换聚焦
void Widget::SwitchFocus(int focus)
{
    switch(focus){
        case 1:ui->view1->setFocus();break;
        case 3:ui->view3->setFocus();break;
        case 5:ui->view5->setFocus();break;
        case 7:ui->view7->setFocus();break;
        case 9:ui->view9->setFocus();break;
        default:;
    }
}

//设置字符背景颜色
void Widget::setCharBackground(int focus,unsigned int pos,QString color)
{
    if(pos <= 0)return ;
    isSetChar = true;
    if(SWITCH)qDebug()<<"setCharBackground starting...";

    QTextEdit * tempview;
    tempview = GetView(focus);

    QTextCursor cursor = tempview->textCursor();
    //行首
    cursor.movePosition( QTextCursor::StartOfLine );
    //向右移动到Pos-1,即待修改的字符左边
    cursor.movePosition( QTextCursor::Right, QTextCursor::MoveAnchor, pos-1);
    //选中这个字符
    cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::KeepAnchor );
    // added,使选中生效,必须有这句
    tempview->setTextCursor(cursor);

    //修改前字符格式
    QTextCharFormat defcharfmt = tempview->currentCharFormat();
    //新增字符格式
    QTextCharFormat newcharfmt = defcharfmt;
    //颜色
    newcharfmt.setBackground(QColor(color));
    tempview->setCurrentCharFormat(newcharfmt);

    //加上这句是为了去除光标selected,这时光标在字符左边
    cursor.movePosition(QTextCursor::PreviousCharacter);
    //这时光标在行尾
    cursor.movePosition(QTextCursor::EndOfLine);
    //added,使选中生效,必须有这句
    tempview->setTextCursor(cursor);

    if(SWITCH)qDebug()<<"setCharBackground ending...";
}

//设置字符前景色，即字符颜色
void Widget::setCharForeground(int focus,unsigned int pos,QString color)
{
    if(pos <= 0)return ;
    isSetChar = true;
    if(SWITCH)qDebug()<<"setCharForeground starting...";

    QTextEdit * tempview;
    tempview = GetView(focus);

    QTextCursor cursor = tempview->textCursor();
    //行首
    cursor.movePosition( QTextCursor::StartOfLine );
    //向右移动到Pos-1,即待修改的字符左边
    cursor.movePosition( QTextCursor::Right, QTextCursor::MoveAnchor, pos-1);
    //选中这个字符
    cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::KeepAnchor );
    //added,使选中生效,必须有这句
    tempview->setTextCursor(cursor);

    //修改前字符格式
    QTextCharFormat defcharfmt = tempview->currentCharFormat();
    //新增字符格式
    QTextCharFormat newcharfmt = defcharfmt;
    //字符颜色
    newcharfmt.setForeground(QColor(color));

    tempview->setCurrentCharFormat(newcharfmt);

    //加上这句是为了去除光标selected，这时光标在字符左边
    cursor.movePosition(QTextCursor::PreviousCharacter);
   //这时光标在行尾
    cursor.movePosition(QTextCursor::EndOfLine);
    //added,使选中生效,必须有这句
    tempview->setTextCursor(cursor);
    if(SWITCH)qDebug()<<"ending...";
}

//设置字符格式
void Widget::SetTexteditCharFormat(int focus,bool fg,bool isbackspace)
{
    unsigned int pos;
    if((focus%2) == 0)return ;
    QTextEdit * tempview;
    tempview = GetView(focus);

    pos = (tempview->toPlainText()).length();

    if(isbackspace){
        //黑色
        setCharForeground(focus-1,pos,"#030303");
        if(SWITCH)qDebug()<<"renew the mirror char";
    }
    else{
        if(fg == true){
            //正确前景色白
            setCharBackground(focus,pos,"#FFFFFF");
            //正确背景色灰
            setCharForeground(focus,pos,"#ABABAB");
        }
        else{
            //错误，前景色黑
            setCharForeground(focus,pos,"#0D0D0D");
            //错误，背景色黄色
            setCharBackground(focus,pos,"#EEEE00");
        }
    }    
}

//文本框格式清除
void Widget::TextEditClear()
{
    ui->view0->clear();
    setCharForeground(0,1,"#030303");
    setCharBackground(0,1,"#FFFFFF");
    ui->view1->clear();
    ui->view2->clear();
    setCharForeground(2,1,"#030303");
    setCharBackground(2,1,"#FFFFFF");
    ui->view3->clear();
    ui->view4->clear();
    setCharForeground(4,1,"#030303");
    setCharBackground(4,1,"#FFFFFF");
    ui->view5->clear();
    ui->view6->clear();
    setCharForeground(6,1,"#030303");
    setCharBackground(6,1,"#FFFFFF");
    ui->view7->clear();
    ui->view8->clear();
    setCharForeground(8,1,"#030303");
    setCharBackground(8,1,"#FFFFFF");
    ui->view9->clear();
}

//获取光标处镜像字符和光标处字符
QString Widget::GetTextEditMirrorOrInputChar(int select,int pos)
{
    if(select == 0){
        QTextEdit * tempview = GetView(CurrentFocus-1);
        QString str = tempview->toPlainText();
        if(str.isEmpty()) return NULL;
        else{
            str = str.left(pos);
            if(pos != 0){
                str = str.right(1);
                return str;
            }
            else return NULL;
        }
    }
    else if(select == 1){
        QTextEdit * tempview = GetView(CurrentFocus);
        QString str = tempview->toPlainText();
        if(str.isEmpty() && str.isNull()) return NULL;
        else{
            str = str.left(pos);
            if(pos != 0){
                str = str.right(1);
                return str;
            }
            else return NULL;
        }
    }
    else return NULL;
}

QTextEdit *Widget::GetView(int viewnum)
{
    switch(viewnum){
    case 0:return ui->view0;break;
    case 1:return ui->view1;break;
    case 2:return ui->view2;break;
    case 3:return ui->view3;break;
    case 4:return ui->view4;break;
    case 5:return ui->view5;break;
    case 6:return ui->view6;break;
    case 7:return ui->view7;break;
    case 8:return ui->view8;break;
    case 9:return ui->view9;break;
    default : return NULL;
    }
    return NULL;
}

int Widget::GetTextEditLineCharNum(int focus)
{
    QTextEdit * tempview = GetView(focus);
    QString str = tempview->toPlainText();
    return str.length();
}

void Widget::TextEditBackspaceEvent()
{
    if((AllNum % (LINE_NUMBER*5)) != 0)BackSpaceFg = 1;
    else BackSpaceFg = 0;
    int linechnum = GetTextEditLineCharNum(CurrentFocus);
    if(linechnum == 0){        
        if(CurrentFocus == 1);
        else{
            //回删前将当前行失能
            QTextEdit *tempview = GetView(CurrentFocus);
            tempview->setEnabled(false);
            //将索引跳到上一行打字输入框
            CurrentFocus -=2;
            //获取光标处上下行的字符，并比较是否相同
            QString str1 = GetTextEditMirrorOrInputChar(0,LINE_NUMBER);
            QString str2 = GetTextEditMirrorOrInputChar(1,LINE_NUMBER);
            if((str1 == str2) && !(str1.isEmpty())){
                CharBeforeBP = 1;
            }
            else CharBeforeBP = 0;
            if(SWITCH)qDebug()<<"Event if CharBeforeBp "<<CharBeforeBP<<str1<<"**"<<str2;
            //不管正确与否,均要将对照行文本还原格式,这个有问题,不会触发槽函数：textchanged
            //SetTexteditCharFormat(CurrentFocus,false,true);
            if(SWITCH)qDebug()<<"Line:375 backspace event linechnum = 0";
            tempview = GetView(CurrentFocus);
            tempview->setEnabled(true);
            QTextCursor cursor = tempview->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.deletePreviousChar();
            tempview->setFocus();
        }
    }
    else{
        QString str1 = GetTextEditMirrorOrInputChar(0,linechnum);
        QString str2 = GetTextEditMirrorOrInputChar(1,linechnum);
        if((str1 == str2) && !(str1.isEmpty())){
            CharBeforeBP = 1;
        }
        else CharBeforeBP = 0;
        if(SWITCH)qDebug()<<"Event else CharBeforeBp "<<CharBeforeBP<<str1<<"**"<<str2;

        //不管正确与否,均要将对照行文本还原格式
        //SetTexteditCharFormat(CurrentFocus,false,true);
        if(SWITCH)qDebug()<<"Line:375 backspace event linechnum != 0";
        QTextEdit *tempview = GetView(CurrentFocus);
        QTextCursor cursor = tempview->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.deletePreviousChar();
    }
}

//回删字符之后的处理
void Widget::TextEditProcess()
{
    if(SWITCH)qDebug()<< "3 BackSpaceFg:" << BackSpaceFg;
    if(BackSpaceFg == 1){
        BackSpaceFg = 0;
        BackSpaceStatus++;
        if(BackSpaceStatus == 1){
            if(CharBeforeBP == 1){
                CorrectNum--;
                if(SWITCH)qDebug()<<"CorrectNum--";
            }
            else{
                ErrorNum--;
                if(SWITCH)qDebug()<<"ErrorNum--";
            }
        }
        else{
            if(CharBeforeBP == 1){
                CorrectNum--;
            }
            else{
                ErrorNum--;
            }
        }
    }
    else{//非回删，即输入字符促发槽函数(textchanged)
        int linechnum = GetTextEditLineCharNum(CurrentFocus);
        if(SWITCH)qDebug()<<"4 After input: linechnum="<< linechnum;
        //获取对照文本的光标处字符
        QString str1 = GetTextEditMirrorOrInputChar(0,linechnum);
        //获取光标处的输入字符
        QString str2 = GetTextEditMirrorOrInputChar(1,linechnum);
        if(SWITCH)qDebug()<<"5 mirror ch ="<<str1;
        if(SWITCH)qDebug()<<"6 input ch ="<<str2;

        BackSpaceStatus = 0;
        if((str1 == str2) && !(str1.isEmpty())){
            CorrectNum++;
            //接下来改变为正确的字体颜色或背景颜色提示
            SetTexteditCharFormat(CurrentFocus,true,false);
        }
        else {
            ErrorNum++;
            //接下来改变为错误的字体颜色或背景颜色提示
            SetTexteditCharFormat(CurrentFocus,false,false);
        }
    }
}

void Widget::KeyEscapeEvent()
{
    if(eventData != -1){
        QString tipStr = nickName+",你要走了吗?";
        switch( QMessageBox::information( this,
            tr("[离开练习]"),                            //dialog‘title
            tipStr,                                     //message
            tr("Yes"), tr("No"),                        //select
        0, 1 ))
        {
            case 0:emit closeSignal();QApplication* app;app->quit();break;
            case 1:
            default:break;
        }
    }
    else if(eventData == -1){
        //发送关闭信号给socket，断开服务器连接
        emit closeSignal();
        QApplication* app;app->quit();
    }
}

//事件过滤器：监听文本框聚焦情况
bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->view1){                          //首先判断控件(这里指 VIEW1)
        if (event->type()==QEvent::FocusIn)             //然后再判断控件的具体事件 (这里指获得焦点事件)
        {
            if(SWITCH)qDebug()<<"wathched1 IN";
            //do something..
            if(CurrentFocus != 1)SwitchFocus(CurrentFocus);
        }
        if (event->type()==QEvent::FocusOut)            // 这里指 view1 控件的失去焦点事件
        {
            if(SWITCH)qDebug()<<"wathched1 OUT";
            //do something...
        }
    }
    if (watched==ui->view3){                             //这里来处理view3 , 和处理view1是一样的
        if (event->type()==QEvent::FocusIn)
        {
            if(SWITCH)qDebug()<<"wathched3 IN";
            //do something...
            if(CurrentFocus != 3)SwitchFocus(CurrentFocus);
        }
        if (event->type()==QEvent::FocusOut)
        {
            if(SWITCH)qDebug()<<"wathched3 OUT";
            //do something...
        }
    }
    if (watched==ui->view5)                             //这里来处理view5 , 和处理view1是一样的
    {
        if (event->type()==QEvent::FocusIn)
        {
            if(SWITCH)qDebug()<<"wathched5 IN";
            //do something...
            if(CurrentFocus != 5)SwitchFocus(CurrentFocus);
        }
        if (event->type()==QEvent::FocusOut)
        {
            if(SWITCH)qDebug()<<"wathched5 OUT";
            //do something...
        }
    }
    if (watched==ui->view7)                             //这里来处理view7 , 和处理view1是一样的
    {
        if (event->type()==QEvent::FocusIn)
        {
            if(SWITCH)qDebug()<<"wathched7 IN";
            //do something...
            if(CurrentFocus != 7)SwitchFocus(CurrentFocus);
        }
        if (event->type()==QEvent::FocusOut)
        {
            if(SWITCH)qDebug()<<"wathched7 OUT";
            //do something...
        }
    }
    if (watched==ui->view9)                             //这里来处理view9 , 和处理view1是一样的
    {
        if (event->type()==QEvent::FocusIn)
        {
            if(SWITCH)qDebug()<<"wathched9 IN";
            //do something...
            if(CurrentFocus != 9)SwitchFocus(CurrentFocus);
        }
        if (event->type()==QEvent::FocusOut)
        {
            if(SWITCH)qDebug()<<"wathched9 OUT";
            //do something...
            ui->view1->setEnabled(true);
        }
    }
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *e = static_cast<QKeyEvent*>(event);
        switch (e->key())
        {//如果采用keyPressEvent在文本框中只能检测到key_escape;加上与不加上return true;有很大区别
         //没加上会是实际效果，加上了就不会处理。
         case Qt::Key_Backspace  : {
                    if(SWITCH)qDebug()<< "key backspace!";
                    TextEditBackspaceEvent();
                    return true;                        //必须有return ture否则backspace会产生效果。
                    break;
                 }
         case Qt::Key_Return     : if(SWITCH)qDebug()<<"Key_Return"; return true;break;
         case Qt::Key_Enter      : if(SWITCH)qDebug()<<"Key_Enter";return true; break;
         case Qt::Key_Delete     : if(SWITCH)qDebug()<<"Key_Delete";return true; break;
         case Qt::Key_Tab        : if(SWITCH)qDebug()<<"Key_Tab";return true; break;
         case Qt::Key_Escape     : {
                if(SWITCH)qDebug()<<"Key_Escape";
                KeyEscapeEvent();
                return true; break;
            }
        }
    }
    return QWidget::eventFilter(watched,event);         // 最后将事件交给上层对话框
}

void Widget::closeEvent(QCloseEvent *event)
{
    if(eventData != -1){
        QString tipStr = nickName+",你要走了吗?";
        switch( QMessageBox::information( this,
          tr("[离开练习]"),                             //dialog‘title
          tipStr,                                      //message
          tr("Yes"), tr("No"),                         //select
        0, 1 ))
        {
            case 0:event->accept();emit closeSignal();break;
            case 1:
            default:event->ignore();break;
        }
    }
    else if(eventData == -1){
        //发送关闭信号给socket，断开服务器连接
        emit closeSignal();
        QApplication* app;app->quit();
    }

}

//-------------------------------------------------------
//槽函数处理
//-------------------------------------------------------
void Widget::on_view1_textChanged()
{
    if(isSetChar == true){
        if(SWITCH)qDebug()<<"view1 isSetchar enter...";
        isSetChar = false;return ;
    }
    if(SWITCH)qDebug()<<"2 View1 textchanged";
    int linechnum = GetTextEditLineCharNum(CurrentFocus);

    TextEditProcess();
    AllNum = CorrectNum + ErrorNum;
    if(SWITCH)qDebug()<<"AllNum="<<AllNum;
    if(SWITCH)qDebug()<<"CorrectNum="<<CorrectNum;
    if(SWITCH)qDebug()<<"ErrorNum"<<ErrorNum;
    if(linechnum > LINE_NUMBER-1){
        CurrentFocus = 3;
        ui->view1->setEnabled(false);
        ui->view1->clearFocus();
        ui->view3->setEnabled(true);
        //换到下一行输入
        ui->view3->setFocus();
    }
}

//view3
void Widget::on_view3_textChanged()
{
    if(isSetChar == true){
        if(SWITCH)qDebug()<<"view1 isSetchar enter...";
        isSetChar = false;return ;
    }
    if(SWITCH)qDebug()<<"View3 textchanged";
    int linechnum = GetTextEditLineCharNum(CurrentFocus);

    TextEditProcess();
    AllNum = CorrectNum + ErrorNum;
    if(SWITCH)qDebug()<<"AllNum="<<AllNum;
    if(SWITCH)qDebug()<<"CorrectNum="<<CorrectNum;
    if(SWITCH)qDebug()<<"ErrorNum"<<ErrorNum;
    if(linechnum > LINE_NUMBER-1){
        CurrentFocus = 5;
        ui->view3->setEnabled(false);
        ui->view3->clearFocus();
        ui->view5->setEnabled(true);
        //换到下一行输入
        ui->view5->setFocus();
    }
}

//view5
void Widget::on_view5_textChanged()
{
    if(isSetChar == true){isSetChar = false;return ;}
    if(SWITCH)qDebug()<<"View5 textchanged";
    int linechnum = GetTextEditLineCharNum(CurrentFocus);

    TextEditProcess();
    AllNum = CorrectNum + ErrorNum;
    if(SWITCH)qDebug()<<"AllNum="<<AllNum;
    if(SWITCH)qDebug()<<"CorrectNum="<<CorrectNum;
    if(SWITCH)qDebug()<<"ErrorNum"<<ErrorNum;
    if(linechnum > LINE_NUMBER-1){
        CurrentFocus = 7;
        ui->view5->setEnabled(false);
        ui->view5->clearFocus();
        ui->view7->setEnabled(true);
        //换到下一行输入
        ui->view7->setFocus();
    }
}

//view7
void Widget::on_view7_textChanged()
{
    if(isSetChar == true){isSetChar = false;return ;}
    if(SWITCH)qDebug()<<"View7 textchanged";
    int linechnum = GetTextEditLineCharNum(CurrentFocus);

    TextEditProcess();
    AllNum = CorrectNum + ErrorNum;
    if(SWITCH)qDebug()<<"AllNum="<<AllNum;
    if(SWITCH)qDebug()<<"CorrectNum="<<CorrectNum;
    if(SWITCH)qDebug()<<"ErrorNum"<<ErrorNum;
    if(linechnum > LINE_NUMBER-1){
        CurrentFocus = 9;
        ui->view7->setEnabled(false);
        ui->view7->clearFocus();
        ui->view9->setEnabled(true);
        //换到下一行输入
        ui->view9->setFocus();
    }
}

//view9
void Widget::on_view9_textChanged()
{
    if(isSetChar == true){isSetChar = false;return ;}
    if(SWITCH)qDebug()<<"View9 textchanged";
    int linechnum = GetTextEditLineCharNum(CurrentFocus);

    TextEditProcess();
    AllNum = CorrectNum + ErrorNum;
    if(SWITCH)qDebug()<<"AllNum="<<AllNum;
    if(SWITCH)qDebug()<<"CorrectNum="<<CorrectNum;
    if(SWITCH)qDebug()<<"ErrorNum"<<ErrorNum;
    if(linechnum > LINE_NUMBER-1){
        //当前页已经练习完。
        TextEditClear();
        if(NULL != (RestContent = ViewGetTextFromContent(RestContent,RestLength))){
        }
        else {
            RestContent = Content;
            RestContent = ViewGetTextFromContent(RestContent,RestLength);
        }
        CurrentFocus = 1;
        ui->view9->setEnabled(false);
        ui->view9->clearFocus();
        //换到下一行输入
        ui->view1->setFocus();
    }
}

void Widget::on_view1_cursorPositionChanged()
{
    if(SWITCH)qDebug()<<"1 View1 cursorPositionChanged";
    //将光标移动到文本结尾
    ui->view1->moveCursor(QTextCursor::End);
}

void Widget::on_view3_cursorPositionChanged()
{
    if(SWITCH)qDebug()<<"View3 cursorPositionChanged";
    //将光标移动到文本结尾
    ui->view3->moveCursor(QTextCursor::End);

}

void Widget::on_view5_cursorPositionChanged()
{
    if(SWITCH)qDebug()<<"View5 cursorPositionChanged";
    //将光标移动到文本结尾
    ui->view5->moveCursor(QTextCursor::End);

}

void Widget::on_view7_cursorPositionChanged()
{
    if(SWITCH)qDebug()<<"View7 cursorPositionChanged";
    //将光标移动到文本结尾
    ui->view7->moveCursor(QTextCursor::End);

}

void Widget::on_view9_cursorPositionChanged()
{
    if(SWITCH)qDebug()<<"View9 cursorPositionChanged";
    //将光标移动到文本结尾
    ui->view9->moveCursor(QTextCursor::End);

}

//-----------------------------------------------------------
//窗口移动、缩放处理部分
//-----------------------------------------------------------
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

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    if(isLeftPressed)
        isLeftPressed=false;
    //恢复鼠标指针形状
    QApplication::restoreOverrideCursor();
    event->ignore();
}

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
                case 11:wid.setTopLeft(wid.topLeft()+ptemp);break;              //左上角
                case 13:wid.setTopRight(wid.topRight()+ptemp);break;            //右上角
                case 31:wid.setBottomLeft(wid.bottomLeft()+ptemp);break;        //左下角
                case 33:wid.setBottomRight(wid.bottomRight()+ptemp);break;      //右下角
                case 12:wid.setTop(wid.top()+ptemp.y());break;                  //中上角
                case 21:wid.setLeft(wid.left()+ptemp.x());break;                //中左角
                case 23:wid.setRight(wid.right()+ptemp.x());break;              //中右角
                case 32:wid.setBottom(wid.bottom()+ptemp.y());break;            //中下角
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
        //QApplication::restoreOverrideCursor();//恢复鼠标指针性状
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
//getDataThread线程从缓冲区中取出接收到信息，然后处理，显示到UI上
//-----------------------------------------------------------
bool compareQVariantMap(const QVariantMap &map1,const QVariantMap &map2)
{
    if(map1["speed"].toInt() > map2["speed"].toInt())return true;
    return false;
}

void Widget::getDataSlot(QVariantMap jsonMap)
{
    if(SWITCH)qDebug()<< "-----------s-----------";
    if(SWITCH)qDebug()<< "Header:" << jsonMap["Header"].toString();
    if(SWITCH)qDebug()<< "rip:" << jsonMap["rip"].toString();
    if(SWITCH)qDebug()<< "speed:" << jsonMap["speed"].toString();
    if(SWITCH)qDebug()<< "name:" << jsonMap["name"].toString();
    if(SWITCH)qDebug()<< "time:" << jsonMap["time"].toString();
    if(SWITCH)qDebug()<< "-----------e-----------";

    QString header = jsonMap["Header"].toString();

    if(header == "LOGIN"){
        QVector<QVariantMap>::iterator iter = typerData.begin();
        int num = 0;
        while(iter != typerData.end()){
            if((*iter)["rip"].toString() == jsonMap["rip"].toString()){
                //已经登录了
                break;
            }
            ++iter;
            ++num;
        }
        //不存在该用户，添加
        if(iter == typerData.end()){
            typerData.append(jsonMap);
        }
    }
    else if(header == "LEAVE"){
        QVector<QVariantMap>::iterator iter = typerData.begin();
        int num = 0;
        while(iter != typerData.end()){
            if(SWITCH)qDebug()<< "[LEAVE]";
            if((*iter)["rip"].toString() == jsonMap["rip"].toString()){
                typerData.removeAt(num);
                break;
            }
            ++iter;
            ++num;
        }
    }
    else if(header == "DATA"){
        QVector<QVariantMap>::iterator iter = typerData.begin();
        int num = 0;
        while(iter != typerData.end()){

            if((*iter)["rip"].toString() == jsonMap["rip"].toString()){
                typerData.replace(num,jsonMap);
                break;
            }
            ++num;
            ++iter;
        }
        //不存在该用户，添加
        if(iter == typerData.end()){
            typerData.append(jsonMap);
        }
    }

    //排序
    qSort(typerData.begin(),typerData.end(),compareQVariantMap);
    //更新显示
    onlineNum = typerData.size();

    ui->labelOnlineNum->setText("当前在线人数 ["+QString::number(onlineNum) + "]");
    //第一名
    if(onlineNum >= 1)
        ui->labelFirst->setText("第一名:" + typerData[0]["name"].toString() +
                "速度 " + typerData[0]["speed"].toString());
    else
        ui->labelFirst->setText("");
    //第二名
    if(onlineNum >= 2)
        ui->labelSecond->setText("第二名:" + typerData[1]["name"].toString() +
                "速度 " + typerData[1]["speed"].toString());
    else
        ui->labelSecond->setText("");
    //第三名
    if(onlineNum >= 3)
        ui->labelThird->setText("第三名:" + typerData[2]["name"].toString() +
                "速度 " + typerData[2]["speed"].toString());
    else
        ui->labelThird->setText("");    
}

//-----------------------------------------------------------
//ReceiveThread
//-----------------------------------------------------------
ReceiveThread::ReceiveThread()
{

}

ReceiveThread::ReceiveThread(QString ip,QString port,QString nickName)
{    
    client = new TcpClientSocket(ip,port,nickName);

    queueData.clear();

    connect(client,SIGNAL(updateClients(QString,int)),
            this,SLOT(updateQueueData(QString,int)));
}

ReceiveThread::~ReceiveThread()
{
    delete client;
}

void ReceiveThread::run()
{
    if(SWITCH)qDebug()<< "ReceiveThread is running...";
    this->exec();
}

void ReceiveThread::closeWidgetSlot()
{
    //发送断开连接
    //client->disconnected();
    if(SWITCH)qDebug()<< "closeWidgetSlot";
}

void ReceiveThread::updateClientDataSlot(QString jsonStr)
{
    int length = 0;
    if((length = client->write(jsonStr.toUtf8(),jsonStr.length())) != jsonStr.length())
    {
        //可能是没有开启服务端的原因，这里可以做下处理
        if(SWITCH)qDebug()<< "updateClientDataSlot write failed!";
        return;
    }
}

void ReceiveThread::updateQueueData(QString msg,int len)
{
    //超过了设置的最大连接数
    if(toGetData.available() == BUFFER_SIZE){
        mutex.lock();
        buffer_full.wait(&mutex);
        mutex.unlock();
    }

    toReceive.acquire();
    mutex2.lock();
    if(SWITCH)qDebug()<< "r mutex2.lock";
    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(msg.left(len).toUtf8(),&jsonError);

    if(jsonError.error == QJsonParseError::NoError){
        if(jsonResponse.isObject()){
            if(SWITCH)qDebug()<< "[receivethread] is object";
            QJsonObject obj = jsonResponse.object();
            QVariantMap jsonMap = obj.toVariantMap();
            //jsonMap.insert("port",port);
            //jsonMap["rip"].toString();
            //jsonMap.insert("ip",address.toString());
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

void GetDataThread::run()
{
    if(SWITCH)qDebug()<< "GetDataThread is running...";
    while(1){
        if(toReceive.available() == BUFFER_SIZE){
            mutex.lock();
            buffer_empty.wait(&mutex);
            mutex.unlock();
        }
        toGetData.acquire();
        mutex2.lock();
        if(SWITCH)qDebug()<< "g mutex2.lock()";
        QVariantMap msg;
        if(queueData.size() > 0){
            msg = queueData.last();
            queueData.pop_back();
        }
        mutex2.unlock();
        toReceive.release();
        buffer_full.wakeAll();
        emit dataGeted(msg);
        if(SWITCH)qDebug()<< "[G]queueData.size=" << queueData.size();

        //一秒获取一次数据，也可不用
        //sleep(1);
    }
}






#include "login.h"
#include "ui_login.h"
#include <QDesktopWidget>
#include <QMouseEvent>

Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    loginInit();
}

Login::~Login()
{
    delete ui;
}

void Login::closeEvent(QCloseEvent *event)
{
    if(SWITCH)qDebug()<< "closeEvent";
    emit loginSendEventData(-1);
}

//登录初始化
void Login::loginInit()
{
    //获取最小化、关闭按钮图标
    QPixmap minPix  = style()->standardPixmap(QStyle::SP_TitleBarMinButton);
    QPixmap closePix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);

    //设置最小化、关闭按钮图标
    ui->minButton->setIcon(minPix);
    ui->closeButton->setIcon(closePix);

    ui->minButton->setToolTip(tr("最小化"));
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
}

//-----------------------------------------------------------
//窗口移动、缩放处理部分
//-----------------------------------------------------------
void Login::mousePressEvent(QMouseEvent *event)
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

void Login::mouseReleaseEvent(QMouseEvent *event)
{
    if(isLeftPressed)
        isLeftPressed=false;
    //恢复鼠标指针形状
    QApplication::restoreOverrideCursor();
    event->ignore();
}

void Login::mouseDoubleClickEvent(QMouseEvent *event)
{
//    if(event->button()==Qt::LeftButton)
//    {
//        if(windowState()!=Qt::WindowFullScreen)
//            //双击窗口全屏
//            setWindowState(Qt::WindowFullScreen);
//        //恢复正常模式
//        else setWindowState(Qt::WindowNoState);
//    }
//    event->ignore();
}

void Login::mouseMoveEvent(QMouseEvent *event)
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
int Login::countFlag(QPoint p,int row)
{
    if(p.y()< MARGIN)
        return 10+row;
    else if(p.y()>this->height() - MARGIN)
        return 30+row;
    else
        return 20+row;
}
//根据鼠标所在位置改变鼠标指针形状
void Login::setCursorType(int flag)
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

int Login::countRow(QPoint p)
{
    return (p.x()<MARGIN)?1:(p.x()>(this->width()-MARGIN)?3:2);
}

//-----------------------------------------------------------
//最大小、关闭按钮槽函数
//-----------------------------------------------------------
void Login::on_minButton_clicked()
{
    this->setWindowState(Qt::WindowMinimized);
}

void Login::on_closeButton_clicked()
{
    this->close();
}

void Login::on_pushButtonLogin_clicked()
{
    QString ip = ui->lineEditIP->text();
    QString port = ui->lineEditPort->text();
    QString nickName = ui->lineEditNickName->text();

    if(SWITCH)qDebug()<< ip <<" " << nickName;
    //正则表达式，判断给定的IP地址能否被正确解析
    QRegExp regExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    int pos = regExp.indexIn(ip);

    if(pos == -1)
    {
        ui->labelTip->setText("IP地址填写错误!");
        return;
    }

    if(port.length() != 4){
        ui->labelTip->setText("端口号长度设定为4!");
        return;
    }
    if(nickName.length() < 3){
        ui->labelTip->setText("昵称长度不够!");
        return;
    }
    ui->labelTip->setText("");

    //向主界面发送登录数据
    emit loginSendSetData(ip,port,nickName);
}

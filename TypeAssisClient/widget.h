#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTextEdit>

#include "login.h"
#include "receivethread.h"
#include "getdatathread.h"
#include "tcpclientsocket.h"

#define LINE_NUMBER 51                      //一行能够显示的字符数
#define MARGIN 5                            //拖动窗口边界，实现放大缩小所需拖动区边界像素大小
#define BUFFER_SIZE   100                   //最大100个用户的缓冲数据
#define SWITCH   0


class ReceiveThread;
class GetDataThread;

namespace Ui {
class Widget;
class ReceiveThread;
class GetDataThread;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
//登录相关
private:
    //昵称
    QString nickName;
    //关闭窗口事件形式，ESC || Close ?
    int eventData;
private slots:
    void widgetReceiveSetData(const QString&,const QString&,const QString&);
    void widgetReceiveEventData(const int&);
//打字用户数据处理
private:
    int onlineNum;
    QVector<QVariantMap> typerData;    
    QString no1Typer;
    QString no2Typer;
    QString no3Typer;
signals:
    void closeSignal();
    void updateClientDataSignal(QString);
//线程相关
private:
    ReceiveThread * receiveThread;
    GetDataThread * getDataThread;
//窗口处理数据槽函数
private slots:
    void getDataSlot(QVariantMap);

//窗口初始化
private:
    Ui::Widget *ui;
    Login *login;

    void initUi();
    void TimerInit();
    void SetProBackground(const QString path);
//鼠标相关
private:
    bool isLeftPressed;
    int curPos;
    QPoint pLast;
    int countFlag(QPoint p, int row);
    void setCursorType(int flag);
    int countRow(QPoint p);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
//最大小、关闭按钮槽函数
private slots:
    void on_minButton_clicked();
    void on_maxButton_clicked();
    void on_closeButton_clicked();

//打字练习部分
private:
    //文章导入相关和处理
    int ShowFileTextInit(const QString textpath);                       //导入并显示文件文本
    QString ViewGetTextFromContent(QString str,unsigned long len);      //各个文本框从文本内容中获取文本【即用来刷新对照文本】
    void SwitchFocus(int focus);
    void setCharBackground(int focus,unsigned int pos,QString color);
    void setCharForeground(int focus,unsigned int pos,QString color);
    void SetTexteditCharFormat(int focus,bool fg,bool isbackspace);
    void TextEditClear();
    QString GetTextEditMirrorOrInputChar(int select,int pos);
    QTextEdit *GetView(int viewnum);
    int GetTextEditLineCharNum(int focus);
    void TextEditBackspaceEvent();
    void TextEditProcess();
    void KeyEscapeEvent();                  //按退出键处理函数
private:
    QString Content;
    unsigned long Length;                   //文件长度
    unsigned long RestLength;               //显示到文本框后，文本剩余长度
    QString RestContent;                    //除去显示完了的文本，即剩下还未显示的文本
    int ct;                                 //行字符个数计数
    int CurrentFocus;                       //当前聚焦
    int BackSpaceFg;                        //回删标记
    int BackSpaceStatus;                    //回删状态

    unsigned long CorrectNum;               //正确个数
    unsigned long ErrorNum;                 //错误个数
    unsigned long AllNum;                   //总个数 = 正确个数+错误个数
    unsigned long TimeCount;                //
    int CharAfterBP;                        //回删之后上下两行文本状态：正确[same] or 错误[different]
    int CharBeforeBP;                       //回删之前
    bool isSetChar;

private slots:
    void on_view9_cursorPositionChanged();
    void on_view7_cursorPositionChanged();
    void on_view5_cursorPositionChanged();
    void on_view3_cursorPositionChanged();
    void on_view1_cursorPositionChanged();
    void on_view9_textChanged();
    void on_view7_textChanged();
    void on_view5_textChanged();
    void on_view3_textChanged();
    void on_view1_textChanged();
public slots:
    bool eventFilter(QObject *,QEvent *);  //事件过滤器来监听文本框聚焦情况
    void closeEvent(QCloseEvent *event);   //拦截关闭按钮
    void timerHandleSlot();

};

#endif // WIDGET_H

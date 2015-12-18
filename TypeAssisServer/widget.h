#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "server.h"
#include "receivethread.h"
#include "getdatathread.h"


#define TABWIDGET_COLUMN 6
#define MARGIN 5
#define SWITCH  0

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
//用户列表操作
public:
    void addANewUser(QString *);
    void updateUserData(QString *);
    void deleteUser(QString *);

//窗口初始化
private:
    void initUi();

private:
    Ui::Widget *ui;
    Server * server;
    int port;
    int maxConnectNum;
    ReceiveThread * receiveThread;
    GetDataThread * getDataThread;
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
//按钮槽函数
private slots:
    void on_minButton_clicked();
    void on_maxButton_clicked();
    void on_closeButton_clicked();
//服务器处理槽函数
public slots:
    void slotCreateServer();
    void getDataSlot(QVariantMap);
};

#endif // WIDGET_H

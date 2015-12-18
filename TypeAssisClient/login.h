#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QtNetwork>

#define MARGIN 5
#define SWITCH     0

namespace Ui {
    class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();
    void loginInit();
private:
    void closeEvent(QCloseEvent *event);

private:
    Ui::Login *ui;
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
    void on_closeButton_clicked();
    void on_pushButtonLogin_clicked();

signals:
    void loginSendSetData(QString,QString,QString);
    void loginSendEventData(int);
};

#endif // LOGIN_H

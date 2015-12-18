#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    //需登录才能显示主界面
    //w.show();

    return a.exec();
}

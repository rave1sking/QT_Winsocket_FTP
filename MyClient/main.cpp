#include "widget.h"

#include <QApplication>
#include "login.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    Login m;
    m.show();
    //点击登录对话框的登录按钮，进入主界面
    QObject::connect(&m,SIGNAL(mainshow()),&w,SLOT(show_mainwin()));
    //点击主界面的注销，返回登录对话框
    QObject::connect(&w,SIGNAL(Exit()),&m,SLOT(show_loginwin()));

    return a.exec();
}

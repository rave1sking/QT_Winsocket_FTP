#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDebug>
#include <winsock2.h>
#include <server.h>
#include <clithread.h>
#include <QMessageBox>

#pragma comment(lib, "ws2_32.lib")

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    //界面美化
    void Construct();
    //初始化服务器
    void SerInit();

private slots:
    //打开服务器
    void OpenSer();
    //消息框内容更新
    void MsgUpdate(QString msg);
    //服务器关闭
    void SerExit();

public:
    QTextBrowser *message;
    QPushButton *openser;
    QPushButton *exit;
    QLineEdit *serport;

    QMessageBox ExitLog;
    QPushButton *ExitL=new QPushButton(QObject::tr("退出"));

private:
    WSADATA wasData;
    SOCKET mSerSocket = INVALID_SOCKET;
    CliThread *mCliThread;
    int port;
};
#endif // WIDGET_H

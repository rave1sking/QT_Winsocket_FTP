#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QTextBrowser>
#include <windows.h>
#include "client.h"
#include <QDebug>
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <QFileDialog>
#include <QLineEdit>
#include <QInputDialog>
#include <imagehlp.h>
#include <QTextStream>

//QTextStream out(stdout);

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void Construct();

public:
    QPushButton *view_folder;
    QPushButton *file_dir;
    QPushButton *userexit;
    QPushButton *download;
    QPushButton *upload;
    QTextBrowser *message_show;

public:
    //客户端向服务器发送请求
    bool Send_Order(SOCKET CliSocket, CmdGram *Command);
    //接收服务器的回复消息
    bool Read_Resp(SOCKET CliSocket, ResponseGram *Response);

signals:
    void Exit();

private slots:
    void show_mainwin();
    void UserExit();

    QString ViewFolder(SOCKET Soc);
    QString FileDir();
    QString Download();
    QString Upload();

};
#endif // WIDGET_H

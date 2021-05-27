#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QDialog>
#include <QIcon>
#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <windows.h>
#include <client.h>
#include <tchar.h>
#include <iostream>
#include <QFileDialog>
#include <QInputDialog>
#include <imagehlp.h>

class Login : public QDialog
{
    Q_OBJECT
public:
    explicit Login(QWidget *parent = nullptr);

    void Construct();

public:
    QLineEdit *my_severloc;
    QString serip;
    QPushButton *connectser;
    QLineEdit *my_port;
    QString serport;

    QLineEdit *my_username;
    QString user;
    QLineEdit *my_password;
    QString psd;
    QPushButton *cli_login;
    const QString Name = "yyy";
    const QString PassWord = "20210521";
    int chances;

    QMessageBox FailedLog, ExitLog;
    QPushButton *Retry=new QPushButton(QObject::tr("重试"));
    QPushButton *ExitL=new QPushButton(QObject::tr("退出"));


public:
    QString IP_NUM;
    QString Port_NUM;

    WSADATA wasData;

signals:
    void mainshow();

public slots:
    void Login_clicked();
    void show_loginwin();

    void ConnectSer();
};

#endif // LOGIN_H

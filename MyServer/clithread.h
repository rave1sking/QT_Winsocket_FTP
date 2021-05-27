#ifndef CLITHREAD_H
#define CLITHREAD_H

#include <QWidget>
#include <QThread>
#include <winsock2.h>
#include <QDebug>
#include <QString>
#include <msgthread.h>

class CliThread : public QThread
{
    Q_OBJECT
public:
    explicit CliThread(SOCKET mListen, QWidget *parent = nullptr);
    void run();
    ~CliThread();

public:
    QString msg;

signals:
    //cli发送过来的所有信息需要传递给ui界面更新，均采用此信号
    void isMsg(QString msg);
    //clithread关闭信号
    void isClose();

public slots:
    //传递文件传输线程中的信号到ui界面的槽函数
    void sendMsg(QString msg);

private:
    SOCKET mListen;
    QWidget *parent;

};

#endif // CLITHREAD_H

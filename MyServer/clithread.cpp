#include "clithread.h"

CliThread::CliThread(SOCKET mListen, QWidget *parent) : QThread(parent)
{
    this->mListen = mListen;
    this->parent = parent;
}

CliThread::~CliThread()
{
    closesocket(mListen);
    emit isClose();
}

void CliThread::run()
{
    //客户端地址
    sockaddr_in cli_address;
    int size = sizeof(cli_address);

    while(!isInterruptionRequested())
    {
        //每次接收新客户端时，将之前的地址信息清0
        memset(&cli_address, 0, sizeof(cli_address));

        //等待新客户端连接
        SOCKET cli_socket = accept(mListen, (SOCKADDR*)&cli_address, &size);
        if(cli_socket == INVALID_SOCKET)
        {
            msg = "创建cli_socket失败";
            emit isMsg(msg);
            return;
        }

        msg = "客户端连接成功";
        emit isMsg(msg);

        //开启新线程与客户端进行通信
        MsgThread* FileThread = new MsgThread(cli_socket, cli_address, parent);
        FileThread->start();
        //绑定信号和槽，转发消息给ui进程进行界面更新
        connect(FileThread, &MsgThread::isMsg, this, &CliThread::sendMsg);

        connect(this, &CliThread::isClose,
                [=]()
        {
            msg = "客户端断开连接";
            emit isMsg(msg);
            FileThread->terminate();
            FileThread->quit();
            delete FileThread;
        });
    }
}

void CliThread::sendMsg(QString msg)
{
    emit isMsg(msg);
    return;
}

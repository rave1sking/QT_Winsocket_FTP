#ifndef MSGTHREAD_H
#define MSGTHREAD_H

#include <QWidget>
#include <QThread>
#include <winsock2.h>
#include <QDebug>
#include <QString>
#include <server.h>
#include <cstring>
#include <iostream>
#include <io.h>

class MsgThread : public QThread
{
    Q_OBJECT
public:
    explicit MsgThread(SOCKET mClient, SOCKADDR_IN mCli_Addr, QWidget *parent = nullptr);
    ~MsgThread();
    void run();

    //ser与cli之间的对接
    bool SerToCli(SOCKET Ser_Cmd_Socket, CmdGram* Command, SOCKADDR_IN *Cli_Data_Port);
    //与cli进行数据连接
    bool DataLink(SOCKET *Ser_Data_Socket, SOCKADDR_IN *Cli_Data_Port);
    //向cli发送报文
    bool SendtoCli(SOCKET Ser_Cmd_Socket, ResponseGram* Response);
    //接收cli的报文
    bool ReceiveCli(SOCKET Ser_Cmd_Socket, char* Command);
    //向cli发送文件
    bool SendFiletoCli(SOCKET Ser_Data_Socket, char* FileName);
    //从cli接收文件
    bool ReceiveFile(SOCKET Ser_Data_Socket, char* FileName);
    //给cli发送文件列表
    bool SendFileList(SOCKET Ser_Data_Socket);
    //返回文件的具体信息
    void ReturnFileInfo(struct _finddata32_t* File_Data, char* fileInfo);

private:
    //与客户端进行文件传输通信的套接字
    SOCKET mClient;
    //客户端地址
    SOCKADDR_IN mCli_Addr;

signals:
    //客户端的相关操作通过此信号传输
    void isMsg(QString msg);

public slots:
};

#endif // MSGTHREAD_H

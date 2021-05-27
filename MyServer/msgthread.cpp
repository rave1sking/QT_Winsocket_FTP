#include "msgthread.h"

MsgThread::MsgThread(SOCKET mClient, SOCKADDR_IN mCli_Addr, QWidget *parent) : QThread(parent)
{
    this->mClient = mClient;
    this->mCli_Addr = mCli_Addr;
}

MsgThread::~MsgThread()
{
    int ret = closesocket(mClient);
    if( ret == 0 )
    {
        qDebug()<< "关闭成功";
    }
    else
    {
        qDebug()<< "关闭失败";
    }
}

void MsgThread::run()
{
    //给客户端发送报文
    ResponseGram responseCli;
    responseCli.response = SUCCESS;
    strcpy(responseCli.content, "SUCCESS");
    if(SendtoCli(mClient, &responseCli));
    else
    {
        emit isMsg("为客户端创建线程成功，但与客户端发送信息失败");
    }

    //开始获取报文命令
    CmdGram commandCli;
    while(true)
    {
        if(ReceiveCli(mClient, (char*)&commandCli));
        else
        {
            emit isMsg("获取客户端命令失败");
            break;
        }
        if(SerToCli(mClient, &commandCli, &mCli_Addr));
        else
        {
            emit isMsg("获取客户端命令成功，但服务器未响应");
            break;
        }
    }

    closesocket(mClient);
}

bool MsgThread::SerToCli(SOCKET Ser_Cmd_Socket, CmdGram *Command, SOCKADDR_IN *Cli_Data_Port)
{
    SOCKET Ser_Data_Socket;
    FILE* fp = NULL;
    ResponseGram* response;
    //回复报文，从ser发往cli
    ResponseGram Response;
    Response.response = SUCCESS;
    strcpy(Response.content, "SUCCESS");

    response = &Response;

    //cli从ser下载文件
    if(Command->cmd == DOWNLOAD)
    {
        emit isMsg("收到下载文件命令");
        fp = fopen(Command->content, "rb");//直接打开命令所在的的文件目录
        if(fp == NULL)
        {
            response->response = ERR;
            strcpy(response->content, "打开文件错误\n\n");

            //发送错误报文
            if(SendtoCli(Ser_Cmd_Socket, response))
            {
                emit isMsg("与客户端报告错误失败");
                return false;
            }
            return false;
        }
        else
        {
            //发送成功报文
            response->response = SUCCESS;
            strcpy(response->content, "打开成功，正在传送......");
            if(SendtoCli(Ser_Cmd_Socket, response))//发送报文过后开始传输文件
            {
                //建立连接
                if(DataLink(&Ser_Data_Socket, Cli_Data_Port))
                {
                    if(SendFiletoCli(Ser_Data_Socket, Command->content))//发送文件
                    {
                        fclose(fp);
                        return true;
                    }
                    else
                        return false;
                }
                else
                    return false;
            }
            else
                return false;
        }
    }
    else if(Command->cmd == UPLOAD)//上传文件
    {
        emit isMsg("收到上传文件命令");
        //先检查服务器中是否存在同名文件
        char fileName[128];
        strcpy(fileName, Command->content);

        FILE* fp = fopen(fileName, "r+");

        if(fp != NULL)//文件能够打开，说明服务器中存在同名文件，提出警告
        {
            response->response = SUCCESS;
            strcpy(response->content, "警告：服务器中已存在该文件，将会覆盖文件\n");
            remove(fileName);
        }
        else
        {
            response->response = SUCCESS;
            strcpy(response->content, "SUCCESS");
        }

        //给cli发送报文
        if(SendtoCli(Ser_Cmd_Socket, response));
        else
        {
            emit isMsg("文件符合上传条件，但与客户端通信失败");
            return false;
        }

        //建立数据传输通道
        if(DataLink(&Ser_Data_Socket, Cli_Data_Port));
        else
        {
            emit isMsg("建立数据传输通道失败");
            return false;
        }

        //接收cli的文件
        if(ReceiveFile(Ser_Data_Socket, fileName));
        else
        {
            emit isMsg("接收客户端文件失败");
            return false;
        }

        return true;
    }

    else if(Command->cmd == QUIT)//退出
    {
        emit isMsg("接收到客户端退出命令");
        response->response = SUCCESS;
        strcpy(response->content, "服务器已接收到断开请求");

        if(SendtoCli(Ser_Cmd_Socket, response));
        else
        {
            emit isMsg("服务器已成功接收客户端断开信息，但与客户端通信失败");
            return false;
        }

        closesocket(Ser_Data_Socket);
        return true;
    }

    else if(Command->cmd == LIST)//列出文件目录
    {
        emit isMsg("收到列出文件目录命令");
        //建立连接
        if(DataLink(&Ser_Data_Socket, Cli_Data_Port));
        else
        {
            emit isMsg("建立数据传输连接失败");
            return false;
        }

        //发送文件目录
        if(SendFileList(Ser_Data_Socket))
        {
            emit isMsg("发送文件目录成功");
            return true;
        }
        else
        {
            emit isMsg("发送文件目录失败");
            return false;
        }
    }

    else if(Command->cmd == PWD)//显示文件路径
    {
        emit isMsg("收到显示当前文件路径命令");
        response->response = SUCCESS;
        if(GetCurrentDirectoryA(RESPONSE_CONTENT_SIZE, response->content));
        else
        {
            strcpy(response->content, "获取当前目录失败");
            return false;
        }
        if(SendtoCli(Ser_Cmd_Socket, response))
            return true;
        else
        {
            emit isMsg("获取当前目录成功，但与客户端发送信息失败");
            return false;
        }
    }

    else
    {
        return true;
    }
}

//向cli发送报文
bool MsgThread::SendtoCli(SOCKET Ser_Cmd_Socket, ResponseGram* Response)
{
    if(send(Ser_Cmd_Socket, (char*)Response, sizeof (ResponseGram), 0) == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        emit isMsg("向客户端发送报文失败");
        qDebug()<<error;
        return false;
    }

    emit isMsg("向客户端发送报文成功");
    return true;
}

//接收来自客户端的命令
bool MsgThread::ReceiveCli(SOCKET Ser_Cmd_Socket, char* Command)
{
    int flag;
    int not_recv = sizeof(CmdGram);

    //开始读取数据
    for(; not_recv > 0;)
    {
        flag = recv(Ser_Cmd_Socket, Command, not_recv, 0);
        if(flag == SOCKET_ERROR)
        {
            emit isMsg("接收命令错误或客户端退出");
            return false;
        }
        else
        {
            emit isMsg("接收命令成功");
        }


        //字符串指针加int的意义：偏移
        not_recv -= flag;
        Command += flag;
    }

    return true;
}

//建立数据通道
bool MsgThread::DataLink(SOCKET *Ser_Data_Socket, SOCKADDR_IN *Cli_Data_Port)
{
    //创建ser端数据的socket
    SOCKET ser_data_socket;
    ser_data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(ser_data_socket == INVALID_SOCKET)
    {
        emit isMsg("创建数据传输套接字失败");
        return false;
    }

    //获取客户端数据传输端口号
    SOCKADDR_IN cli_data_port;
    std::memcpy(&cli_data_port, Cli_Data_Port, sizeof(SOCKADDR_IN));
    cli_data_port.sin_port = htons(DATA_PORT_PV);

    //连接
    if(::connect(ser_data_socket, (SOCKADDR*)&cli_data_port, sizeof(SOCKADDR)))
    {
        emit isMsg("与客户端的连接出现问题");
        closesocket(ser_data_socket);
        return false;
    }

    *Ser_Data_Socket = ser_data_socket;
    emit isMsg("与客户端数据传输连接成功");
    return true;
}

//给cli发送文件
bool MsgThread::SendFiletoCli(SOCKET Ser_Data_Socket, char *FileName)
{
    //开始读文件
    char buf[2048];

    FILE *fp = fopen(FileName, "rb");
    if(fp == NULL)
    {
        emit isMsg("文件不存在");
        fclose(fp);
        closesocket(Ser_Data_Socket);
        return false;
    }

    while(1)
    {
        int size = fread(buf, 1, 2048, fp);

        if(send(Ser_Data_Socket, buf, size, 0) == SOCKET_ERROR)
        {
            emit isMsg("发送文件过程中发生错误");
            closesocket(Ser_Data_Socket);
            return false;
        }
        if(size < 2048)
            break;
    }

    fclose(fp);
    closesocket(Ser_Data_Socket);
    emit isMsg("文件发送完成");
    return true;
}

//从cli接收文件
bool MsgThread::ReceiveFile(SOCKET Ser_Data_Socket, char *FileName)
{
    //先创建文件，再读取
    char buf[2048];
    int recv_size;

    FILE* fp = fopen(FileName, "wb");
    if(fp == NULL)
    {
        emit isMsg("创建文件过程中发生错误");
        fclose(fp);
        closesocket(Ser_Data_Socket);
        return false;
    }

    while(1)
    {
        recv_size = recv(Ser_Data_Socket, buf, 2048, 0);
        if(recv_size == SOCKET_ERROR)
        {
            emit isMsg("上传文件时发生错误");
            fclose(fp);
            closesocket(Ser_Data_Socket);
            return false;
        }

        if(recv_size == 0)
            break;

        fwrite(buf, 1, recv_size, fp);
    }

    fclose(fp);
    closesocket(Ser_Data_Socket);
    emit isMsg("完成上传");
    return true;
}

//向cli发送文件列表
bool MsgThread::SendFileList(SOCKET Ser_Data_Socket)
{
    long handle;
    struct _finddata_t File_Data;
    char fileInfo[500];
    const char column[100] = "          Name          |       Updata Time       |     Size(Bytes)     \n";

    //使用通配符找到当前目录，并将其信息存入指针
    handle = _findfirst32("*", &File_Data);

    if(handle == -1)
    {
        emit isMsg("服务器读取文件列表失败");
        if(send(Ser_Data_Socket, "获取文件列表失败", sizeof("获取文件列表失败"), 0));
        else
        {
            emit isMsg("服务器发送信息失败");
            return false;
        }
    }
    else
    {
        send(Ser_Data_Socket, column, sizeof(column), 0);
        while(true)
        {
            //查找下一个文件
            int FindNext = _findnext32(handle, &File_Data);

            if(FindNext == -1)
            {
                emit isMsg("服务器已查找完所有文件");
                break;
            }
            else
            {
                ReturnFileInfo(&File_Data, fileInfo);
                if(send(Ser_Data_Socket, fileInfo, sizeof(fileInfo), 0));
                else
                {
                    emit isMsg("服务器发送信息失败");
                    return false;
                }
            }
        }
    }

    closesocket(Ser_Data_Socket);
    _findclose(handle);
    return true;
}

//返回文件具体信息
void MsgThread::ReturnFileInfo(struct _finddata32_t *File_Data, char *fileInfo)
{
    char fileinfo[500];
    int size;

    time_t fileUpdata_time = File_Data->time_write;
    tm* time_wanted;
    time_wanted = NULL;
    time_wanted = localtime(&fileUpdata_time);

    size = File_Data->size;

    sprintf(fileinfo, "| %24s|   %d/%d/%d %d:%d:%d   | %d \n",
            File_Data->name, 1900 + time_wanted->tm_year, 1 + time_wanted->tm_mon, time_wanted->tm_mday,
            time_wanted->tm_hour, time_wanted->tm_min, time_wanted->tm_sec, size);

    strcpy(fileInfo, fileinfo);
}

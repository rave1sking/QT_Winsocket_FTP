#ifndef SERVER_H
#define SERVER_H

/*
 *与server相关的头文件
 */

//定义一些常数

//client侦听sever数据端口，应大于1024
#define DATA_PORT_PV 5850
//client与sever连接的命令端口，应大于1024
#define CMD_PROT_AC 4096

//命令报文参数缓存的大小
#define CMD_PARAM_SIZE 256
//回复报文消息缓存的大小
#define RESPONSE_CONTENT_SIZE 256
#define BACKLOG 10
#define DATA_BUFSIZE 4096

/*
 * 命令类型：
    LIST:查看文件列表
    PWD:显示当前目录
    DOWNLOAD:下载文件
    UPLOAD:上传文件
    QUIT:退出

    分别对应:int 0~4
 */
typedef enum
{
    LIST,PWD,DOWNLOAD,UPLOAD,QUIT
}CMD;

//命令报文,从客户端发往服务器
typedef struct CmdGram {
    CMD cmd;
    char content[CMD_PARAM_SIZE];
} CmdGram;

//回复报文的类型
typedef enum {
    SUCCESS, ERR
} RESPONSE;

//回复报文,从服务器发往客户端
typedef struct ResponseGram {
    RESPONSE response;
    //回复报文的具体内容
    char content[RESPONSE_CONTENT_SIZE];
} ResponseGram;
#endif // SERVER_H

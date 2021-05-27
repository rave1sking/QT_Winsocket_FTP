#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    Construct();
}

Widget::~Widget()
{
    closesocket(My_Socket);
    WSACleanup();
}

//客户端向服务器发送请求
bool Widget::Send_Order(SOCKET CliSocket, CmdGram *Command)
{
    int size = sizeof(CmdGram);
    if(send(CliSocket, (char*)Command, size, 0) == SOCKET_ERROR)
    {
        message_show->append("向服务器发送请求失败");
        closesocket(CliSocket);
        WSACleanup();
        return false;
    }

    return true;
}

//接收服务器的回复消息
bool Widget::Read_Resp(SOCKET CliSocket, ResponseGram *Response)
{
    int num;
    int size = sizeof(ResponseGram);
    int mark;
    for(num = 0, mark = 1 ; num < size; )
    {
        mark = recv(CliSocket, (char*)Response + num , size - num, 0);
        if(mark <= 0)
        {
            message_show->append("读取服务器回复消息错误");
            closesocket(CliSocket);
            return false;
        }
        num += mark;
    }
    return true;
}

//创建套接字用于接收从服务器端传来的文件信息
SOCKET Create_Socket()
{
    SOCKET Soc;
    SOCKADDR_IN addr;

    unsigned int mark_socket=(Soc=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP));
    if(mark_socket==INVALID_SOCKET)
    {
        qDebug()<<"Create Socket Error";
        WSACleanup();
        exit(-1);
    }

    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_family=AF_INET;
    addr.sin_port=htons(DATA_PORT_PV);
    memset(&(addr.sin_zero), 0, sizeof(addr.sin_zero));

    int mark_bind=bind(Soc,(SOCKADDR *) &addr , sizeof(SOCKADDR));
    if(mark_bind==SOCKET_ERROR)
    {
        qDebug()<<"Bind Addr Error";
        closesocket(Soc);
        WSACleanup();
        exit(-1);
    }
    //
    int mark_listen=listen(Soc,1);
    if(mark_listen==SOCKET_ERROR)
    {
        qDebug()<<"listen error";
        closesocket(Soc);
        WSACleanup();
        exit(-1);
    }
    return Soc;
}


QString Widget::ViewFolder(SOCKET Soc)
{
     QString Sum_Data;
     int size;
     int num;
     CmdGram C_Packet;
     SOCKET new_Soc;
     SOCKET temp_Soc;
     SOCKADDR_IN addr;
     char buffer[DATA_BUFSIZE];

     new_Soc = Create_Socket();
     C_Packet.cmd = LIST;
     Soc = My_Socket;
     Send_Order(Soc,& C_Packet);
     size = sizeof(SOCKADDR_IN);
     unsigned int mark_list = (temp_Soc=accept(new_Soc , (SOCKADDR*) &addr , &size));

     Sum_Data = "";
     while(true)
     {
         num = recv(temp_Soc, buffer, DATA_BUFSIZE-1, 0);
         if(num == SOCKET_ERROR)
         {
             qDebug()<<"Server Error";
             closesocket(temp_Soc);
             closesocket(new_Soc);
             closesocket(Soc);
             WSACleanup();
             exit(-1);
         }
         if(num == 0)
             break;
         Sum_Data += QString(buffer);
     }

     closesocket(new_Soc);
     closesocket(temp_Soc);

     message_show->append("*********************");
     message_show->append(Sum_Data);
     message_show->moveCursor(message_show->textCursor().End);
     return Sum_Data;
}

QString Widget::FileDir()
{
    CmdGram View_Packet;
    ResponseGram Re_View_Packet;
    View_Packet.cmd = PWD; //显示当前目录
    //message_show->setText("*********************");
    Send_Order(My_Socket, &View_Packet);
    Read_Resp(My_Socket, &Re_View_Packet);
    message_show->append("*********************");
    message_show->append(Re_View_Packet.content);
    message_show->moveCursor(message_show->textCursor().End);
    return QString(Re_View_Packet.content);

}


QString Widget::Download()
{
       FILE *file;   //server端的文件
       char buffer[DATA_BUFSIZE];
       CmdGram C_Packet;
       ResponseGram R_Packet;
       SOCKET new_Soc;
       SOCKET temp_Soc;
       SOCKADDR_IN addr;
       int size;
       int num;
       QString Server_pos =  this->FileDir();
       Server_pos = this->FileDir();         //Server 根目录
       //qDebug<<Server_pos;
       QString Server_File = QFileDialog::getOpenFileName(0 , "文件标题" , Server_pos);
       //Server_pos.append(Server_File);
       message_show->append("download ....");
       message_show->append(Server_File);  //文件目录

       message_show->append(Server_pos);
       message_show->moveCursor(message_show->textCursor().End);

       QString file_get = QFileInfo(Server_File).fileName(); //从路径中获取文件名
       message_show->append(file_get);
       message_show->moveCursor(message_show->textCursor().End);

       QByteArray Target = file_get.toLatin1();
       strcpy(C_Packet.content, Target.data());

       QString DownPath = "D:\\yyy\\courses\\NetWork\\yyy NetWork\\MyClient\\" + file_get; //此处可改
       QByteArray Location = DownPath.toLatin1(); //Location为目标路径
       C_Packet.cmd = DOWNLOAD;

       file = fopen(Location.data(), "wb");
       if(file == NULL)
       {
           qDebug()<<"Open File Error";
           return QString("Open File Error");
       }

       new_Soc = Create_Socket();
       Send_Order(My_Socket, &C_Packet);
       Read_Resp(My_Socket, &R_Packet);
       if(R_Packet.response == ERR)
       {
           qDebug()<<"No Right File";
           fclose(file);
           closesocket(new_Soc);
           return QString("No Right File");
       }
       size = sizeof(SOCKADDR_IN);

       temp_Soc = accept(new_Soc,(SOCKADDR *)&addr, &size);
       if(temp_Soc == INVALID_SOCKET)
       {
           qDebug()<<"Accept File Error";
           closesocket(new_Soc);
           fclose(file);
           return QString("Accept File Error");
       }

       for(num = 2; num > 0; )
       {
           num = recv(temp_Soc, buffer, DATA_BUFSIZE, 0);
           fwrite(buffer, sizeof(char), num, file);
       }


       fclose(file);

       closesocket(temp_Soc);
       closesocket(new_Soc);

       qDebug()<<"Get File Successfully";
       return QString("Get File Successfully");
}

QString Widget::Upload()
{
    QString My_File=QFileDialog::getOpenFileName(0 , "文件标题" , ".");
    FILE *file;
    char buffer[DATA_BUFSIZE];
    CmdGram C_Packet;
    ResponseGram R_Packet;
    SOCKET new_Soc;
    SOCKET temp_Soc;
    SOCKADDR_IN addr;
    int size;
    int num;

    QString file_get=QFileInfo(My_File).fileName();
    QByteArray Target=file_get.toLatin1();
    strcpy(C_Packet.content , Target.data());
    QByteArray Location=My_File.toLatin1();
    C_Packet.cmd=UPLOAD;

    message_show->append("Upload......");
    message_show->moveCursor(message_show->textCursor().End);

    qDebug()<<Location.data();

    file=fopen(Location.data() , "rb");
    if(file==NULL)
    {
        qDebug()<<"Open File Error";
        return QString("Open File Error");
    }

    new_Soc=Create_Socket();
    Send_Order(My_Socket , &C_Packet);
    Read_Resp(My_Socket , &R_Packet);
    if(R_Packet.response==ERR)
    {
        qDebug()<<"Same File Has Existed";
        fclose(file);
        return QString("Same File Has Existed");
    }

    size =sizeof(SOCKADDR_IN);
    unsigned int mark_accept=(temp_Soc = accept(new_Soc , (SOCKADDR*) &addr , &size));
    if(mark_accept== INVALID_SOCKET)
    {
        qDebug()<<"upload error";
        closesocket(new_Soc);
        fclose(file);
        return QString("upload error");
    }

    num=0;
    while (true)
    {
        num=fread(buffer , sizeof(char) , DATA_BUFSIZE , file);
        send(temp_Soc , buffer , num , 0);
        if(num<DATA_BUFSIZE)
        {
            qDebug()<<"File Upload Successfully";
            break;
        }
    }
    closesocket(temp_Soc);
    closesocket(new_Soc);
    fclose(file);
    message_show->append("Upload Successfully");
    message_show->moveCursor(message_show->textCursor().End);
    return QString("File Upload Successfully");
}

void Widget::show_mainwin()
{
    this->show();
}

void Widget::UserExit()
{
    //清除全部内容
    message_show->clearHistory();
    closesocket(My_Socket);
    this->close();
    emit Exit();
}

void Widget::Construct()
{
    this->setWindowTitle("闪翼文件传输系统-客户端");
    this->resize(1200, 800);
    this->setWindowIcon(QIcon(":/logo/logo.png"));
    this->setFixedSize(this->width(), this->height());

    QImage  image;
    image.load(":/logo/widget_background.png");
    QPalette palet;
    palet.setBrush(this->backgroundRole(),QBrush(image));
    this->setPalette(palet);

    file_dir = new QPushButton(this); file_dir->resize(200,70); file_dir->move(960,180);
    QString styleSheetString2("QPushButton{border-image:url(\":/logo/filedir1.png\");}");
    styleSheetString2+="QPushButton:hover{border-image:url(\":/logo/filedir2.png\");}";
    styleSheetString2+="QPushButton:pressed{border-image:url(\":/logo/filedir3.png\");}";
    file_dir->setStyleSheet(styleSheetString2);
    connect(file_dir, &QPushButton::clicked, this, &Widget::FileDir);

    view_folder = new QPushButton(this); view_folder->resize(200,70); view_folder->move(960,280);
    QString styleSheetString1("QPushButton{border-image:url(\":/logo/viewfolder1.png\");}");
    styleSheetString1+="QPushButton:hover{border-image:url(\":/logo/viewfolder2.png\");}";
    styleSheetString1+="QPushButton:pressed{border-image:url(\":/logo/viewfolder3.png\");}";
    view_folder->setStyleSheet(styleSheetString1);
    connect(view_folder, &QPushButton::clicked, this, &Widget::ViewFolder);

    upload = new QPushButton(this); upload->resize(200,70); upload->move(960,430);
    QString styleSheetString3("QPushButton{border-image:url(\":/logo/upload1.png\");}");
    styleSheetString3+="QPushButton:hover{border-image:url(\":/logo/upload2.png\");}";
    styleSheetString3+="QPushButton:pressed{border-image:url(\":/logo/upload3.png\");}";
    upload->setStyleSheet(styleSheetString3);
    connect(upload, &QPushButton::clicked, this, &Widget::Upload);

    download = new QPushButton(this); download->resize(200,70); download->move(960,530);
    QString styleSheetString4("QPushButton{border-image:url(\":/logo/download1.png\");}");
    styleSheetString4+="QPushButton:hover{border-image:url(\":/logo/download2.png\");}";
    styleSheetString4+="QPushButton:pressed{border-image:url(\":/logo/download3.png\");}";
    download->setStyleSheet(styleSheetString4);
    connect(download, &QPushButton::clicked, this, &Widget::Download);

    userexit = new QPushButton(this); userexit->resize(80,80); userexit->move(1020,665);
    QString styleSheetString5("QPushButton{border-image:url(\":/logo/exit1.png\");}");
    styleSheetString5+="QPushButton:hover{border-image:url(\":/logo/exit2.png\");}";
    styleSheetString5+="QPushButton:pressed{border-image:url(\":/logo/exit3.png\");}";
    userexit->setStyleSheet(styleSheetString5);
    connect(userexit, &QPushButton::clicked, this, &Widget::UserExit);

    message_show = new QTextBrowser(this);
    message_show->resize(900, 700);
    message_show->move(50, 60);
    message_show->setFont(QFont("Consolas", 12, QFont::Bold));
}



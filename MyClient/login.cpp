#include "login.h"

Login::Login(QWidget *parent) : QDialog(parent)
{
    Construct();
    chances = 3;
}

//连接到serip:serport
SOCKET My_Socket;

void Login::ConnectSer()
{
    //从输入框中获取服务器地址和端口
    serip = my_severloc->text();
    serport = my_port->text();

    IP_NUM = serip;
    Port_NUM = serport;
    SOCKADDR_IN addr;
    if(WSAStartup(MAKEWORD(2, 2), &wasData) != 0)
    {
        qDebug()<<"WinSock初始化失败";
    }

    //创建套接口
    My_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(My_Socket == INVALID_SOCKET)
    {
        qDebug()<<"创建套接口失败";
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(Port_NUM.toInt());

    char* temp;
    QByteArray Tran = IP_NUM.toLatin1();
    temp = Tran.data();
    addr.sin_addr.S_un.S_addr = inet_addr(temp);
    memset(&(addr.sin_zero), 0, sizeof(addr.sin_zero));

    if (::connect(My_Socket, (SOCKADDR*)&addr, sizeof(SOCKADDR)) == SOCKET_ERROR)  //避免与QTconnect冲突
    {
        qDebug()<<"未能连接上服务器";
        closesocket(My_Socket);
        WSACleanup();
        return;
    }

    qDebug()<<"连接成功，可进行登录";
    cli_login->setEnabled(true);
    return;
}

void Login::Login_clicked()
{
    user = my_username->text();
    psd = my_password->text();

    if((user == Name) && (psd == PassWord))
    {
        this->close();
        emit mainshow();
    }
    else
    {
        //拥有三次机会输入
        chances--;
        if(chances >= 1)
        {
            switch (chances)
            {
            case 1:
                FailedLog.setText("用户名或密码错误，您还有1次输入机会！");
                break;
            case 2:
                FailedLog.setText("用户名或密码错误，您还有2次输入机会！");
                break;
            default:
                break;
            }
            FailedLog.show();
        }
        else
        {
            ExitLog.show();
        }
    }
}

void Login::show_loginwin()
{
    this->show();
    //清空文本框，重新连接服务器
    my_severloc->clear();
    my_port->clear();
    my_username->clear();
    my_password->clear();
    //设置光标
    my_severloc->setFocus();
    cli_login->setEnabled(false);
}

void Login::Construct()
{
    this->setWindowTitle("闪翼文件传输系统-客户端登录");
    this->resize(1200, 800);
    this->setWindowIcon(QIcon(":/logo/logo.png"));
    setFixedSize(this->width(), this->height());

    QImage  image;
    image.load(":/logo/login_background.png");
    QPalette palet;
    palet.setBrush(this->backgroundRole(),QBrush(image));
    this->setPalette(palet);

    //服务器地址输入布局
    my_severloc = new QLineEdit(this);
    my_severloc->setStyleSheet("QLineEdit{border-width:0;border-style:outset;background-color:rgba(242,242,242,0)}");
    my_severloc->resize(300, 45);
    my_severloc->move(250, 340);
    my_severloc->setFont(QFont("Consolas", 15, QFont::Bold));

    my_port = new QLineEdit(this);
    my_port->setStyleSheet("QLineEdit{border-width:0;border-style:outset;background-color:rgba(242,242,242,0)}");
    my_port->resize(300, 45);
    my_port->move(250, 435);
    my_port->setFont(QFont("Consolas", 15, QFont::Bold));

    //登录界面布局
    my_username = new QLineEdit(this);
    my_username->setStyleSheet("QLineEdit{border-width:0;border-style:outset;background-color:rgba(242,242,242,0)}");
    my_username->resize(300, 55);
    my_username->move(780, 316);
    my_username->setFont(QFont("Consolas", 15, QFont::Bold));

    my_password = new QLineEdit(this);
    my_password->setStyleSheet("QLineEdit{border-width:0;border-style:outset;background-color:rgba(242,242,242,0)}");
    my_password->resize(300, 55);
    my_password->move(780, 424);
    my_password->setFont(QFont("Consolas", 15, QFont::Bold));
    my_password->setEchoMode(QLineEdit::Password);

    //按钮设置
    connectser = new QPushButton(this); connectser->resize(220,60); connectser->move(295,515);
    QString styleSheetString1("QPushButton{border-image:url(\":/logo/connect1.png\");}");
    styleSheetString1+="QPushButton:hover{border-image:url(\":/logo/connect2.png\");}";
    styleSheetString1+="QPushButton:pressed{border-image:url(\":/logo/connect3.png\");}";
    connectser->setStyleSheet(styleSheetString1);
    connect(connectser, &QPushButton::clicked, this, &Login::ConnectSer);

    cli_login = new QPushButton(this); cli_login->resize(220,60); cli_login->move(800,515);
    QString styleSheetString2("QPushButton{border-image:url(\":/logo/login1.png\");}");
    styleSheetString2+="QPushButton:hover{border-image:url(\":/logo/login2.png\");}";
    styleSheetString2+="QPushButton:pressed{border-image:url(\":/logo/login3.png\");}";
    cli_login->setStyleSheet(styleSheetString2);
    connect(cli_login, &QPushButton::clicked, this, &Login::Login_clicked);
    cli_login->setEnabled(false);

    //设置重试对话框
    FailedLog.setWindowTitle("用户登录");
    FailedLog.setWindowIcon(QIcon(":/logo/logo.png"));
    FailedLog.addButton(Retry, QMessageBox::AcceptRole);
    connect(Retry, &QPushButton::released,
            [=]()
    {
        my_username->clear();
        my_password->clear();
    });

    //设置失败对话框
    ExitLog.setWindowTitle("用户登录");
    ExitLog.setWindowIcon(QIcon(":/logo/logo.png"));
    ExitLog.setText("您的机会已用完，请您退出登录。");
    ExitLog.addButton(ExitL, QMessageBox::RejectRole);
    connect(ExitL, &QPushButton::released,
            [=]()
    {
        closesocket(My_Socket);
        WSACleanup();
        close();
    });
}

#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    Construct();
    SerInit();
}

Widget::~Widget()
{
    ::closesocket(mSerSocket);
    WSACleanup();
}

//服务器端初始化
void Widget::SerInit()
{
    if(WSAStartup(MAKEWORD(2, 2), &wasData) != 0)
    {
        message->append("初始化失败");
        return;
    }
    return;
}

//启动服务器
void Widget::OpenSer()
{
    //先获取输入端口
    QString thePort;
    thePort = serport->text();
    this->port = thePort.toInt();
    message->append("服务器端口号为：" + thePort);

    //创建套接字
    SOCKET mListen;
    mListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(mListen ==INVALID_SOCKET)
    {
        message->append("创建socket失败");
        WSACleanup();
        return;
    }

    //将ser套接字与本地端口地址绑定
    SOCKADDR_IN ser;
    ser.sin_family = AF_INET;
    ser.sin_port = htons(port);
    ser.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    memset(&(ser.sin_zero), 0, sizeof(ser.sin_zero));
    if(bind(mListen, (SOCKADDR*)&ser, sizeof(ser)) != 0)
    {
        WSACleanup();
        qDebug()<<"绑定socket与本地端口失败";
        message->append("绑定socket与本地端口失败");
        return;
    }

    //进入监听状态
    if(listen(mListen, 5) != 0)
    {
        WSACleanup();
        qDebug()<<"监听错误";
        message->append("监听错误");
        return;
    }

    message->append("服务器打开成功");
    message->append("-------------------------服务器开始工作-------------------------");
    openser->setStyleSheet("QPushButton{border-image:url(\":/logo/openser3.png\");}");
    openser->setEnabled(false);
    mSerSocket = mListen;

    //启动后台线程与客户端进行连接
    mCliThread = new CliThread(mSerSocket);
    mCliThread->start();

    //信号槽连接
    connect(mCliThread, &CliThread::isMsg, this, &Widget::MsgUpdate);
}

//消息框内容更新
void Widget::MsgUpdate(QString msg)
{
    message->append(msg);
    return;
}

//Ser退出
void Widget::SerExit()
{
    mCliThread->terminate();
    mCliThread->quit();
    delete mCliThread;
    closesocket(mSerSocket);
    WSACleanup();
    close();
}

//服务器界面美化
void Widget::Construct()
{
    this->setWindowTitle("闪翼文件传输系统-服务器");
    this->resize(1200, 800);
    this->setWindowIcon(QIcon(":/logo/logo.png"));
    this->setFixedSize(this->width(), this->height());

    QImage  image;
    image.load(":/logo/ser_background.png");
    QPalette palet;
    palet.setBrush(this->backgroundRole(),QBrush(image));
    this->setPalette(palet);

    openser = new QPushButton(this); openser->resize(200,70); openser->move(920,400);
    QString styleSheetString1("QPushButton{border-image:url(\":/logo/openser1.png\");}");
    styleSheetString1+="QPushButton:hover{border-image:url(\":/logo/openser2.png\");}";
    styleSheetString1+="QPushButton:pressed{border-image:url(\":/logo/openser3.png\");}";
    openser->setStyleSheet(styleSheetString1);
    connect(openser, &QPushButton::clicked, this, &Widget::OpenSer);

    exit = new QPushButton(this); exit->resize(80,80); exit->move(980,665);
    QString styleSheetString2("QPushButton{border-image:url(\":/logo/exit1.png\");}");
    styleSheetString2+="QPushButton:hover{border-image:url(\":/logo/exit2.png\");}";
    styleSheetString2+="QPushButton:pressed{border-image:url(\":/logo/exit3.png\");}";
    exit->setStyleSheet(styleSheetString2);
    connect(exit, &QPushButton::clicked,
            [=]()
    {
        ExitLog.show();
    });

    serport = new QLineEdit(this);
    serport->setStyleSheet("QLineEdit{border-width:0;border-style:outset;background-color:rgba(242,242,242,0)}");
    serport->resize(245, 50);
    serport->move(900, 322);
    serport->setFont(QFont("Consolas", 15, QFont::Bold));

    message = new QTextBrowser(this);
    message->resize(820, 700);
    message->move(45, 60);
    message->setFont(QFont("Consolas", 12, QFont::Bold));

    //设置退出对话框
    ExitLog.setWindowTitle("闪翼");
    ExitLog.setWindowIcon(QIcon(":/logo/logo.png"));
    ExitLog.setText("感谢您选择闪翼文件传输，欢迎您下次使用，再见~");
    ExitLog.addButton(ExitL, QMessageBox::RejectRole);
    connect(ExitL, &QPushButton::released, this, &Widget::SerExit);
}

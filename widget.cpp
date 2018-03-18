#include "widget.h"
#include "ui_widget.h"
#include <QUdpSocket>
#include <QHostInfo>
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QNetworkInterface>
#include <QProcess>
#include "tcpserver.h"
#include "tcpclient.h"
#include <QFileDialog>
#include <QDebug>
#include "chat.h"
#include <QColorDialog>
#include "camera.h"
#include"screenshot.h"
#include<QApplication>
#include <QDesktopWidget>
#include "audiorecorder.h"
#include "qaudiolevel.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget){

    ui->setupUi(this);



    privatechat = NULL;
    bb = 0;


    //创建套接字并进行初始化
    udpSocket = new QUdpSocket(this);
    port = 45456;
    udpSocket->bind(port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);//QUdpSocket::ShareAddress 绑定模式  其他服务也可以绑定在其上
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
    sendMessage(NewParticipant);//调用sendMessage(NewParticipant)放udp广播

    server = new TcpServer(this);

    //关联信号 获取文件名 并调用sendmessage函数发送文件
    connect(server, SIGNAL(sendFileName(QString)), this, SLOT(getFileName(QString)));

    connect(ui->messageTextEdit_2, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
            this, SLOT(currentFormatChanged(const QTextCharFormat)));
}

Widget::~Widget()
{
    delete ui;
}

// 使用UDP广播发送信息 type用于区分数据类型
void Widget::sendMessage(MessageType type, QString serverAddress)
{

    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    QString localHostName = QHostInfo::localHostName();
    QString address = getIP();
    out << type << getUserName() << localHostName;

    switch(type)
    {
    case Message :
        if (ui->messageTextEdit_2->toPlainText() == "") {
            QMessageBox::warning(0,tr("Warning!"),tr("Cannot send nothing"),QMessageBox::Ok);
            return;
        }
        out << address << getMessage();
        ui->messageBrowser_2->verticalScrollBar()
                ->setValue(ui->messageBrowser_2->verticalScrollBar()->maximum());
        break;

    case NewParticipant :
        out << address;
        break;

    case ParticipantLeft :
        break;

    case FileName : {
        int row = ui->userTableWidget_2->currentRow();
        QString clientAddress = ui->userTableWidget_2->item(row, 2)->text();
        out << address << clientAddress << fileName;
        break;
    }

    case Refuse :
        out << serverAddress;
        break;
    }
    //调用udpsocket进行udp广播
    udpSocket->writeDatagram(data,data.length(),QHostAddress::Broadcast, port);
}


// 接收UDP广播进行处理
void Widget::processPendingDatagrams()
{
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());
        QDataStream in(&datagram, QIODevice::ReadOnly);
        int messageType1;
        in >> messageType1;
        QString userName,localHostName,ipAddress,message;
        QString time = QDateTime::currentDateTime()
                .toString("yyyy-MM-dd hh:mm:ss");


        switch(messageType1)
        {

        case Message1:
            in >> userName >> localHostName >> ipAddress >> message;
            ui->messageBrowser_2->setTextColor(Qt::blue);
            ui->messageBrowser_2->setCurrentFont(QFont("Times New Roman",12));
            ui->messageBrowser_2->append("[ " +userName+" ] "+ time);
            ui->messageBrowser_2->append(message);
            qDebug()<<"processPendingDatagrams()";
            break;

        case NewParticipant1:
            in >>userName >>localHostName >>ipAddress;
            newParticipant(userName,localHostName,ipAddress, 1);
            break;

        case ParticipantLeft1:
            in >>userName >>localHostName;
            participantLeft(userName,localHostName,time);
            break;

        case FileName1: {
            in >> userName >> localHostName >> ipAddress;
            QString clientAddress, fileName;
            in >> clientAddress >> fileName;
            hasPendingFile(userName, ipAddress, clientAddress, fileName);
            break;
        }

        case Refuse1: {
            in >> userName >> localHostName;
            QString serverAddress;
            in >> serverAddress;
            QString ipAddress = getIP();

            if(ipAddress == serverAddress)
            {
                server->refused();
            }
            break;

        }
        case Xchat1:
            {
                in >>userName >>localHostName >>ipAddress;
                showxchat(localHostName,ipAddress);//显示与主机名聊天中，不是用户名
                break;
            }
        }
    }
}


// 处理用户离开 仅仅在用户列表中删除掉该用户的信息
void Widget::participantLeft(QString userName, QString localHostName, QString time)
{
    int rowNum = ui->userTableWidget_2->findItems(localHostName, Qt::MatchExactly).first()->row();
    ui->userTableWidget_2->removeRow(rowNum);
    ui->messageBrowser_2->setTextColor(Qt::gray);
    ui->messageBrowser_2->setCurrentFont(QFont("Times New Roman", 10));
    ui->messageBrowser_2->append(tr("%1 at %2 leave!").arg(userName).arg(time));
    ui->userNumLabel_2->setText(tr("online now：%1").arg(ui->userTableWidget_2->rowCount()));
}

void Widget::on_userTableWidget_2_doubleClicked(const QModelIndex &index)
{
    if(ui->userTableWidget_2->item(index.row(),0)->text() == getUserName() &&
        ui->userTableWidget_2->item(index.row(),2)->text() == getIP())
    {
        QMessageBox::warning(0,"Warning","No one talks with himself!",QMessageBox::Ok);
    }
    else
    {
        if(!privatechat){

               privatechat = new chat(ui->userTableWidget_2->item(index.row(),1)->text(), //接收主机名
                                      ui->userTableWidget_2->item(index.row(),2)->text()) ;//接收用户IP
               }
       //		if( privatechat->is_opened )delete privatechat;//如果其曾经显示过则删除掉
               QByteArray data;
               QDataStream out(&data,QIODevice::WriteOnly);
               QString localHostName = QHostInfo::localHostName();
               QString address = getIP();
               out << Xchat << getUserName() << localHostName << address;
               //ui->userTableWidget_2->item(index.row(),2)->text()
               udpSocket->writeDatagram(data,data.length(),QHostAddress(ui->userTableWidget_2->item(index.row(),2)->text()), port);

       //		privatechat->xchat->writeDatagram(data,data.length(),QHostAddress::QHostAddress(ui->tableWidget->item(index.row(),2)->text()), 45456);
             //  if(!privatechat->is_opened)d
               privatechat->setWindowOpacity(1);
               privatechat->setWindowFlags(Qt::FramelessWindowHint);
               privatechat->setAttribute(Qt::WA_TranslucentBackground);
                   privatechat->show();


               privatechat->is_opened = true;
           //	(privatechat->a) = 0;
                           qDebug()<<"ye is gay";


    }

}

// 是否接收文件
void Widget::hasPendingFile(QString userName, QString serverAddress,
                            QString clientAddress, QString fileName)
{
    QString ipAddress = getIP();
    if(ipAddress == clientAddress)
    {
        int btn = QMessageBox::information(this,tr("Receive"),
                                           tr("File from%1(%2) Name:%3,Receive?")
                                           .arg(userName).arg(serverAddress).arg(fileName),
                                           QMessageBox::Yes,QMessageBox::No);
        if (btn == QMessageBox::Yes) {
            QString name = QFileDialog::getSaveFileName(0,tr("Save"),fileName);
            if(!name.isEmpty())
            {
                TcpClient *client = new TcpClient(this);
                client->setFileName(name);
                client->setHostAddress(QHostAddress(serverAddress));
                client->show();
            }
        } else {
            sendMessage(Refuse, serverAddress);
        }
    }
}

// 传输文件按钮
void Widget::on_sendToolBtn_2_clicked()
{
    if(ui->userTableWidget_2->selectedItems().isEmpty())
    {
        QMessageBox::warning(0, tr("Choose one"),
                             tr("Choose one from the list!"), QMessageBox::Ok);
        return;
    }
    server->show();
    server->initServer();
}


// 处理新用户加
void Widget::newParticipant(QString userName, QString localHostName, QString ipAddress, int Boo = 1)
{
        //去重处理
        bool isEmpty = ui->userTableWidget_2->findItems(localHostName, Qt::MatchExactly).isEmpty();
        if (isEmpty) {



        QTableWidgetItem *user = new QTableWidgetItem(userName);
        QTableWidgetItem *host = new QTableWidgetItem(localHostName);
        QTableWidgetItem *ip = new QTableWidgetItem(ipAddress);

        ui->userTableWidget_2->insertRow(0);
        ui->userTableWidget_2->setItem(0,0,user);
        ui->userTableWidget_2->setItem(0,1,host);
        ui->userTableWidget_2->setItem(0,2,ip);
        ui->messageBrowser_2->setTextColor(Qt::gray);
        ui->messageBrowser_2->setCurrentFont(QFont("Times New Roman",10));
        ui->messageBrowser_2->append(tr("%1 online!").arg(userName));
        ui->userNumLabel_2->setText(tr("current online:%1").arg(ui->userTableWidget_2->rowCount()));
        sendMessage(NewParticipant);
        qDebug()<<"Aasfa";

     //老用户向新用户发送自己的信息
    if(Boo==1)
    {
            QByteArray data;
            QDataStream out(&data, QIODevice::WriteOnly);
            QString localHostName = QHostInfo::localHostName();
            QString address = getIP();
            int o = 2;
            out << address << getUserName() << localHostName<<o;
            udpSocket->writeDatagram(data,data.length(),QHostAddress(ipAddress), port);

           /* QByteArray data;
            QDataStream out(&data,QIODevice::WriteOnly);
            QString localHostName = QHostInfo::localHostName();
            QString address = getIP();
            out << type << getUserName() << localHostName;
            xchat->writeDatagram(data,data.length(),QHostAddress(xpasvuserip), 80);*/
    }


       }
   }

// 获取ipv4地址
QString Widget::getIP()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list) {
        if(address.protocol() == QAbstractSocket::IPv4Protocol)
            return address.toString();
    }
    return 0;
}

// 获取用户名 使用QProcess 类进行获取
QString Widget::getUserName()
{
    QStringList envVariables;
    envVariables << "USERNAME.*" << "USER.*" << "USERDOMAIN.*"
                 << "HOSTNAME.*" << "DOMAINNAME.*";
    QStringList environment = QProcess::systemEnvironment();
    foreach (QString string, envVariables) {
        int index = environment.indexOf(QRegExp(string));
        if (index != -1) {
            QStringList stringList = environment.at(index).split('=');
            if (stringList.size() == 2) {
                return stringList.at(1);
                break;
            }
        }
    }
    return "unknown";
}

// 获得要发送的消息 并清空输入框
QString Widget::getMessage()
{
    QString msg = ui->messageTextEdit_2->toHtml();

    ui->messageTextEdit_2->clear();
    ui->messageTextEdit_2->setFocus();
    return msg;
}


// 发送消息
void Widget::on_sendButton_2_clicked()
{
    sendMessage(Message);
}



// 获取要发送的文件名
void Widget::getFileName(QString name)
{
    fileName = name;
    sendMessage(FileName);
}


// 更改字体族
void Widget::on_fontComboBox_currentFontChanged(QFont f)
{
    ui->messageTextEdit_2->setCurrentFont(f);
    ui->messageTextEdit_2->setFocus();
}


// 更改字体大小
void Widget::on_sizeComboBox_2_currentIndexChanged(QString size)
{
    ui->messageTextEdit_2->setFontPointSize(size.toDouble());
    ui->messageTextEdit_2->setFocus();
}

// 加粗
void Widget::on_boldToolBtn_2_clicked(bool checked)
{
    if(checked)
        ui->messageTextEdit_2->setFontWeight(QFont::Bold);
    else
        ui->messageTextEdit_2->setFontWeight(QFont::Normal);
    ui->messageTextEdit_2->setFocus();
}

// 倾斜
void Widget::on_italicToolBtn_2_clicked(bool checked)
{
    ui->messageTextEdit_2->setFontItalic(checked);
    ui->messageTextEdit_2->setFocus();
}

// 下划线
void Widget::on_underlineToolBtn_2_clicked(bool checked)
{
    ui->messageTextEdit_2->setFontUnderline(checked);
    ui->messageTextEdit_2->setFocus();
}

// 颜色
void Widget::on_colorToolBtn_2_clicked()
{
    color = QColorDialog::getColor(color, this);
    if (color.isValid()) {
        ui->messageTextEdit_2->setTextColor(color);
        ui->messageTextEdit_2->setFocus();
    }
}

void Widget::currentFormatChanged(const QTextCharFormat &format)
{
    ui->fontComboBox->setCurrentFont(format.font());

    // 如果字体大小出错(因为我们最小的字体为9)，使用12
    if (format.fontPointSize() < 9) {
        ui->sizeComboBox_2->setCurrentIndex(3);
    } else {
        ui->sizeComboBox_2->setCurrentIndex( ui->sizeComboBox_2
                                          ->findText(QString::number(format.fontPointSize())));
    }
    ui->boldToolBtn_2->setChecked(format.font().bold());
    ui->italicToolBtn_2->setChecked(format.font().italic());
    ui->underlineToolBtn_2->setChecked(format.font().underline());
    color = format.foreground().color();
}

// 保存聊天记录
void Widget::on_saveToolBtn_2_clicked()
{
    if (ui->messageBrowser_2->document()->isEmpty()) {
        QMessageBox::warning(0, tr("Warning"), tr("Nothing to save"), QMessageBox::Ok);
    } else {
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Save record"), tr("Record"), tr("textfile(*.txt);;All File(*.*)"));
        if(!fileName.isEmpty())
            saveFile(fileName);
    }
}

// 保存聊天记录
bool Widget::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Save"),
                             tr("Save failed %1:\n %2").arg(fileName)
                             .arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out << ui->messageBrowser_2->toPlainText();

    return true;
}

// 清空聊天记录
void Widget::on_clearToolBtn_2_clicked()
{
    ui->messageBrowser_2->clear();
}

// 退出按钮
void Widget::on_exitButton_2_clicked()
{
    close();
}

// 关闭事件
void Widget::closeEvent(QCloseEvent *e)
{
    sendMessage(ParticipantLeft);

}

QString Widget::getUName()
{
    return getUserName();
}

void Widget::on_pushButton_3_clicked(){}

void Widget::showxchat(QString name, QString ip)
{
//	if(!privatechat){
 // chat *privatechatTemp;
    if(!privatechat1)
    privatechat1 = new chat(name,ip);
//	privatechat = privatechatTemp;}
//	chat privatechat(name,ip);//如果不用new函数，则程序运行时只是闪烁显示一下就没了，因为类的生命周期结束了
//	privatechat->is_opened = false;
 // privatechat->show();
  //privatechat.textBrowser.show();
  //privatechat->is_opened = true;
    bb++;
    //delete privatechat;

}

void Widget::on_pushButton_4_clicked()
{

    Camera *C = new Camera;
    C->show();
}

void Widget::on_pushButton_clicked()
{

    Screenshot* S = new Screenshot;
    S->move(QApplication::desktop()->availableGeometry(S).topLeft() + QPoint(20, 20));
    S->show();
}

void Widget::on_pushButton_2_clicked()
{
    AudioRecorder* recorder = new AudioRecorder;
    recorder->show();
}


void Widget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


bool Widget::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->messageTextEdit_2)
    {
        if(event->type() == QEvent::KeyPress)//回车键
        {
             QKeyEvent *k = static_cast<QKeyEvent *>(event);
             if(k->key() == Qt::Key_Return)
             {
                 on_sendButton_2_clicked();
                 return true;
             }
        }
    }
    return QWidget::eventFilter(target,event);
}

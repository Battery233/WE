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


    //�����׽��ֲ����г�ʼ��
    udpSocket = new QUdpSocket(this);
    port = 45456;
    udpSocket->bind(port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);//QUdpSocket::ShareAddress ��ģʽ  ��������Ҳ���԰�������
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
    sendMessage(NewParticipant);//����sendMessage(NewParticipant)��udp�㲥

    server = new TcpServer(this);

    //�����ź� ��ȡ�ļ��� ������sendmessage���������ļ�
    connect(server, SIGNAL(sendFileName(QString)), this, SLOT(getFileName(QString)));

    connect(ui->messageTextEdit_2, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
            this, SLOT(currentFormatChanged(const QTextCharFormat)));
}

Widget::~Widget()
{
    delete ui;
}

// ʹ��UDP�㲥������Ϣ type����������������
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
    //����udpsocket����udp�㲥
    udpSocket->writeDatagram(data,data.length(),QHostAddress::Broadcast, port);
}


// ����UDP�㲥���д���
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
                showxchat(localHostName,ipAddress);//��ʾ�������������У������û���
                break;
            }
        }
    }
}


// �����û��뿪 �������û��б���ɾ�������û�����Ϣ
void Widget::participantLeft(QString userName, QString localHostName, QString time)
{
    int rowNum = ui->userTableWidget_2->findItems(localHostName, Qt::MatchExactly).first()->row();
    ui->userTableWidget_2->removeRow(rowNum);
    ui->messageBrowser_2->setTextColor(Qt::gray);
    ui->messageBrowser_2->setCurrentFont(QFont("Times New Roman", 10));
    ui->messageBrowser_2->append(tr("%1 at %2 leave!").arg(userName).arg(time));
    ui->userNumLabel_2->setText(tr("online now��%1").arg(ui->userTableWidget_2->rowCount()));
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

               privatechat = new chat(ui->userTableWidget_2->item(index.row(),1)->text(), //����������
                                      ui->userTableWidget_2->item(index.row(),2)->text()) ;//�����û�IP
               }
       //		if( privatechat->is_opened )delete privatechat;//�����������ʾ����ɾ����
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

// �Ƿ�����ļ�
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

// �����ļ���ť
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


// �������û���
void Widget::newParticipant(QString userName, QString localHostName, QString ipAddress, int Boo = 1)
{
        //ȥ�ش���
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

     //���û������û������Լ�����Ϣ
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

// ��ȡipv4��ַ
QString Widget::getIP()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list) {
        if(address.protocol() == QAbstractSocket::IPv4Protocol)
            return address.toString();
    }
    return 0;
}

// ��ȡ�û��� ʹ��QProcess ����л�ȡ
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

// ���Ҫ���͵���Ϣ ����������
QString Widget::getMessage()
{
    QString msg = ui->messageTextEdit_2->toHtml();

    ui->messageTextEdit_2->clear();
    ui->messageTextEdit_2->setFocus();
    return msg;
}


// ������Ϣ
void Widget::on_sendButton_2_clicked()
{
    sendMessage(Message);
}



// ��ȡҪ���͵��ļ���
void Widget::getFileName(QString name)
{
    fileName = name;
    sendMessage(FileName);
}


// ����������
void Widget::on_fontComboBox_currentFontChanged(QFont f)
{
    ui->messageTextEdit_2->setCurrentFont(f);
    ui->messageTextEdit_2->setFocus();
}


// ���������С
void Widget::on_sizeComboBox_2_currentIndexChanged(QString size)
{
    ui->messageTextEdit_2->setFontPointSize(size.toDouble());
    ui->messageTextEdit_2->setFocus();
}

// �Ӵ�
void Widget::on_boldToolBtn_2_clicked(bool checked)
{
    if(checked)
        ui->messageTextEdit_2->setFontWeight(QFont::Bold);
    else
        ui->messageTextEdit_2->setFontWeight(QFont::Normal);
    ui->messageTextEdit_2->setFocus();
}

// ��б
void Widget::on_italicToolBtn_2_clicked(bool checked)
{
    ui->messageTextEdit_2->setFontItalic(checked);
    ui->messageTextEdit_2->setFocus();
}

// �»���
void Widget::on_underlineToolBtn_2_clicked(bool checked)
{
    ui->messageTextEdit_2->setFontUnderline(checked);
    ui->messageTextEdit_2->setFocus();
}

// ��ɫ
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

    // ��������С����(��Ϊ������С������Ϊ9)��ʹ��12
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

// ���������¼
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

// ���������¼
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

// ��������¼
void Widget::on_clearToolBtn_2_clicked()
{
    ui->messageBrowser_2->clear();
}

// �˳���ť
void Widget::on_exitButton_2_clicked()
{
    close();
}

// �ر��¼�
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
//	chat privatechat(name,ip);//�������new���������������ʱֻ����˸��ʾһ�¾�û�ˣ���Ϊ����������ڽ�����
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
        if(event->type() == QEvent::KeyPress)//�س���
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

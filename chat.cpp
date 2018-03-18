#include "chat.h"
#include "camera.h"
#include "ui_chat.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTcpServer>
#include <QColorDialog>
#include"screenshot.h"
#include<QApplication>
#include <QDesktopWidget>
#include "audiorecorder.h"
#include "qaudiolevel.h"

//chat::chat():ui(new Ui::chat)
//{
//	is_opened = false;
//}


chat::chat(QString pasvusername, QString pasvuserip) : ui(new Ui::chat)
{
    ui->setupUi(this);
    ui->textEdit->setFocusPolicy(Qt::StrongFocus);
    ui->textBrowser->setFocusPolicy(Qt::NoFocus);

    ui->textEdit->setFocus();
    ui->textEdit->installEventFilter(this);

    a = 0;
    is_opened = false;
    xpasvusername = pasvusername;
    xpasvuserip = pasvuserip;

    ui->label->setText(tr("with user:%2").arg(xpasvusername).arg(pasvuserip));

    //UDP����
    xchat = new QUdpSocket(this);
    xport = 46468;
    xchat->bind( QHostAddress(getIP()), xport );
    connect(xchat, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));

    //TCP����
    server = new TcpServer(this);
    connect(server,SIGNAL(sendFileName(QString)),this,SLOT(sentFileName(QString)));

    connect(ui->textEdit,SIGNAL(currentCharFormatChanged(QTextCharFormat)),this,SLOT(currentFormatChanged(const QTextCharFormat)));
}

chat::~chat()
{
    is_opened = false;
    delete ui;
}

bool chat::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->textEdit)
    {
        if(event->type() == QEvent::KeyPress)//���¼���ĳ��
        {
             QKeyEvent *k = static_cast<QKeyEvent *>(event);
             if(k->key() == Qt::Key_Return)//�س���
             {
                 on_send_clicked();
                 return true;
             }
        }
    }
    return QWidget::eventFilter(target,event);
}

//�����û��뿪
void chat::participantLeft(QString userName,QString localHostName,QString time)
{
    ui->textBrowser->setTextColor(Qt::gray);
    ui->textBrowser->setCurrentFont(QFont("Times New Roman",10));
    ui->textBrowser->append(tr("%1 left at%2").arg(userName).arg(time));
}

QString chat::getUserName()  //��ȡ�û���
{
    QStringList envVariables;
    envVariables << "USERNAME.*" << "USER.*" << "USERDOMAIN.*"
                 << "HOSTNAME.*" << "DOMAINNAME.*";
    QStringList environment = QProcess::systemEnvironment();
    foreach (QString string, envVariables)
    {
        int index = environment.indexOf(QRegExp(string));
        if (index != -1)
        {

            QStringList stringList = environment.at(index).split('=');
            if (stringList.size() == 2)
            {
                return stringList.at(1);
                break;
            }
        }
    }
    return false;
}

QString chat::getIP()  //��ȡip��ַ
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
       if(address.protocol() == QAbstractSocket::IPv4Protocol) //����ʹ��IPv4��ַ
            return address.toString();
    }
       return 0;
}

void chat::hasPendingFile(QString userName,QString serverAddress,  //�����ļ�
                            QString clientAddress,QString fileName)
{
    QString ipAddress = getIP();
    if(ipAddress == clientAddress)
    {
        int btn = QMessageBox::information(this,tr("recieve file"),
                                 tr("form%1(%2)'s file%3,recieve?")
                                 .arg(userName).arg(serverAddress).arg(fileName),
                                 QMessageBox::Yes,QMessageBox::No);
        if(btn == QMessageBox::Yes)
        {
            QString name = QFileDialog::getSaveFileName(0,tr("save file"),fileName);
            if(!name.isEmpty())
            {
                TcpClient *client = new TcpClient(this);
                client->setFileName(name);
                client->setHostAddress(QHostAddress(serverAddress));
                client->show();

            }

        }
        else{
            sendMessage(Refuse,serverAddress);
        }
    }
}

void chat::processPendingDatagrams()   //��������UDP
{
    while(xchat->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(xchat->pendingDatagramSize());
        xchat->readDatagram(datagram.data(),datagram.size());
        QDataStream in(&datagram,QIODevice::ReadOnly);
        int messageType1;
        in >> messageType1;
        QString userName,localHostName,ipAddress,messagestr;
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        switch(messageType1)
        {

            case Message:
                {
                    in >>userName >>localHostName >>ipAddress >>messagestr;
                    ui->textBrowser->setTextColor(Qt::blue);
                    ui->textBrowser->setCurrentFont(QFont("Times New Roman",12));
                    ui->textBrowser->append("[ " +localHostName+" ] "+ time);//��������������
                    ui->textBrowser->append(messagestr);

                    {
                        this->show();////���bug1.�յ�˽����Ϣ�����ʾ

                        is_opened = true;
                    }
                    break;
                }
        case Xchat:
        {

            this->show();
            break;
        }
        case FileName:
            {
                in >>userName >>localHostName >> ipAddress;
                QString clientAddress,fileName;
                in >> clientAddress >> fileName;
                hasPendingFile(userName,ipAddress,clientAddress,fileName);
                break;
            }
        case Refuse:
            {
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
        case ParticipantLeft:
            {
                in >>userName >>localHostName;
                participantLeft(userName,localHostName,time);
                QMessageBox::information(0,tr("shut down current"),tr("OT shut down current "),QMessageBox::Ok);
                a = 1;
                ui->textBrowser->clear();
                ui->~chat();
                close();
                break;
            }
        }
    }
}

void chat::sentFileName(QString fileName)
{
    this->fileName = fileName;
    sendMessage(FileName);
}

QString chat::getMessage()  //���Ҫ���͵���Ϣ
{
    QString msg = ui->textEdit->toHtml();
    qDebug()<<msg;
    ui->textEdit->clear();
    ui->textEdit->setFocus();
    return msg;
}

//ͨ��˽���׽��ַ��͵��Է���˽��ר�ö˿���
void chat::sendMessage(MessageType type , QString serverAddress)  //������Ϣ
{
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    QString localHostName = QHostInfo::localHostName();
    QString address = getIP();
    out << type << getUserName() << localHostName;


    switch(type)
    {
    case ParticipantLeft:
        {
            break;
        }
    case Message :
        {
            if(ui->textEdit->toPlainText() == "")
            {
                QMessageBox::warning(0,"warning","can't send empty",QMessageBox::Ok);
                return;
            }
            message = getMessage();
            out << address << message;
            //ui->textBrowser->verticalScrollBar()->setValue(ui->textBrowser->verticalScrollBar()->maximum());
            break;
        }
    case FileName:
            {
                QString clientAddress = xpasvuserip;
                out << address << clientAddress << fileName;
                break;
            }
    case Refuse:
            {
                out << serverAddress;
                break;
            }
    }
    xchat->writeDatagram(data,data.length(),QHostAddress(xpasvuserip), 46468);
    qDebug()<<xpasvuserip;
    qDebug()<<QHostAddress(xpasvuserip);

}

void chat::currentFormatChanged(const QTextCharFormat &format)
{//���༭���������ʽ�ı�ʱ�������ò���״̬Ҳ��֮�ı�
    ui->fontComboBox->setCurrentFont(format.font());

    if(format.fontPointSize()<9)  //��������С������Ϊ������С������Ϊ9
    {
        ui->fontsizecomboBox->setCurrentIndex(3); //����ʾ12
    }
    else
    {
        ui->fontsizecomboBox->setCurrentIndex(ui->fontsizecomboBox->findText(QString::number(format.fontPointSize())));

    }

    ui->textbold->setChecked(format.font().bold());
    ui->textitalic->setChecked(format.font().italic());
    ui->textUnderline->setChecked(format.font().underline());
    color = format.foreground().color();
}

void chat::on_fontComboBox_currentFontChanged(QFont f)//��������
{
    ui->textEdit->setCurrentFont(f);
    ui->textEdit->setFocus();
}

void chat::on_fontsizecomboBox_currentIndexChanged(QString size)
{
   ui->textEdit->setFontPointSize(size.toDouble());
   ui->textEdit->setFocus();
}

void chat::on_textbold_clicked(bool checked)
{
    if(checked)
        ui->textEdit->setFontWeight(QFont::Bold);
    else
        ui->textEdit->setFontWeight(QFont::Normal);
    ui->textEdit->setFocus();
}

void chat::on_textitalic_clicked(bool checked)
{
    ui->textEdit->setFontItalic(checked);
    ui->textEdit->setFocus();
}

void chat::on_save_clicked()//���������¼
{
    if(ui->textBrowser->document()->isEmpty())
        QMessageBox::warning(0,tr("warning"),tr("list is empty, can't save"),QMessageBox::Ok);
    else
    {
       //����ļ���
       QString fileName = QFileDialog::getSaveFileName(this,tr("save list"),tr("list"),tr("�ı�(*.txt);;All File(*.*)"));
       if(!fileName.isEmpty())
           saveFile(fileName);
    }
}

void chat::on_clear_clicked()//��������¼
{
    ui->textBrowser->clear();
}

bool chat::saveFile(const QString &fileName)//�����ļ�
{
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly | QFile::Text))

    {
        QMessageBox::warning(this,tr("save file"),
        tr("can't save file %1:\n %2").arg(fileName)
        .arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out << ui->textBrowser->toPlainText();

    return true;
}

void chat::on_textUnderline_clicked(bool checked)
{
    ui->textEdit->setFontUnderline(checked);
    ui->textEdit->setFocus();
}

void chat::on_textcolor_clicked()
{
    color = QColorDialog::getColor(color,this);
    if(color.isValid())
    {
        ui->textEdit->setTextColor(color);
        ui->textEdit->setFocus();
    }
}



void chat::on_close_clicked()
{
    sendMessage(ParticipantLeft);
    a = 1;
    ui->textBrowser->clear();
    //is_opened = true;
//	this->is_opened = false;
    close();
    ui->~chat();

    //this->close();
    /*delete ui;
    ui = 0;*/

}

void chat::on_send_clicked()
{
    sendMessage(Message);
    QString localHostName = QHostInfo::localHostName();
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->textBrowser->setTextColor(Qt::blue);
    ui->textBrowser->setCurrentFont(QFont("Times New Roman",12));
    ui->textBrowser->append("[ " +localHostName+" ] "+ time);
    ui->textBrowser->append(message);
//	is_opened = true;
}

void chat::on_sendfile_clicked()
{
    server->show();
    server->initServer();
}

void chat::on_pushButton_clicked()
{
    Camera camera;
    camera.show();
}

void chat::on_pushButton_2_clicked()
{
    Screenshot* S = new Screenshot;
    S->move(QApplication::desktop()->availableGeometry(S).topLeft() + QPoint(20, 20));
    S->show();
}

void chat::on_voiceButton_clicked()
{
        AudioRecorder* recorder = new AudioRecorder;
        recorder->show();

}

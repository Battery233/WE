#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtNetwork>
#include <QtGui>
#include "tcpclient.h"
#include "tcpserver.h"
#include "chat.h"
#include <QTextCharFormat>
class QUdpSocket;

class TcpServer;



namespace Ui {
class Widget;
}

enum MessageType1
{

    Message1,
    NewParticipant1,
    ParticipantLeft1,
    FileName1,
    Refuse1,
    Xchat1
};



class Widget : public QWidget
{
    Q_OBJECT

public:

    /**����˽�Ĺ���*/
    explicit Widget(QWidget *parent = 0);

    ~Widget();
    QString getUName();
    QString getIP();
    chat* privatechat;
    chat* privatechat1;

    QString getUserName();
    QString getMessage();



protected:
    void changeEvent(QEvent *e);
    void newParticipant(QString userName,
                        QString localHostName, QString ipAddress, int Boo);
    void participantLeft(QString userName,
                         QString localHostName, QString time);
    void sendMessage(MessageType type, QString serverAddress="");



    void hasPendingFile(QString userName, QString serverAddress,
                        QString clientAddress, QString fileName);


    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject *target, QEvent *event);//�¼�������

private:
    Ui::Widget *ui;
    QUdpSocket *udpSocket;
    qint16 port;
    qint32 bb;

    QString fileName;
    TcpServer *server;

    QColor color;
    bool saveFile(const QString& fileName);//����������¼
    void showxchat(QString name, QString ip);

private slots:

    void on_fontComboBox_currentFontChanged(QFont f);
    void on_sizeComboBox_2_currentIndexChanged(QString );
    void processPendingDatagrams();
    void on_sendButton_2_clicked();
    void on_clearToolBtn_2_clicked();
    void on_saveToolBtn_2_clicked();
    void on_colorToolBtn_2_clicked();
    void on_exitButton_2_clicked();
    void on_italicToolBtn_2_clicked(bool checked);
    void on_boldToolBtn_2_clicked(bool checked);
    void on_sendToolBtn_2_clicked();
    void getFileName(QString);


    void on_underlineToolBtn_2_clicked(bool checked);


    void currentFormatChanged(const QTextCharFormat &format);




    void on_userTableWidget_2_doubleClicked(const QModelIndex &index);
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
};

#endif // WIDGET_H

#include "welcome.h"
#include "ui_welcome.h"
#include "bmw.h"
#include <QDateTime>
#include "widget.h"
#include <QWidget>
#include <Qtgui>
#include <QMovie>
#include <QHostInfo>
#include <QNetworkInterface>
#include <widget.h>
welcome::welcome(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::welcome)
{
    ui->setupUi(this);
    ui->label->setText(tr("programing is loading "));
    myBMW *bmw = new myBMW(this);
     QMovie *moive = new QMovie(":/images/nofirstgif.gif");
    bmw->resize(50,50);
    bmw->setGeometry(550,1,50,50);
    ui->label_2->setText("当前时间 "+gettime());
    ui->pushButton->setStyleSheet("QPushButton{border-image: url(:/images/btnnormal.png);}"
                                  "QPushButton:hover{border-image: url(:/images/btnressed.png);}"
                                  "QPushButton:pressed{border-image: url(:/images/btnressed.png);}");

    ui->label_4->setMovie(moive);
    Widget xxa;
    ui->label_3->setText("Welcome:"+xxa.getUName());
    moive->start();
    ui->label_4->show();
}

welcome::~welcome()
{
    delete ui;
}

QString welcome::gettime(){

    QDateTime dt;
    QTime time;
    QDate date;
    dt.setTime(time.currentTime());
    dt.setDate(date.currentDate());
    QString currentDate = dt.toString("hh : mm");
   return currentDate;
}

void welcome::on_pushButton_clicked()
{

    Widget *w = new Widget;
    close();
    w->setWindowOpacity(1);
    w->setWindowFlags(Qt::FramelessWindowHint);
    w->setAttribute(Qt::WA_TranslucentBackground);
    w->show();
}




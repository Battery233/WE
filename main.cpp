#include <QtGui>
#include <QApplication>
#include "widget.h"
#include <QDialog>
#include <QDebug>
#include "welcome.h"
#include <QElapsedTimer>
int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    welcome app;
    app.setWindowTitle("ImClient");
    app.setWindowOpacity(1);
    app.setWindowFlags(Qt::FramelessWindowHint);
    app.show();

    int delaytime = 3;
    QElapsedTimer timer;
    timer.start();
    while(timer.elapsed()<(delaytime*1000)){
        a.processEvents();
    }
    //app.close();

    return a.exec();
}

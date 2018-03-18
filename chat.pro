#-------------------------------------------------
#
# Project created by QtCreator 2016-7-1T19:53:56
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT+=widgets
QT += network
TARGET = chat
TEMPLATE = app
QT += multimedia multimediawidgets

SOURCES += main.cpp\
        widget.cpp \
    tcpserver.cpp \
    tcpclient.cpp \
    welcome.cpp \
    bmw.cpp \
    chat.cpp \
    camera.cpp \
    imagesettings.cpp \
    videosettings.cpp \
    screenshot.cpp \
    audiorecorder.cpp \
    qaudiolevel.cpp

HEADERS  += widget.h \
    tcpserver.h \
    tcpclient.h \
    welcome.h \
    bmw.h \
    chat.h \
    camera.h \
    imagesettings.h \
    videosettings.h \
    screenshot.h \
    qaudiolevel.h \
    audiorecorder.h

FORMS    += widget.ui \
    tcpserver.ui \
    tcpclient.ui \
    welcome.ui \
    chat.ui \
    videosettings.ui \
    imagesettings.ui \
    camera.ui \
    audiorecorder.ui
QT += multimedia

win32:INCLUDEPATH += $$PWD
RESOURCES += \
    images.qrc
CONFIG += qtestlib\

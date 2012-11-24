#-------------------------------------------------
#
# Project created by QtCreator 2012-11-21T12:18:47
#
#-------------------------------------------------

QT       += core gui

TARGET = QtMedia
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    videowidget.cpp \
    imagesequence.cpp \
    mediabuffer.cpp \
    vlcbackend.cpp \
    avbinbackend.cpp

HEADERS  += mainwindow.h \
    videowidget.h \
    imagesequence.h \
    mediabuffer.h \
    vlcbackend.h \
    avbinbackend.h

FORMS    += mainwindow.ui \
    videowidget.ui

OTHER_FILES +=

RESOURCES +=

unix|win32: LIBS += -lvlc

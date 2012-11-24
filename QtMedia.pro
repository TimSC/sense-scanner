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
    avbinbackend.cpp \
    avbinmedia.cpp

HEADERS  += mainwindow.h \
    videowidget.h \
    imagesequence.h \
    mediabuffer.h \
    avbinbackend.h \
    avbinmedia.h

FORMS    += mainwindow.ui \
    videowidget.ui

OTHER_FILES +=

RESOURCES +=

#unix|win32: LIBS += -lvlc
unix|win32: LIBS += -lavbin
unix|win32: LIBS += -lpthread

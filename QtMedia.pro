#-------------------------------------------------
#
# Project created by QtCreator 2012-11-21T12:18:47
#
#-------------------------------------------------

QT       += core gui

TARGET = QtMedia
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++0x

SOURCES += main.cpp\
        mainwindow.cpp \
    videowidget.cpp \
    imagesequence.cpp \
    mediabuffer.cpp \
    avbinbackend.cpp \
    avbinmedia.cpp \
    eventloop.cpp \
    avbinapi.cpp \
    scenecontroller.cpp

HEADERS  += mainwindow.h \
    videowidget.h \
    imagesequence.h \
    mediabuffer.h \
    avbinbackend.h \
    avbinmedia.h \
    eventloop.h \
    avbinapi.h \
    scenecontroller.h \
    vectors.h

FORMS    += mainwindow.ui \
    videowidget.ui

OTHER_FILES +=

RESOURCES +=

#unix|win32: LIBS += -lvlc
unix|win32: LIBS += -lavbin
unix|win32: LIBS += -lpthread

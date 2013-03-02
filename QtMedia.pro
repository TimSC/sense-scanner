#-------------------------------------------------
#
# Project created by QtCreator 2012-11-21T12:18:47
#
#-------------------------------------------------

QT       += core gui
QT       += xml webkit
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
    scenecontroller.cpp \
    workspace.cpp \
    algorithm.cpp \
    annotation.cpp \
    qblowfish/src/qblowfish.cpp \
    sourcealggui.cpp \
    aboutgui.cpp \
    useractions.cpp \
    applymodel.cpp

HEADERS  += mainwindow.h \
    videowidget.h \
    imagesequence.h \
    mediabuffer.h \
    avbinbackend.h \
    avbinmedia.h \
    eventloop.h \
    avbinapi.h \
    scenecontroller.h \
    vectors.h \
    workspace.h \
    algorithm.h \
    localints.h \
    localmutex.h \
    localsleep.h \
    annotation.h \
    sourcealggui.h \
    aboutgui.h \
    useractions.h \
    applymodel.h

FORMS    += mainwindow.ui \
    videowidget.ui \
    sourcealggui.ui \
    aboutgui.ui

OTHER_FILES +=

RESOURCES +=

#unix|win32: LIBS += -lvlc
unix|win32: LIBS += -lavbin
unix|win32: LIBS += -lpthread

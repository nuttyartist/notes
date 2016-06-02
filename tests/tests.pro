#-------------------------------------------------
#
# Project created by QtCreator 2016-04-26T19:28:36
#
#-------------------------------------------------

QT       += widgets testlib network

TARGET    = test
CONFIG   += testcase
CONFIG   -= app_bundle

TEMPLATE = app

unix:!mac{
LIBS += -lX11
}

DEPENDPATH += ../src/OBJ

HEADERS += \
    tst_mainwindow.h \
    tst_notedata.h \
    tst_notemodel.h \
    tst_noteview.h

SOURCES += \
    main.cpp \
    tst_notedata.cpp \
    tst_mainwindow.cpp \
    tst_notemodel.cpp \
    tst_noteview.cpp

DEFINES += SRCDIR=\\\"$$PWD\\\"

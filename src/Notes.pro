#-------------------------------------------------
#
# Project created by QtCreator 2014-08-08T10:38:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Notes
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    notedata.cpp \
    notewidgetdelegate.cpp \
    notemodel.cpp \
    noteview.cpp

HEADERS  += mainwindow.h \
    notedata.h \
    notewidgetdelegate.h \
    notemodel.h \
    noteview.h

FORMS    += mainwindow.ui

RESOURCES += \
    images.qrc

CONFIG   += c++11

DESTDIR = ../bin

win32:RC_FILE = images\notes.rc

macx:ICON = images\notes_icon.icns

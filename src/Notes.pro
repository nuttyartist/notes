#-------------------------------------------------
#
# Project created by QtCreator 2014-08-08T10:38:29
#
#-------------------------------------------------

VERSION = 0.9.0

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Notes
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    notedata.cpp \
    notewidgetdelegate.cpp \
    notemodel.cpp \
    noteview.cpp \
    singleinstance.cpp

HEADERS  += mainwindow.h \
    notedata.h \
    notewidgetdelegate.h \
    notemodel.h \
    noteview.h \
    singleinstance.h

FORMS    += mainwindow.ui

RESOURCES += \
    images.qrc

CONFIG   += c++11 debug

DESTDIR = ../bin

unix {
    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    BINDIR = $$PREFIX/bin
    DATADIR =$$PREFIX/share

    target.path = $$BINDIR
    INSTALLS += target

    GIT_REV = $$system(git rev-parse --short HEAD)
    SNAPDIR = $$PWD/../packaging/linux/snap

    snap_bump_version.commands = \
        sed -i \"s/\\(^version:\\).*$$/\\1 $$VERSION~git$$GIT_REV/1\" $$SNAPDIR/snapcraft.yaml

    snap.commands = cd $$SNAPDIR && \
        snapcraft clean && snapcraft
    snap.depends = snap_bump_version

    QMAKE_EXTRA_TARGETS   += \
        snap \
        snap_bump_version
}

win32:RC_FILE = images\notes.rc

macx:ICON = images\notes_icon.icns

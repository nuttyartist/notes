#-------------------------------------------------
#
# Project created by QtCreator 2014-08-08T10:38:29
#
#-------------------------------------------------

QT       += core gui network
QT       += gui-private

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET    = Notes
TEMPLATE  = app
CONFIG   += c++11

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    notedata.cpp \
    notewidgetdelegate.cpp \
    notemodel.cpp \
    noteview.cpp \
    singleinstance.cpp \
    ../3rdParty/qxt/qxtglobalshortcut.cpp \
    ../3rdParty/qxt/qxtglobal.cpp

HEADERS  += \
    mainwindow.h \
    notedata.h \
    notewidgetdelegate.h \
    notemodel.h \
    noteview.h \
    singleinstance.h \
    ../3rdParty/qxt/qxtglobalshortcut_p.h \
    ../3rdParty/qxt/qxtglobalshortcut.h \
    ../3rdParty/qxt/qxtglobal.h

FORMS    += mainwindow.ui

INCLUDEPATH += ../3rdParty/qxt

RESOURCES += \
    images.qrc

unix:!macx {
    SOURCES += ../3rdParty/qxt/qxtglobalshortcut_x11.cpp
    LIBS += -lX11

    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    BINDIR = $$PREFIX/bin
    DATADIR =$$PREFIX/share

    target.path = $$BINDIR
    INSTALLS += target
}

macx{
    SOURCES += ../3rdParty/qxt/qxtglobalshortcut_mac.cpp
    DESTDIR = ../bin
    ICON = images\notes_icon.icns
}

win32 {
    SOURCES += ../3rdParty/qxt/qxtglobalshortcut_win.cpp
    DESTDIR = ../bin
    RC_FILE = images\notes.rc
    DEFINES += QXT_STATIC
}

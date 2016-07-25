#-------------------------------------------------
#
# Project created by QtCreator 2014-08-08T10:38:29
#
#-------------------------------------------------

QT += core gui network sql
QT += gui-private

TARGET    = Notes
TEMPLATE  = app
CONFIG   += c++11

UI_DIR = uic
MOC_DIR = moc
RCC_DIR = qrc
OBJECTS_DIR = obj

greaterThan (QT_MAJOR_VERSION, 4): QT += widgets

include ($$PWD/../3rdParty/qxt/qxt.pri)
include ($$PWD/../3rdParty/QSimpleUpdater/QSimpleUpdater.pri)

SOURCES += \
    $$PWD/main.cpp\
    $$PWD/mainwindow.cpp \
    $$PWD/notedata.cpp \
    $$PWD/notewidgetdelegate.cpp \
    $$PWD/notemodel.cpp \
    $$PWD/noteview.cpp \
    $$PWD/singleinstance.cpp \
    dbmanager.cpp

HEADERS  += \
    $$PWD/mainwindow.h \
    $$PWD/notedata.h \
    $$PWD/notewidgetdelegate.h \
    $$PWD/notemodel.h \
    $$PWD/noteview.h \
    $$PWD/singleinstance.h \
    dbmanager.h

FORMS += $$PWD/mainwindow.ui
RESOURCES += $$PWD/images.qrc

linux:!android {
    isEmpty (PREFIX) {
        PREFIX = /usr
    }

    BINDIR  = $$PREFIX/bin

    target.path = $$BINDIR
    icon.path =  $$PREFIX/share/pixmaps
    desktop.path =  $$PREFIX/share/applications
    icon.files += $$PWD/packaging/linux/common/notes.png
    desktop.files += $$PWD/packaging/linux/common/notes.desktop

    TARGET = notes
    INSTALLS += target desktop icon
}

macx {
    DESTDIR = $$PWD/../bin
    ICON = $$PWD/images\notes_icon.icns
}

win32 {
    DESTDIR = $$PWD/../bin
    RC_FILE = $$PWD/images\notes.rc
}

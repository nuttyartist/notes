#-------------------------------------------------
#
# Project created by QtCreator 2014-08-08T10:38:29
#
#-------------------------------------------------

VERSION = 0.9.0

QT += core gui network sql
QT += gui-private
QT += concurrent

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
    $$PWD/updaterwindow.cpp \
    $$PWD/dbmanager.cpp

HEADERS  += \
    $$PWD/mainwindow.h \
    $$PWD/notedata.h \
    $$PWD/notewidgetdelegate.h \
    $$PWD/notemodel.h \
    $$PWD/noteview.h \
    $$PWD/singleinstance.h \
    $$PWD/updaterwindow.h \
    $$PWD/dbmanager.h

FORMS += \
    $$PWD/mainwindow.ui \
    $$PWD/updaterwindow.ui

RESOURCES += \
    $$PWD/images.qrc \
    $$PWD/fonts.qrc \
    $$PWD/styles.qrc

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

    GIT_REV = $$system(git rev-parse --short HEAD)
    SNAPDIR = $$PWD/../packaging/linux/snap

    # This command bumps the version in the final snap every time it is built,
    # appending the git version of the latest commit to the VERSION variable
    # defined in this project file
    snap_bump_version.commands = \
        sed -i \"s/\\(^version:\\).*$$/\\1 \'$$VERSION~git$$GIT_REV\'/1\" $$SNAPDIR/snapcraft.yaml

    # Note: while it is planned to make snapcraft work across distros at the
    # time of writing `snapcraft` only works on Ubuntu. This means the snap
    # needs to be built from an Ubuntu host.
    snap.commands = cd $$SNAPDIR && \
        snapcraft clean && snapcraft
    snap.depends = snap_bump_version

    QMAKE_EXTRA_TARGETS   += \
        snap \
        snap_bump_version
}

macx {
    DESTDIR = $$PWD/../bin
    ICON = $$PWD/images\notes_icon.icns
}

win32 {
    DESTDIR = $$PWD/../bin
    RC_FILE = $$PWD/images\notes.rc
}

#-------------------------------------------------
#
# Project created by QtCreator 2014-08-08T10:38:29
#
#-------------------------------------------------

VERSION = 1.0.0

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

    target.path    = $$BINDIR
    icon.path      = $$PREFIX/share/pixmaps
    desktop.path   = $$PREFIX/share/applications
    icon.files    += $$PWD/packaging/linux/common/notes.png
    desktop.files += $$PWD/packaging/linux/common/notes.desktop

    TARGET    = notes
    INSTALLS += target desktop icon

    # SNAP  --------------------------------------------------------------------------------
    GIT_BRANCH = $$system(git rev-parse --abbrev-ref HEAD)
    GIT_REV = $$system(git rev-parse --short HEAD)
    SNAPDIR = $$_PRO_FILE_PWD_/../packaging/linux/snap

    # This command bumps the version in the final snap every time it is built,
    # appending the git version of the latest commit to the VERSION variable
    # defined in this project file
    equals(GIT_BRANCH, "master"){
        snap_bump_version.commands = \
            sed -i \"s/\\(^version:\\).*$$/\\1 \'$$VERSION\'/1\" $$SNAPDIR/snapcraft.yaml
    }else{
        snap_bump_version.commands = \
            sed -i \"s/\\(^version:\\).*$$/\\1 \'$$VERSION~git$$GIT_REV\'/1\" $$SNAPDIR/snapcraft.yaml
    }

    # Note: while it is planned to make snapcraft work across distros at the
    # time of writing `snapcraft` only works on Ubuntu. This means the snap
    # needs to be built from an Ubuntu host.
    snap.commands = rm -rf snap &&\
                    mkdir snap &&\
                    cd snap &&\
                    ln -s $$SNAPDIR/snapcraft.yaml ./snapcraft.yaml &&\
                    cp -r $$_PRO_FILE_PWD_/../../notes /tmp/ && rm -r /tmp/notes/build && cp -r /tmp/notes . &&\
                    sed -i \"s@^\\( *source: *\\).*@\\1./notes/@g\" snapcraft.yaml &&\
                    snapcraft clean && snapcraft

    snap.depends = snap_bump_version

    # Debian -------------------------------------------------------------------------------

    License = gpl2
    Project = "$$TARGET-$$VERSION"

    AuthorEmail = \"awesomeness.notes@gmail.com\"
    AuthorName = \"Nutty Artist\"

    deb.target   = deb
    deb.depends  = $$TARGET
    deb.depends  = fix_deb_dependencies
    deb.commands = rm -rf deb &&\
                   mkdir -p deb/$$Project &&\
                   cp $$TARGET deb/$$Project &&\
                   cp $$_PRO_FILE_PWD_/../packaging/linux/common/LICENSE deb/$$Project/license.txt &&\
                   cp -a $$_PRO_FILE_PWD_/../packaging/linux/common/icons deb/$$Project/ &&\
                   cp $$_PRO_FILE_PWD_/../packaging/linux/common/notes.desktop deb/$$Project/notes.desktop &&\
                   cp $$_PRO_FILE_PWD_/../packaging/linux/debian/copyright deb/$$Project/copyright &&\
                   cp -avr $$_PRO_FILE_PWD_/../packaging/linux/debian deb/$$Project/debian &&\
                   cd deb/$$Project/ &&\
                   DEBFULLNAME=$$AuthorName EMAIL=$$AuthorEmail dh_make -y -s -c $$License --createorig; \
                   dpkg-buildpackage -uc -us

    fix_deb_dependencies.commands = \
        sed -i -- 's/5.2/$$QT_MAJOR_VERSION\.$$QT_MINOR_VERSION/g'  $$_PRO_FILE_PWD_/../packaging/linux/debian/control

    # AppImage -------------------------------------------------------------------------------

    appimage.target   = appimage
    appimage.depends  = deb
    appimage.commands = rm -rf appdir &&\
                        mkdir appdir &&\
                        cd appdir &&\
                        dpkg -x ../deb/*.deb . &&\
                        cp $$_PRO_FILE_PWD_/../packaging/linux/common/notes.desktop . &&\
                        cp $$_PRO_FILE_PWD_/../packaging/linux/common/icons/256x256/notes.png . &&\
                        mkdir -p ./usr/share/icons/default/256x256/apps/ &&\
                        cp $$_PRO_FILE_PWD_/../packaging/linux/common/icons/256x256/notes.png ./usr/share/icons/default/256x256/apps/  &&\
                        cd .. &&\
                        wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/4/linuxdeployqt-4-x86_64.AppImage"  &&\
                        chmod a+x linuxdeployqt*.AppImage  &&\
                        unset QTDIR; unset QT_PLUGIN_PATH &&\
                        unset LD_LIBRARY_PATH  &&\
                        ./linuxdeployqt*.AppImage ./appdir/usr/bin/notes -bundle-non-qt-libs  &&\
                        ./linuxdeployqt*.AppImage ./appdir/usr/bin/notes -bundle-non-qt-libs  &&\
                        ./linuxdeployqt*.AppImage --appimage-extract  &&\
                        wget -c https://github.com/probonopd/AppImageKit/raw/master/desktopintegration -O ./appdir/usr/bin/notes.wrapper  &&\
                        chmod a+x ./appdir/usr/bin/notes.wrapper  &&\
                        (cd ./appdir/ ; rm AppRun ; ln -s ./usr/bin/notes.wrapper AppRun)  &&\
                        ./squashfs-root/usr/bin/appimagetool ./appdir/

    # EXTRA --------------------------------------------------------------------------------
    QMAKE_EXTRA_TARGETS += \
                           snap                 \
                           snap_bump_version    \
                           deb                  \
                           fix_deb_dependencies \
                           appimage
}

macx {
    DESTDIR = $$PWD/../bin
    ICON = $$PWD/images\notes_icon.icns
}

win32 {
    DESTDIR = $$PWD/../bin
    RC_FILE = $$PWD/images\notes.rc
}

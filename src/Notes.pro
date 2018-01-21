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
include ($$PWD/../3rdParty/qautostart/src/qautostart.pri)

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
    deb.depends  = fix_deb_dependencies
    deb.depends += $$TARGET
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

    appimage.target    = appimage
    appimage.depends   = $$TARGET
    appimage.commands  = mkdir -p Notes/usr/bin;
    appimage.commands += cp $$TARGET Notes/usr/bin;
    appimage.commands += mkdir -p Notes/usr/share/applications/;
    appimage.commands += cp $$_PRO_FILE_PWD_/../packaging/linux/common/notes.desktop Notes/usr/share/applications/;
    appimage.commands += cp $$_PRO_FILE_PWD_/../packaging/linux/common/icons/256x256/notes.png Notes;
    appimage.commands += mkdir -p Notes/usr/share/icons/default/256x256/apps/;
    appimage.commands += cp $$_PRO_FILE_PWD_/../packaging/linux/common/icons/256x256/notes.png Notes/usr/share/icons/default/256x256/apps/;
    appimage.commands += wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage";
    appimage.commands += chmod a+x linuxdeployqt*.AppImage;
    appimage.commands += unset QTDIR; unset QT_PLUGIN_PATH; unset LD_LIBRARY_PATH;
    appimage.commands += ./linuxdeployqt*.AppImage Notes/usr/share/applications/*.desktop -bundle-non-qt-libs;
    appimage.commands += cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6 Notes/usr/lib;
    appimage.commands += ./linuxdeployqt*.AppImage Notes/usr/share/applications/*.desktop -appimage;
    appimage.commands += find Notes -executable -type f -exec ldd {} \; | grep \" => /usr\" | cut -d \" \" -f 2-3 | sort | uniq;

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

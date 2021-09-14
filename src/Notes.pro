#-------------------------------------------------
#
# Project created by QtCreator 2014-08-08T10:38:29
#
#-------------------------------------------------

VERSION = 1.5.0

QT += core gui widgets network sql
QT += gui-private
QT += concurrent

TARGET    = Notes
TEMPLATE  = app
CONFIG   += c++11

UI_DIR = uic
MOC_DIR = moc
RCC_DIR = qrc
OBJECTS_DIR = obj

include ($$PWD/../3rdParty/qxt/qxt.pri)
include ($$PWD/../3rdParty/QSimpleUpdater/QSimpleUpdater.pri)
include ($$PWD/../3rdParty/qmarkdowntextedit/qmarkdowntextedit.pri)
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
    $$PWD/dbmanager.cpp \
    aboutwindow.cpp \
    customdocument.cpp \
    editorsettingsbutton.cpp \
    styleeditorwindow.cpp

HEADERS  += \
    $$PWD/mainwindow.h \
    $$PWD/notedata.h \
    $$PWD/notewidgetdelegate.h \
    $$PWD/notemodel.h \
    $$PWD/noteview.h \
    $$PWD/singleinstance.h \
    $$PWD/updaterwindow.h \
    $$PWD/dbmanager.h \
    aboutwindow.h \
    customDocument.h \
    editorsettingsbutton.h \
    framelesswindow.h \
    styleeditorwindow.h

FORMS += \
    $$PWD/mainwindow.ui \
    $$PWD/updaterwindow.ui \
    aboutwindow.ui \
    styleeditorwindow.ui

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
    snap_pack.commands = snapcraft clean && snapcraft

    # Debian -------------------------------------------------------------------------------

    License = gpl2
    Project = "$$TARGET-$$VERSION"

    AuthorEmail = \"awesomeness.notes@gmail.com\"
    AuthorName = \"Ruby Mamistvalove\"

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
                           snap_pack            \
                           deb                  \
                           fix_deb_dependencies \
                           appimage
}

macx {
    DESTDIR = $$PWD/../bin
    ICON = $$PWD/images\notes_icon.icns
    OBJECTIVE_SOURCES += \
                framelesswindow.mm
    LIBS += -framework Cocoa
}

win32 {
    DESTDIR = $$PWD/../bin
    RC_FILE = $$PWD/images\notes.rc
    SOURCES += \
        framelesswindow.cpp
}

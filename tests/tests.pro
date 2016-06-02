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
LIBS += \
        ../src/qxtglobalshortcut_x11.o      \
        -lX11
}

macx{
LIBS += \
        ../src/qxtglobalshortcut_mac.o
}

macx{
LIBS += \
        ../src/qxtglobalshortcut_win.o
}

LIBS += ../src/qxtglobal.o                  \
        ../src/qxtglobalshortcut.o          \
        ../src/moc_qxtglobalshortcut.o      \
        ../src/moc_singleinstance.o         \
        ../src/singleinstance.o             \
        ../src/moc_notedata.o               \
        ../src/notedata.o                   \
        ../src/moc_noteview.o               \
        ../src/noteview.o                   \
        ../src/notemodel.o                  \
        ../src/moc_notewidgetdelegate.o     \
        ../src/notewidgetdelegate.o         \
        ../src/mainwindow.o                 \
        ../src/moc_mainwindow.o

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

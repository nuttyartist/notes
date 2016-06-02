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
        ../src/OBJ/qxtglobalshortcut_x11.o      \
        -lX11
}

macx{
LIBS += \
        ../src/OBJ/qxtglobalshortcut_mac.o
}

win32{
LIBS += \
        ../src/OBJ/qxtglobalshortcut_win.o
}

LIBS += ../src/OBJ/qxtglobal.o                  \
        ../src/OBJ/qxtglobalshortcut.o          \
        ../src/OBJ/moc_qxtglobalshortcut.o      \
        ../src/OBJ/moc_singleinstance.o         \
        ../src/OBJ/singleinstance.o             \
        ../src/OBJ/moc_notedata.o               \
        ../src/OBJ/notedata.o                   \
        ../src/OBJ/moc_noteview.o               \
        ../src/OBJ/noteview.o                   \
        ../src/OBJ/notemodel.o                  \
        ../src/OBJ/moc_notewidgetdelegate.o     \
        ../src/OBJ/notewidgetdelegate.o         \
        ../src/OBJ/mainwindow.o                 \
        ../src/OBJ/moc_mainwindow.o

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

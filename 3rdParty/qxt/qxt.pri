INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/qxtglobal.h \
    $$PWD/qxtglobalshortcut_p.h \
    $$PWD/qxtglobalshortcut.h

SOURCES += \
    $$PWD/qxtglobal.cpp \
    $$PWD/qxtglobalshortcut.cpp

win32 {
    LIBS+= -luser32
    DEFINES += QXT_STATIC
    SOURCES += $$PWD/qxtglobalshortcut_win.cpp
}

macx {
    LIBS += -framework Carbon
    SOURCES += $$PWD/qxtglobalshortcut_mac.cpp
}

unix:!macx {
    LIBS += -lX11
    SOURCES += $$PWD/qxtglobalshortcut_x11.cpp
}

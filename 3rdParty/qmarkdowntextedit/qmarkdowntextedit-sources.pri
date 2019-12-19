INCLUDEPATH += $$PWD/

QT       += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    $$PWD/markdownhighlighter.cpp \
    $$PWD/qmarkdowntextedit.cpp \
    $$PWD/qplaintexteditsearchwidget.cpp

RESOURCES += \
    $$PWD/media.qrc

FORMS += $$PWD/qplaintexteditsearchwidget.ui

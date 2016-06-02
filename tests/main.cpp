#include <QApplication>
#include <QTest>
#include "tst_notedata.h"
#include "tst_notemodel.h"
#include "tst_noteview.h"
#include "tst_mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTest::qExec(new tst_NoteData, argc, argv);
    QTest::qExec(new tst_NoteModel, argc, argv);
    QTest::qExec(new tst_NoteView, argc, argv);
    QTest::qExec(new tst_MainWindow, argc, argv);
    return 0;
}

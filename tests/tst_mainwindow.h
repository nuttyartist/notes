#ifndef TST_MAINWINDOW_H
#define TST_MAINWINDOW_H

#include <QString>
#include <QtTest>
#include "../src/mainwindow.h"

class tst_MainWindow : public QObject
{
    Q_OBJECT

public:
    tst_MainWindow();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

};

#endif // TST_MAINWINDOW_H

#ifndef TST_NOTEDATA_H
#define TST_NOTEDATA_H

#include <QtTest>
#include "../src/notedata.h"

class tst_NoteData : public QObject
{
    Q_OBJECT

public:
    tst_NoteData();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

};

#endif // TST_NOTEDATA_H

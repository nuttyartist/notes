#ifndef TST_NOTEMODEL_H
#define TST_NOTEMODEL_H

#include <QObject>
#include <QtTest>

class tst_NoteModel : public QObject
{
    Q_OBJECT
public:
    tst_NoteModel();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
};

#endif // TST_NOTEMODEL_H

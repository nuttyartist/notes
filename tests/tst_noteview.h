#ifndef TST_NOTEVIEW_H
#define TST_NOTEVIEW_H

#include <QObject>

class tst_NoteView : public QObject
{
    Q_OBJECT
public:
    tst_NoteView();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
};

#endif // TST_NOTEVIEW_H

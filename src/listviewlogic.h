#ifndef LISTVIEWLOGIC_H
#define LISTVIEWLOGIC_H

#include <QObject>
#include <QVector>
#include "nodedata.h"

class NoteListView;
class NoteListModel;
class DBManager;

class ListViewLogic : public QObject
{
    Q_OBJECT
public:
    explicit ListViewLogic(NoteListView* noteView,
                           NoteListModel* noteModel,
                           DBManager* dbManager,
                           QObject *parent = nullptr);

public slots:
    void moveNoteToTop(const NodeData& note);
    void setNoteData(const NodeData& note);

signals:
    void showNoteInEditor(const NodeData& noteData);

private slots:
    void loadNoteListModel(QVector<NodeData> noteList);

private:
    void selectFirstNote();
private:
    NoteListView* m_listView;
    NoteListModel* m_listModel;
    DBManager* m_dbManager;
};

#endif // LISTVIEWLOGIC_H

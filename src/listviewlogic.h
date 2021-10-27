#ifndef LISTVIEWLOGIC_H
#define LISTVIEWLOGIC_H

#include <QObject>
#include <QVector>
#include "nodedata.h"

class NoteListView;
class NoteListModel;
class DBManager;
class TagPool;

class ListViewLogic : public QObject
{
    Q_OBJECT
public:
    explicit ListViewLogic(NoteListView* noteView,
                           NoteListModel* noteModel,
                           TagPool* tagPool,
                           DBManager* dbManager,
                           QObject *parent = nullptr);
    void selectNote(const QModelIndex &noteIndex);

public slots:
    void moveNoteToTop(const NodeData& note);
    void setNoteData(const NodeData& note);
    void deleteTempNote(const NodeData& note);
signals:
    void showNoteInEditor(const NodeData& noteData);
    void requestAddTagDb(int noteId, int tagId);

private slots:
    void loadNoteListModel(QVector<NodeData> noteList);
    void onAddTagRequest(const QModelIndex& index, int tagIds);
    void onNotePressed(const QModelIndex& index);

private:
    void selectFirstNote();
private:
    NoteListView* m_listView;
    NoteListModel* m_listModel;
    DBManager* m_dbManager;
};

#endif // LISTVIEWLOGIC_H

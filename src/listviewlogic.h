#ifndef LISTVIEWLOGIC_H
#define LISTVIEWLOGIC_H

#include <QObject>
#include <QVector>
#include "nodedata.h"
#include "dbmanager.h"

class NoteListView;
class NoteListModel;
class TagPool;
class NoteListDelegate;

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
    void onNoteEditClosed(const NodeData& note);
    void deleteNoteRequested(const NodeData& note);
signals:
    void showNoteInEditor(const NodeData& noteData);
    void requestAddTagDb(int noteId, int tagId);
    void requestRemoveNoteDb(const NodeData& noteData);
    void closeNoteEditor();

private slots:
    void loadNoteListModel(const QVector<NodeData>& noteList, const ListViewInfo& inf);
    void onAddTagRequest(const QModelIndex& index, int tagIds);
    void onNotePressed(const QModelIndex& index);
    void deleteNoteRequestedI(const QModelIndex& index);

private:
    void selectFirstNote();

private:
    NoteListView* m_listView;
    NoteListModel* m_listModel;
    DBManager* m_dbManager;
    NoteListDelegate* m_listDelegate;

    ListViewInfo m_listViewInfo;
};

#endif // LISTVIEWLOGIC_H

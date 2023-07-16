#ifndef LISTVIEWLOGIC_H
#define LISTVIEWLOGIC_H

#include <QObject>
#include <QVector>
#include "nodedata.h"
#include "dbmanager.h"
#include "editorsettingsoptions.h"
#include <QModelIndex>

class NoteListView;
class NoteListModel;
class TagPool;
class NoteListDelegate;
class QLineEdit;
class QToolButton;
class TagPool;

class ListViewLogic : public QObject
{
    Q_OBJECT
public:
    explicit ListViewLogic(NoteListView *noteView, NoteListModel *noteModel, QLineEdit *searchEdit,
                           QToolButton *clearButton, TagPool *tagPool, DBManager *dbManager,
                           QObject *parent = nullptr);
    void selectNote(const QModelIndex &noteIndex);

    const ListViewInfo &listViewInfo() const;
    void selectFirstNote();
    void setTheme(Theme::Value theme);
    bool isAnimationRunning();
    void setLastSavedState(const QSet<int> &lastSelectedNotes, int needLoadSavedState = 2);
    void requestLoadSavedState(int needLoadSavedState);
    void selectAllNotes();
public slots:
    void moveNoteToTop(const NodeData &note);
    void setNoteData(const NodeData &note);
    void onNoteEditClosed(const NodeData &note, bool selectNext);
    void deleteNoteRequested(const NodeData &note);
    void selectNoteUp();
    void selectNoteDown();
    void onSearchEditTextChanged(const QString &keyword);
    void clearSearch(bool createNewNote = false, int scrollToId = SpecialNodeID::InvalidNodeId);
    void onAddTagRequestD(int noteId, int tagId);
    void onNoteMovedOut(int nodeId, int targetId);
    void setLastSelectedNote();
    void loadLastSelectedNoteRequested();
    void onNotesListInFolderRequested(int parentID, bool isRecursive, bool newNote, int scrollToId);
    void onNotesListInTagsRequested(const QSet<int> &tagIds, bool newNote, int scrollToId);
    void selectNotes(const QModelIndexList &indexes);
signals:
    void showNotesInEditor(const QVector<NodeData> &notesData);
    void requestAddTagDb(int noteId, int tagId);
    void requestRemoveTagDb(int noteId, int tagId);
    void requestRemoveNoteDb(const NodeData &noteData);
    void requestMoveNoteDb(int noteId, const NodeData &targetFolder);
    void requestHighlightSearch();
    void closeNoteEditor();
    void noteTagListChanged(int noteId, const QSet<int> &tagIds);
    void requestSearchInDb(const QString &keyword, const ListViewInfo &inf);
    void requestClearSearchDb(const ListViewInfo &inf);
    void requestClearSearchUI();
    void requestNewNote();
    void moveNoteRequested(int id, int target);
    void listViewLabelChanged(const QString &label1, const QString &label2);
    void setNewNoteButtonVisible(bool visible);
    void requestNotesListInFolder(int parentID, bool isRecursive, bool newNote, int scrollToId);
    void requestNotesListInTags(const QSet<int> &tagIds, bool newNote, int scrollToId);

private slots:
    void loadNoteListModel(const QVector<NodeData> &noteList, const ListViewInfo &inf);
    void onAddTagRequest(const QModelIndex &index, int tagId);
    void onRemoveTagRequest(const QModelIndex &index, int tagId);
    void onNotePressed(const QModelIndexList &indexes);
    void deleteNoteRequestedI(const QModelIndexList &indexes);
    void restoreNotesRequestedI(const QModelIndexList &indexes);
    void updateListViewLabel();
    void onRowCountChanged();
    void onNoteDoubleClicked(const QModelIndex &index);
    void onSetPinnedNoteRequested(const QModelIndexList &indexes, bool isPinned);
    void onListViewClicked();

private:
    NoteListView *m_listView;
    NoteListModel *m_listModel;
    QLineEdit *m_searchEdit;
    QToolButton *m_clearButton;
    DBManager *m_dbManager;
    NoteListDelegate *m_listDelegate;
    TagPool *m_tagPool;
    ListViewInfo m_listViewInfo;
    QVector<QModelIndex> m_editorIndexes;

    int m_needLoadSavedState;
    QSet<int> m_lastSelectedNotes;
};

#endif // LISTVIEWLOGIC_H

#ifndef LISTVIEWLOGIC_H
#define LISTVIEWLOGIC_H

#include <QObject>
#include <QVector>
#include "nodedata.h"
#include "dbmanager.h"
#include "styleeditorwindow.h"
#include <QModelIndex>
#include <utility>
#include <functional>

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
    explicit ListViewLogic(NoteListView* noteView,
                           NoteListModel* noteModel,
                           NoteListView* pinnedNoteView,
                           NoteListModel* pinnedNoteModel,
                           QLineEdit* searchEdit,
                           QToolButton* clearButton,
                           TagPool* tagPool,
                           DBManager* dbManager,
                           QObject *parent = nullptr);
    void selectNote(NoteListView &listView, const QModelIndex &noteIndex);

    const ListViewInfo &listViewInfo() const;
    void selectFirstNote();
    void selectLastNote();
    void setTheme(Theme theme);
    bool isAnimationRunning();
    void setLastSavedState(int lastSelectedNote);
    bool isHavePinnedNote() const;

    int minimiumNoteListHeight();
    int minimiumPinnedNoteListHeight();
    int maximiumPinnedNoteListHeight();
    void selectFirstUnpinned();

public slots:
    void moveNoteToTop(const NodeData& note);
    void setNoteData(const NodeData& note);
    void onNoteEditClosed(const NodeData& note);
    void deleteNoteRequested(const NodeData& note);
    void selectNoteUp();
    void selectNoteDown();
    void onSearchEditTextChanged(const QString &keyword);
    void clearSearch(bool createNewNote = false, int scrollToId = SpecialNodeID::InvalidNodeId);
    void onAddTagRequestD(int noteId, int tagId);
    void onNoteMovedOut(int nodeId, int targetId);

signals:
    void showNoteInEditor(const NodeData& noteData);
    void requestAddTagDb(int noteId, int tagId);
    void requestRemoveTagDb(int noteId, int tagId);
    void requestRemoveNoteDb(const NodeData& noteData);
    void requestMoveNoteDb(int noteId, const NodeData& targetFolder);
    void requestHightlightSearch();
    void closeNoteEditor();
    void noteTagListChanged(int noteId, const QSet<int>& tagIds);
    void requestSearchInDb(const QString& keyword, const ListViewInfo& inf);
    void requestClearSearchDb(const ListViewInfo& inf);
    void requestClearSearchUI();
    void requestNewNote();
    void moveNoteRequested(int id, int target);
    void listViewLabelChanged(const QString& label1, const QString& label2, bool havePinnedNote);
    void setNewNoteButtonVisible(bool visible);
    void pinnedNoteListVisibleChanged(bool visible);
    void requestUpdatePinnedDb(int noteId, bool isPinned);
    void configPinnedNoteSpliter();

private slots:
    void loadNoteListModel(const QVector<NodeData>& noteList, const ListViewInfo& inf);
    void onAddTagRequest(NoteListView &listView, const QModelIndex& index, int tagId);
    void onRemoveTagRequest(NoteListView &listView, const QModelIndex& index, int tagId);
    void onNotePressed(NoteListView &listView, const QModelIndex& index);
    void deleteNoteRequestedI(NoteListView &listView, const QModelIndex& index);
    void restoreNoteRequestedI(NoteListView &listView, const QModelIndex& index);
    void updateListViewLabel();
    void onRowCountChanged(NoteListView &listView);
    void onNoteDoubleClicked(const QModelIndex& index);
    void onSetPinnedNoteRequested(int noteId, bool isPinned);

private:
    NoteListView* m_listView;
    NoteListModel* m_listModel;
    NoteListView* m_pinnedNoteView;
    NoteListModel* m_pinnedNoteModel;
    QLineEdit* m_searchEdit;
    QToolButton* m_clearButton;
    DBManager* m_dbManager;
    NoteListDelegate* m_noteListDelegate;
    NoteListDelegate* m_pinnedNoteListDelegate;
    TagPool* m_tagPool;
    ListViewInfo m_listViewInfo;
    QVector<QModelIndex> m_editorIndexes;

    int m_needLoadSavedState;
    int m_lastSelectedNote;

    bool havePinnedNote() const;
    std::pair<QModelIndex, NoteListView &> getNoteIndex(int id) const;
    std::pair<QModelIndex, NoteListView &> getCurrentIndex() const;
};

#endif // LISTVIEWLOGIC_H

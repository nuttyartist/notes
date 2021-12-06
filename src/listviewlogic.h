#ifndef LISTVIEWLOGIC_H
#define LISTVIEWLOGIC_H

#include <QObject>
#include <QVector>
#include "nodedata.h"
#include "dbmanager.h"
#include "styleeditorwindow.h"
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
    explicit ListViewLogic(NoteListView* noteView,
                           NoteListModel* noteModel,
                           QLineEdit* searchEdit,
                           QToolButton* clearButton,
                           TagPool* tagPool,
                           DBManager* dbManager,
                           QObject *parent = nullptr);
    void selectNote(const QModelIndex &noteIndex);

    const ListViewInfo &listViewInfo() const;
    void selectFirstNote();
    void setTheme(Theme theme);
    bool isAnimationRunning();

public slots:
    void moveNoteToTop(const NodeData& note);
    void setNoteData(const NodeData& note);
    void onNoteEditClosed(const NodeData& note);
    void deleteNoteRequested(const NodeData& note);
    void selectNoteUp();
    void selectNoteDown();
    void onSearchEditTextChanged(const QString &keyword);
    void clearSearch();
    void onAddTagRequestD(int noteId, int tagId);

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
    void listViewLabelChanged(const QString& label1, const QString& label2);
private slots:
    void loadNoteListModel(const QVector<NodeData>& noteList, const ListViewInfo& inf);
    void onAddTagRequest(const QModelIndex& index, int tagId);
    void onRemoveTagRequest(const QModelIndex& index, int tagId);
    void onNotePressed(const QModelIndex& index);
    void deleteNoteRequestedI(const QModelIndex& index);
    void restoreNoteRequestedI(const QModelIndex& index);
    void updateListViewLabel();
    void onRowCountChanged();
private:
    NoteListView* m_listView;
    NoteListModel* m_listModel;
    QLineEdit* m_searchEdit;
    QToolButton* m_clearButton;
    DBManager* m_dbManager;
    NoteListDelegate* m_listDelegate;
    TagPool* m_tagPool;
    ListViewInfo m_listViewInfo;
    QVector<QModelIndex> m_editorIndexes;
};

#endif // LISTVIEWLOGIC_H

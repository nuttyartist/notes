#ifndef TREEVIEWLOGIC_H
#define TREEVIEWLOGIC_H

#include <QObject>
#include "dbmanager.h"
#include "editorsettingsoptions.h"

class NodeTreeView;
class NodeTreeModel;
class NodeTreeDelegate;
class DBManager;
class CustomApplicationStyle;
class NoteListView;

class TreeViewLogic : public QObject
{
    Q_OBJECT
public:
    explicit TreeViewLogic(NodeTreeView *treeView, NodeTreeModel *treeModel, DBManager *dbManager,
                           NoteListView *listView, QObject *parent = nullptr);
    void openFolder(int id);
    void onMoveNodeRequested(int nodeId, int targetId);
    void setTheme(Theme::Value theme);
    void setLastSavedState(bool isLastSelectFolder, const QString &lastSelectFolder,
                           const QSet<int> &lastSelectTag, const QStringList &expandedFolder);
private slots:
    void updateTreeViewSeparator();
    void loadTreeModel(const NodeTagTreeData &treeData);
    void onAddTagRequested();
    void onRenameNodeRequestedFromTreeView(const QModelIndex &index, const QString &newName);
    void onDeleteFolderRequested(const QModelIndex &index);
    void onRenameTagRequestedFromTreeView(const QModelIndex &index, const QString &newName);
    void onChangeTagColorRequested(const QModelIndex &index);
    void onDeleteTagRequested(const QModelIndex &index);
    void onChildNotesCountChangedTag(int tagId, int notesCount);
    void onChildNoteCountChangedFolder(int folderId, const QString &absPath, int notesCount);

signals:
    void requestRenameNodeInDB(int id, const QString &newName);
    void requestRenameTagInDB(int id, const QString &newName);
    void requestChangeTagColorInDB(int id, const QString &newColor);
    void requestMoveNodeInDB(int id, const NodeData &target);
    void addNoteToTag(int noteId, int tagId);
    void noteMoved(int nodeId, int targetId);

private:
    void onAddFolderRequested(bool fromPlusButton);

private:
    NodeTreeView *m_treeView;
    NodeTreeModel *m_treeModel;
    NoteListView *m_listView;
    NodeTreeDelegate *m_treeDelegate;
    DBManager *m_dbManager;
    CustomApplicationStyle *m_style;
    bool m_needLoadSavedState;
    bool m_isLastSelectFolder;
    QString m_lastSelectFolder;
    QSet<int> m_lastSelectTags;
    QStringList m_expandedFolder;
};

#endif // TREEVIEWLOGIC_H

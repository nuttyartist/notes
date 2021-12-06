#ifndef TREEVIEWLOGIC_H
#define TREEVIEWLOGIC_H

#include <QObject>
#include "dbmanager.h"
#include "styleeditorwindow.h"

class NodeTreeView;
class NodeTreeModel;
class NodeTreeDelegate;
class DBManager;
class CustomApplicationStyle;

class TreeViewLogic : public QObject
{
    Q_OBJECT
public:
    explicit TreeViewLogic(NodeTreeView* treeView,
                                        NodeTreeModel* treeModel,
                                        DBManager* dbManager,
                                        QObject *parent = nullptr);
    void openFolder(int id);
    void onMoveNodeRequested(int nodeId, int targetId);
    void setTheme(Theme theme);

private slots:
    void updateTreeViewSeparator();
    void loadTreeModel(const NodeTagTreeData &treeData);
    void onAddTagRequested();
    void onRenameNodeRequestedFromTreeView(const QModelIndex& index, const QString& newName);
    void onDeleteFolderRequested(const QModelIndex& index);
    void onRenameTagRequestedFromTreeView(const QModelIndex& index, const QString& newName);
    void onChangeTagColorRequested(const QModelIndex& index);
    void onDeleteTagRequested(const QModelIndex& index);
signals:
    void requestRenameNodeInDB(int id, const QString& newName);
    void requestRenameTagInDB(int id, const QString& newName);
    void requestChangeTagColorInDB(int id, const QString& newColor);
    void requestMoveNodeInDB(int id, const NodeData& target);
    void addNoteToTag(int noteId, int tagId);

private:
    void onAddFolderRequested(bool fromPlusButton);

private:
    NodeTreeView* m_treeView;
    NodeTreeModel* m_treeModel;
    NodeTreeDelegate* m_treeDelegate;
    DBManager* m_dbManager;
    CustomApplicationStyle* m_style;
};

#endif // TREEVIEWLOGIC_H

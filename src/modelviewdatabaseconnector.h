#ifndef MODELVIEWDATABASECONNECTOR_H
#define MODELVIEWDATABASECONNECTOR_H

#include <QObject>
#include "dbmanager.h"

class NodeTreeView;
class NodeTreeModel;
class NodeTreeDelegate;
class DBManager;

class ModelViewDatabaseConnector : public QObject
{
    Q_OBJECT
public:
    explicit ModelViewDatabaseConnector(NodeTreeView* treeView,
                                        NodeTreeModel* treeModel,
                                        DBManager* dbManager,
                                        QObject *parent = nullptr);

public slots:

private slots:
    void updateTreeViewSeparator();
    void loadTreeModel(const NodeTagTreeData &treeData);
    void onAddFolderRequested();
    void onAddTagRequested();
    void onRenameNodeRequestedFromTreeView(const QModelIndex& index, const QString& newName);
    void onDeleteFolderRequested(const QModelIndex& index);
signals:
    void requestRenameNodeInDB(int id, const QString& newName);

private:
    NodeTreeView* m_treeView;
    NodeTreeModel* m_treeModel;
    NodeTreeDelegate* m_treeDelegate;
    DBManager* m_dbManager;
};

#endif // MODELVIEWDATABASECONNECTOR_H

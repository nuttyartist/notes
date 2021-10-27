#include "treeviewlogic.h"
#include "nodetreeview.h"
#include "nodetreemodel.h"
#include "nodetreedelegate.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QMetaObject>
#include <QMessageBox>

TreeViewLogic::TreeViewLogic(NodeTreeView* treeView,
                             NodeTreeModel* treeModel,
                             DBManager* dbManager,
                             QObject *parent) :
    QObject(parent),
    m_treeView{treeView},
    m_treeModel{treeModel},
    m_dbManager{dbManager}
{
    m_treeDelegate = new NodeTreeDelegate(m_treeView, m_treeView);
    m_treeView->setItemDelegate(m_treeDelegate);
    connect(m_dbManager, &DBManager::nodesTagTreeReceived,
            this, &TreeViewLogic::loadTreeModel);
    connect(m_treeModel, &NodeTreeModel::topLevelItemLayoutChanged,
            this, &TreeViewLogic::updateTreeViewSeparator);
    connect(m_treeView, &NodeTreeView::addFolderRequested,
            this, &TreeViewLogic::onAddFolderRequested);
    connect(m_treeDelegate, &NodeTreeDelegate::addFolderRequested,
            this, &TreeViewLogic::onAddFolderRequested);
    connect(m_treeDelegate, &NodeTreeDelegate::addTagRequested,
            this, &TreeViewLogic::onAddTagRequested);
    connect(m_treeView, &NodeTreeView::renameFolderInDatabase,
            this, &TreeViewLogic::onRenameNodeRequestedFromTreeView);
    connect(this, &TreeViewLogic::requestRenameNodeInDB,
            m_dbManager, &DBManager::renameNode, Qt::QueuedConnection);
    connect(m_treeView, &NodeTreeView::deleteNodeRequested,
            this, &TreeViewLogic::onDeleteFolderRequested);
    connect(m_treeView, &NodeTreeView::loadNotesInFolderRequested,
            m_dbManager, &DBManager::onNotesListInFolderRequested, Qt::QueuedConnection);
    connect(m_treeView, &NodeTreeView::loadNotesInTagRequested,
            m_dbManager, &DBManager::onNotesListInTagRequested, Qt::QueuedConnection);
}

void TreeViewLogic::updateTreeViewSeparator()
{
    m_treeView->setTreeSeparator(m_treeModel->getSeparatorIndex());
}

void TreeViewLogic::loadTreeModel(const NodeTagTreeData &treeData)
{
    m_treeModel->setTreeData(treeData);
    updateTreeViewSeparator();
}

void TreeViewLogic::onAddFolderRequested()
{
    auto currentIndex = m_treeView->currentIndex();
    int parentId = SpecialNodeID::RootFolder;
    if (currentIndex.isValid()) {
        auto type = static_cast<NodeItem::Type>(currentIndex.data(NodeItem::Roles::ItemType).toInt());
        if (type == NodeItem::FolderItem) {
            parentId = currentIndex.data(NodeItem::Roles::NodeId).toInt();
            // we don't allow subfolder under default notes folder
            if (parentId == SpecialNodeID::DefaultNotesFolder) {
                parentId = SpecialNodeID::RootFolder;
            }
        } else if (type == NodeItem::NoteItem) {
            qDebug() << "Can create folder under this item";
            return;
        } else {
            currentIndex = m_treeModel->rootIndex();
        }
    } else {
        currentIndex = m_treeModel->rootIndex();
    }
    int newlyCreatedNodeId;
    NodeData newFolder;
    newFolder.setNodeType(NodeData::Folder);
    QDateTime noteDate = QDateTime::currentDateTime();
    newFolder.setCreationDateTime(noteDate);
    newFolder.setLastModificationDateTime(noteDate);
    if (parentId != SpecialNodeID::RootFolder) {
        newFolder.setFullTitle(m_treeModel->getNewFolderPlaceholderName(currentIndex));
    } else {
        newFolder.setFullTitle(m_treeModel->getNewFolderPlaceholderName(m_treeModel->rootIndex()));
    }
    newFolder.setParentId(parentId);

    QMetaObject::invokeMethod(m_dbManager, "addNode", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(int, newlyCreatedNodeId),
                              Q_ARG(NodeData, newFolder)
                              );

    QHash<NodeItem::Roles, QVariant> hs;
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::FolderItem;
    hs[NodeItem::Roles::DisplayText] = newFolder.fullTitle();
    hs[NodeItem::Roles::NodeId] = newlyCreatedNodeId;
    if (parentId != SpecialNodeID::RootFolder) {
        m_treeModel->appendChildNodeToParent(currentIndex, hs);
        if (!m_treeView->isExpanded(currentIndex)) {
            m_treeView->expand(currentIndex);
        }
    } else {
        m_treeModel->appendChildNodeToParent(m_treeModel->rootIndex(), hs);
    }
}

void TreeViewLogic::onAddTagRequested()
{
    int newlyCreatedTagId;
    TagData newTag;
    newTag.setName(m_treeModel->getNewTagPlaceholderName());
    // random color generator
    double rand = QRandomGenerator::global()->generateDouble();
    int h = rand * 359;
    int s = 255;
    int v = 128 + rand * 127;
    auto color = QColor::fromHsv(h,s,v);

    newTag.setColor(color.name());
    QMetaObject::invokeMethod(m_dbManager, "addTag", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(int, newlyCreatedTagId),
                              Q_ARG(TagData, newTag)
                              );

    QHash<NodeItem::Roles, QVariant> hs;
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::TagItem;
    hs[NodeItem::Roles::DisplayText] = newTag.name();
    hs[NodeItem::Roles::TagColor] = newTag.color();
    hs[NodeItem::Roles::NodeId] = newlyCreatedTagId;
    m_treeModel->appendChildNodeToParent(m_treeModel->rootIndex(), hs);
}

void TreeViewLogic::onRenameNodeRequestedFromTreeView(const QModelIndex &index, const QString &newName)
{
    m_treeModel->setData(index, newName, NodeItem::Roles::DisplayText);
    auto id = index.data(NodeItem::Roles::NodeId).toInt();
    emit requestRenameNodeInDB(id, newName);
}

void TreeViewLogic::onDeleteFolderRequested(const QModelIndex &index)
{
    auto btn = QMessageBox::question(nullptr, "Are you sure you want to delete this folder",
                                     "Are you sure you want to delete this folder? All notes and any subfolders will be deleted.");
    if (btn == QMessageBox::Yes) {
        auto id = index.data(NodeItem::Roles::NodeId).toInt();
        if (id < SpecialNodeID::DefaultNotesFolder) {
            qDebug() << __FUNCTION__ << "Failed while trying to delete folder with id" << id;
            return;
        }
        NodeData node;
        QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(NodeData, node),
                                  Q_ARG(int, id)
                                  );
        auto parentPath = NodePath{node.absolutePath()}.parentPath();
        auto parentIndex = m_treeModel->indexFromIdPath(parentPath);
        if (parentIndex.isValid()) {
            m_treeModel->deleteRow(index, parentIndex);
            QMetaObject::invokeMethod(m_dbManager, "moveFolderToTrash", Qt::QueuedConnection,
                                      Q_ARG(NodeData, node));
        } else {
            qDebug() << __FUNCTION__ << "Parent index with path" << parentPath.path()
                     << "is not valid";
        }
    }
}


#include "modelviewdatabaseconnector.h"
#include "nodetreeview.h"
#include "nodetreemodel.h"
#include "nodetreedelegate.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QMetaObject>

ModelViewDatabaseConnector::ModelViewDatabaseConnector(NodeTreeView* treeView,
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
            this, &ModelViewDatabaseConnector::loadTreeModel);
    connect(m_treeModel, &NodeTreeModel::topLevelItemLayoutChanged,
            this, &ModelViewDatabaseConnector::updateTreeViewSeparator);
    connect(m_treeView, &NodeTreeView::addFolderRequested,
            this, &ModelViewDatabaseConnector::onAddFolderRequested);
    connect(m_treeDelegate, &NodeTreeDelegate::addFolderRequested,
            this, &ModelViewDatabaseConnector::onAddFolderRequested);
    connect(m_treeDelegate, &NodeTreeDelegate::addTagRequested,
            this, &ModelViewDatabaseConnector::onAddTagRequested);
    connect(m_treeView, &NodeTreeView::renameFolderInDatabase,
            this, &ModelViewDatabaseConnector::onRenameNodeRequestedFromTreeView);
    connect(this, &ModelViewDatabaseConnector::requestRenameNodeInDB,
            m_dbManager, &DBManager::renameNode, Qt::QueuedConnection);
}

void ModelViewDatabaseConnector::updateTreeViewSeparator()
{
    m_treeView->setTreeSeparator(m_treeModel->getSeparatorIndex());
}

void ModelViewDatabaseConnector::loadTreeModel(const NodeTagTreeData &treeData)
{
    m_treeModel->setTreeData(treeData);
    updateTreeViewSeparator();
}

void ModelViewDatabaseConnector::onAddFolderRequested()
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

void ModelViewDatabaseConnector::onAddTagRequested()
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

void ModelViewDatabaseConnector::onRenameNodeRequestedFromTreeView(const QModelIndex &index, const QString &newName)
{
    m_treeModel->setData(index, newName, NodeItem::Roles::DisplayText);
    auto id = index.data(NodeItem::Roles::NodeId).toInt();
    emit requestRenameNodeInDB(id, newName);
}


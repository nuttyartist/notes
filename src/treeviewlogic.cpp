#include "treeviewlogic.h"
#include "nodetreeview.h"
#include "nodetreemodel.h"
#include "nodetreedelegate.h"
#include "notelistview.h"
#include <QDebug>
#include <QMetaObject>
#include <QMessageBox>
#include <QColorDialog>
#include <random>
#include <QApplication>
#include "customapplicationstyle.h"

TreeViewLogic::TreeViewLogic(NodeTreeView *treeView, NodeTreeModel *treeModel, DBManager *dbManager,
                             NoteListView *listView, QObject *parent)
    : QObject(parent),
      m_treeView{ treeView },
      m_treeModel{ treeModel },
      m_listView{ listView },
      m_dbManager{ dbManager },
      m_needLoadSavedState{ false },
      m_isLastSelectFolder{ true },
      m_lastSelectFolder{},
      m_lastSelectTags{},
      m_expandedFolder{}
{
    m_treeDelegate = new NodeTreeDelegate(m_treeView, m_treeView, m_listView);
    m_treeView->setItemDelegate(m_treeDelegate);
    connect(m_dbManager, &DBManager::nodesTagTreeReceived, this, &TreeViewLogic::loadTreeModel,
            Qt::QueuedConnection);
    connect(m_treeModel, &NodeTreeModel::topLevelItemLayoutChanged, this,
            &TreeViewLogic::updateTreeViewSeparator);
    connect(m_treeView, &NodeTreeView::addFolderRequested, this,
            [this] { onAddFolderRequested(false); });
    connect(m_treeDelegate, &NodeTreeDelegate::addFolderRequested, this,
            [this] { onAddFolderRequested(true); });
    connect(m_treeDelegate, &NodeTreeDelegate::addTagRequested, this,
            &TreeViewLogic::onAddTagRequested);
    connect(m_treeView, &NodeTreeView::renameFolderInDatabase, this,
            &TreeViewLogic::onRenameNodeRequestedFromTreeView);
    connect(m_treeView, &NodeTreeView::renameTagInDatabase, this,
            &TreeViewLogic::onRenameTagRequestedFromTreeView);
    connect(this, &TreeViewLogic::requestRenameNodeInDB, m_dbManager, &DBManager::renameNode,
            Qt::QueuedConnection);
    connect(this, &TreeViewLogic::requestRenameTagInDB, m_dbManager, &DBManager::renameTag,
            Qt::QueuedConnection);
    connect(m_treeView, &NodeTreeView::deleteNodeRequested, this,
            &TreeViewLogic::onDeleteFolderRequested);
    connect(m_treeView, &NodeTreeView::changeTagColorRequested, this,
            &TreeViewLogic::onChangeTagColorRequested);
    connect(m_treeView, &NodeTreeView::deleteTagRequested, this,
            &TreeViewLogic::onDeleteTagRequested);
    connect(this, &TreeViewLogic::requestChangeTagColorInDB, m_dbManager,
            &DBManager::changeTagColor, Qt::QueuedConnection);
    connect(this, &TreeViewLogic::requestMoveNodeInDB, m_dbManager, &DBManager::moveNode,
            Qt::QueuedConnection);
    connect(m_treeView, &NodeTreeView::moveNodeRequested, this, [this](int nodeId, int targetId) {
        onMoveNodeRequested(nodeId, targetId);
        emit noteMoved(nodeId, targetId);
    });
    connect(m_treeView, &NodeTreeView::addNoteToTag, this, &TreeViewLogic::addNoteToTag);
    connect(m_treeModel, &NodeTreeModel::requestExpand, m_treeView, &NodeTreeView::onRequestExpand);
    connect(m_treeModel, &NodeTreeModel::requestUpdateAbsPath, m_treeView,
            &NodeTreeView::onUpdateAbsPath);
    connect(m_treeModel, &NodeTreeModel::requestMoveNode, this,
            &TreeViewLogic::onMoveNodeRequested);
    connect(m_treeModel, &NodeTreeModel::requestUpdateNodeRelativePosition, m_dbManager,
            &DBManager::updateRelPosNode, Qt::QueuedConnection);
    connect(m_treeModel, &NodeTreeModel::requestUpdateTagRelativePosition, m_dbManager,
            &DBManager::updateRelPosTag, Qt::QueuedConnection);
    connect(m_treeModel, &NodeTreeModel::dropFolderSuccessful, m_treeView,
            &NodeTreeView::onFolderDropSuccessful);
    connect(m_treeModel, &NodeTreeModel::dropTagsSuccessful, m_treeView,
            &NodeTreeView::onTagsDropSuccessful);
    connect(m_treeModel, &NodeTreeModel::requestMoveFolderToTrash, this,
            &TreeViewLogic::onDeleteFolderRequested);
    connect(m_dbManager, &DBManager::childNotesCountUpdatedFolder, this,
            &TreeViewLogic::onChildNoteCountChangedFolder);
    connect(m_dbManager, &DBManager::childNotesCountUpdatedTag, this,
            &TreeViewLogic::onChildNotesCountChangedTag);
    m_style = new CustomApplicationStyle();
    qApp->setStyle(m_style);
}

void TreeViewLogic::updateTreeViewSeparator()
{
    m_treeView->setTreeSeparator(m_treeModel->getSeparatorIndex(),
                                 m_treeModel->getDefaultNotesIndex());
}

void TreeViewLogic::loadTreeModel(const NodeTagTreeData &treeData)
{
    m_treeModel->setTreeData(treeData);
    {
        NodeData node;
        QMetaObject::invokeMethod(m_dbManager, "getChildNotesCountFolder",
                                  Qt::BlockingQueuedConnection, Q_RETURN_ARG(NodeData, node),
                                  Q_ARG(int, SpecialNodeID::RootFolder));
        auto index = m_treeModel->getAllNotesButtonIndex();
        if (index.isValid()) {
            m_treeModel->setData(index, node.childNotesCount(), NodeItem::Roles::ChildCount);
        }
    }
    {
        NodeData node;
        QMetaObject::invokeMethod(m_dbManager, "getChildNotesCountFolder",
                                  Qt::BlockingQueuedConnection, Q_RETURN_ARG(NodeData, node),
                                  Q_ARG(int, SpecialNodeID::TrashFolder));
        auto index = m_treeModel->getTrashButtonIndex();
        if (index.isValid()) {
            m_treeModel->setData(index, node.childNotesCount(), NodeItem::Roles::ChildCount);
        }
    }
    if (m_needLoadSavedState) {
        m_needLoadSavedState = false;
        m_treeView->reExpandC(m_expandedFolder);
        if (m_isLastSelectFolder) {
            QModelIndex index;
            if (m_lastSelectFolder == NodePath::getAllNoteFolderPath()) {
                index = m_treeModel->getAllNotesButtonIndex();
            } else if (m_lastSelectFolder == NodePath::getTrashFolderPath()) {
                index = m_treeModel->getTrashButtonIndex();
            } else {
                index = m_treeModel->folderIndexFromIdPath(m_lastSelectFolder);
            }
            if (index.isValid()) {
                m_treeView->setCurrentIndexC(index);
            } else {
                m_treeView->setCurrentIndexC(m_treeModel->getAllNotesButtonIndex());
            }
        } else {
            for (const auto &id : qAsConst(m_lastSelectTags)) {
                m_treeView->setCurrentIndexNC(m_treeModel->tagIndexFromId(id));
            }
        }
        m_lastSelectFolder.clear();
        m_lastSelectTags.clear();
        m_expandedFolder.clear();
    } else {
        m_treeView->setCurrentIndexC(m_treeModel->getAllNotesButtonIndex());
    }
    updateTreeViewSeparator();
}

void TreeViewLogic::onAddFolderRequested(bool fromPlusButton)
{
    QModelIndex currentIndex;
    if (fromPlusButton) {
        currentIndex = m_treeView->currentIndex();
    } else {
        currentIndex = m_treeView->currentEditingIndex();
        if (!currentIndex.isValid()) {
            currentIndex = m_treeView->currentIndex();
        }
    }
    int parentId = SpecialNodeID::RootFolder;
    NodeItem::Type currentType = NodeItem::AllNoteButton;
    QString currentAbsPath;
    int currentTagId = SpecialNodeID::InvalidNodeId;
    if (currentIndex.isValid() && !fromPlusButton) {
        auto type =
                static_cast<NodeItem::Type>(currentIndex.data(NodeItem::Roles::ItemType).toInt());
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
        currentType =
                static_cast<NodeItem::Type>(currentIndex.data(NodeItem::Roles::ItemType).toInt());
        if (currentType == NodeItem::FolderItem) {
            currentAbsPath = currentIndex.data(NodeItem::Roles::AbsPath).toString();
        } else if (currentType == NodeItem::TagItem) {
            currentTagId = currentIndex.data(NodeItem::Roles::NodeId).toInt();
        }
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
                              Q_RETURN_ARG(int, newlyCreatedNodeId), Q_ARG(NodeData, newFolder));

    QHash<NodeItem::Roles, QVariant> hs;
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::FolderItem;
    hs[NodeItem::Roles::DisplayText] = newFolder.fullTitle();
    hs[NodeItem::Roles::NodeId] = newlyCreatedNodeId;

    if (parentId != SpecialNodeID::RootFolder) {
        hs[NodeItem::Roles::AbsPath] = currentIndex.data(NodeItem::Roles::AbsPath).toString()
                + PATH_SEPARATOR + QString::number(newlyCreatedNodeId);
        m_treeModel->appendChildNodeToParent(currentIndex, hs);
        if (!m_treeView->isExpanded(currentIndex)) {
            m_treeView->expand(currentIndex);
        }
    } else {
        hs[NodeItem::Roles::AbsPath] = PATH_SEPARATOR + QString::number(SpecialNodeID::RootFolder)
                + PATH_SEPARATOR + QString::number(newlyCreatedNodeId);
        m_treeModel->appendChildNodeToParent(m_treeModel->rootIndex(), hs);
    }
    if (fromPlusButton) {
        if (currentType == NodeItem::FolderItem) {
            currentIndex = m_treeModel->folderIndexFromIdPath(currentAbsPath);
        } else if (currentType == NodeItem::TagItem) {
            currentIndex = m_treeModel->tagIndexFromId(currentTagId);
        } else if (currentType == NodeItem::AllNoteButton) {
            currentIndex = m_treeModel->getAllNotesButtonIndex();
        } else if (currentType == NodeItem::TrashButton) {
            currentIndex = m_treeModel->getTrashButtonIndex();
        } else {
            currentIndex = m_treeModel->getAllNotesButtonIndex();
        }
        m_treeView->setIgnoreThisCurrentLoad(true);
        m_treeView->reExpandC();
        m_treeView->setCurrentIndexC(currentIndex);
        updateTreeViewSeparator();
        m_treeView->setIgnoreThisCurrentLoad(false);
    }
}

void TreeViewLogic::onAddTagRequested()
{
    int newlyCreatedTagId;
    TagData newTag;
    newTag.setName(m_treeModel->getNewTagPlaceholderName());
    // random color generator
    const double lower_bound = 0;
    const double upper_bound = 1;
    static std::uniform_real_distribution<double> unif(lower_bound, upper_bound);
    static std::random_device rand_dev;
    static std::mt19937 rand_engine(rand_dev());
    double rand = unif(rand_engine);
    int h = rand * 359;
    int s = 255;
    int v = 128 + rand * 127;
    auto color = QColor::fromHsv(h, s, v);

    newTag.setColor(color.name());
    QMetaObject::invokeMethod(m_dbManager, "addTag", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(int, newlyCreatedTagId), Q_ARG(TagData, newTag));

    QHash<NodeItem::Roles, QVariant> hs;
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::TagItem;
    hs[NodeItem::Roles::DisplayText] = newTag.name();
    hs[NodeItem::Roles::TagColor] = newTag.color();
    hs[NodeItem::Roles::NodeId] = newlyCreatedTagId;
    m_treeModel->appendChildNodeToParent(m_treeModel->rootIndex(), hs);
}

void TreeViewLogic::onRenameNodeRequestedFromTreeView(const QModelIndex &index,
                                                      const QString &newName)
{
    m_treeModel->setData(index, newName, NodeItem::Roles::DisplayText);
    auto id = index.data(NodeItem::Roles::NodeId).toInt();
    emit requestRenameNodeInDB(id, newName);
}

void TreeViewLogic::onDeleteFolderRequested(const QModelIndex &index)
{
    auto btn = QMessageBox::question(nullptr, "Are you sure you want to delete this folder",
                                     "Are you sure you want to delete this folder? All notes and "
                                     "any subfolders will be deleted.");
    if (btn == QMessageBox::Yes) {
        auto id = index.data(NodeItem::Roles::NodeId).toInt();
        if (id < SpecialNodeID::DefaultNotesFolder) {
            qDebug() << __FUNCTION__ << "Failed while trying to delete folder with id" << id;
            return;
        }
        NodeData node;
        QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(NodeData, node), Q_ARG(int, id));
        auto parentPath = NodePath{ node.absolutePath() }.parentPath();
        auto parentIndex = m_treeModel->folderIndexFromIdPath(parentPath);
        if (parentIndex.isValid()) {
            m_treeModel->deleteRow(index, parentIndex);
            QMetaObject::invokeMethod(m_dbManager, "moveFolderToTrash", Qt::QueuedConnection,
                                      Q_ARG(NodeData, node));
            m_treeView->setCurrentIndexC(m_treeModel->getAllNotesButtonIndex());
        } else {
            qDebug() << __FUNCTION__ << "Parent index with path" << parentPath.path()
                     << "is not valid";
        }
    } else {
        m_treeView->closePersistentEditor(m_treeModel->getTrashButtonIndex());
        m_treeView->update(m_treeModel->getTrashButtonIndex());
    }
}

void TreeViewLogic::onRenameTagRequestedFromTreeView(const QModelIndex &index,
                                                     const QString &newName)
{
    m_treeModel->setData(index, newName, NodeItem::Roles::DisplayText);
    auto id = index.data(NodeItem::Roles::NodeId).toInt();
    emit requestRenameTagInDB(id, newName);
}

void TreeViewLogic::onChangeTagColorRequested(const QModelIndex &index)
{
    if (index.isValid()) {
        auto currentColor = m_treeModel->data(index, NodeItem::Roles::TagColor).toString();
        auto newColor = QColorDialog::getColor(QColor{ currentColor });
        if (newColor.isValid()) {
            m_treeModel->setData(index, newColor.name(), NodeItem::Roles::TagColor);
            auto id = index.data(NodeItem::Roles::NodeId).toInt();
            emit requestChangeTagColorInDB(id, newColor.name());
        }
    }
}

void TreeViewLogic::onDeleteTagRequested(const QModelIndex &index)
{
    auto id = index.data(NodeItem::Roles::NodeId).toInt();
    m_treeModel->deleteRow(index, m_treeModel->rootIndex());
    QMetaObject::invokeMethod(m_dbManager, "removeTag", Qt::QueuedConnection, Q_ARG(int, id));
    m_treeView->setCurrentIndexC(m_treeModel->getAllNotesButtonIndex());
}

void TreeViewLogic::onChildNotesCountChangedTag(int tagId, int notesCount)
{
    auto index = m_treeModel->tagIndexFromId(tagId);
    if (index.isValid()) {
        m_treeModel->setData(index, notesCount, NodeItem::Roles::ChildCount);
    }
}

void TreeViewLogic::onChildNoteCountChangedFolder(int folderId, const QString &absPath,
                                                  int notesCount)
{
    QModelIndex index;
    if (folderId == SpecialNodeID::RootFolder) {
        index = m_treeModel->getAllNotesButtonIndex();
    } else if (folderId == SpecialNodeID::TrashFolder) {
        index = m_treeModel->getTrashButtonIndex();
    } else {
        index = m_treeModel->folderIndexFromIdPath(absPath);
    }
    if (index.isValid()) {
        m_treeModel->setData(index, notesCount, NodeItem::Roles::ChildCount);
    }
}

void TreeViewLogic::openFolder(int id)
{
    NodeData target;
    QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(NodeData, target), Q_ARG(int, id));
    if (target.nodeType() != NodeData::Folder) {
        qDebug() << __FUNCTION__ << "Target is not folder!";
        return;
    }
    if (target.id() == SpecialNodeID::TrashFolder) {
        m_treeView->setCurrentIndexC(m_treeModel->getTrashButtonIndex());
    } else if (target.id() == SpecialNodeID::RootFolder) {
        m_treeView->setCurrentIndexC(m_treeModel->getAllNotesButtonIndex());
    } else {
        auto index = m_treeModel->folderIndexFromIdPath(target.absolutePath());
        if (index.isValid()) {
            m_treeView->setCurrentIndexC(index);
        } else {
            m_treeView->setCurrentIndexC(m_treeModel->getAllNotesButtonIndex());
        }
    }
}

void TreeViewLogic::onMoveNodeRequested(int nodeId, int targetId)
{
    NodeData target;
    QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(NodeData, target), Q_ARG(int, targetId));
    if (target.nodeType() != NodeData::Folder) {
        qDebug() << __FUNCTION__ << "Target is not folder!";
        return;
    }
    emit requestMoveNodeInDB(nodeId, target);
}

void TreeViewLogic::setTheme(Theme::Value theme)
{
    m_treeView->setTheme(theme);
    m_treeDelegate->setTheme(theme);
    m_style->setTheme(theme);
}

void TreeViewLogic::setLastSavedState(bool isLastSelectFolder, const QString &lastSelectFolder,
                                      const QSet<int> &lastSelectTag,
                                      const QStringList &expandedFolder)
{
    m_isLastSelectFolder = isLastSelectFolder;
    m_lastSelectFolder = lastSelectFolder;
    m_lastSelectTags = lastSelectTag;
    m_expandedFolder = expandedFolder;
    m_needLoadSavedState = true;
}

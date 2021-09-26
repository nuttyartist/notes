#include "nodetreemodel.h"
#include <QDebug>

NodeTreeItem::NodeTreeItem(const QHash<NodeItem::Roles, QVariant> &data, NodeTreeItem *parent)
    : m_itemData(data), m_parentItem(parent)
{}

NodeTreeItem::~NodeTreeItem()
{
    qDeleteAll(m_childItems);
}

void NodeTreeItem::appendChild(NodeTreeItem *item)
{
    m_childItems.append(item);
}

NodeTreeItem *NodeTreeItem::child(int row)
{
    if (row < 0 || row >= m_childItems.size()) {
        return nullptr;
    }
    return m_childItems.at(row);
}

int NodeTreeItem::childCount() const
{
    return m_childItems.count();
}

int NodeTreeItem::columnCount() const
{
    return m_itemData.count();
}

QVariant NodeTreeItem::data(NodeItem::Roles role) const
{
    return m_itemData.value(role, QVariant());
}

NodeTreeItem *NodeTreeItem::parentItem()
{
    return m_parentItem;
}

int NodeTreeItem::row() const
{
    if (m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<NodeTreeItem*>(this));
    }

    return 0;
}

NodeTreeModel::NodeTreeModel(QObject *parent) : QAbstractItemModel(parent)
{
    auto hs = QHash<NodeItem::Roles, QVariant>{};
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::RootItem;
    rootItem = new NodeTreeItem(hs);

    {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::AllNoteButton;
        hs[NodeItem::Roles::DisplayText] = tr("All Notes");
        hs[NodeItem::Roles::Icon] = ":/images/trashCan_Regular.png";
        auto allNodeButton = new NodeTreeItem(hs, rootItem);
        rootItem->appendChild(allNodeButton);
    }
    {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::TrashButton;
        hs[NodeItem::Roles::DisplayText] = tr("Trash");
        hs[NodeItem::Roles::Icon] = ":/images/trashCan_Hovered.png";
        auto trashButton = new NodeTreeItem(hs, rootItem);
        rootItem->appendChild(trashButton);
    }
    {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::FolderSeparator;
        hs[NodeItem::Roles::DisplayText] = tr("Folders");
        auto folderSepButton = new NodeTreeItem(hs, rootItem);
        rootItem->appendChild(folderSepButton);
    }
    {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::FolderItem;
        hs[NodeItem::Roles::DisplayText] = tr("Notes");
        auto notesFolder = new NodeTreeItem(hs, rootItem);
        rootItem->appendChild(notesFolder);
    }
    {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::FolderItem;
        hs[NodeItem::Roles::DisplayText] = tr("Poems");
        hs[NodeItem::Roles::Icon] = "";
        auto poemsFolder = new NodeTreeItem(hs, rootItem);
        rootItem->appendChild(poemsFolder);
        hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::FolderItem;
        hs[NodeItem::Roles::DisplayText] = tr("Novels");
        hs[NodeItem::Roles::Icon] = "";
        auto novelsFolder = new NodeTreeItem(hs, poemsFolder);
        poemsFolder->appendChild(novelsFolder);
        hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::NoteItem;
        hs[NodeItem::Roles::DisplayText] = tr("Alice's Adventure");
        hs[NodeItem::Roles::Icon] = "";
        auto aliceNote = new NodeTreeItem(hs, novelsFolder);
        novelsFolder->appendChild(aliceNote);
    }
    {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::TagSeparator;
        hs[NodeItem::Roles::DisplayText] = tr("Tags");
        auto tagSepButton = new NodeTreeItem(hs, rootItem);
        rootItem->appendChild(tagSepButton);
    }
    {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::TagItem;
        hs[NodeItem::Roles::DisplayText] = "Important";
        hs[NodeItem::Roles::TagColor] = "#f75a51";
        auto tagSepButton = new NodeTreeItem(hs, rootItem);
        rootItem->appendChild(tagSepButton);
    }
    {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::TagItem;
        hs[NodeItem::Roles::DisplayText] = "Free-time";
        hs[NodeItem::Roles::TagColor] = "#338df7";
        auto tagSepButton = new NodeTreeItem(hs, rootItem);
        rootItem->appendChild(tagSepButton);
    }
    {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::TagItem;
        hs[NodeItem::Roles::DisplayText] = "Needs-editing";
        hs[NodeItem::Roles::TagColor] = "#f7a233";
        auto tagSepButton = new NodeTreeItem(hs, rootItem);
        rootItem->appendChild(tagSepButton);
    }
    {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::TagItem;
        hs[NodeItem::Roles::DisplayText] = "When-home";
        hs[NodeItem::Roles::TagColor] = "#4bcf5f";
        auto tagSepButton = new NodeTreeItem(hs, rootItem);
        rootItem->appendChild(tagSepButton);
    }
}

NodeTreeModel::~NodeTreeModel()
{
    delete rootItem;
}

QModelIndex NodeTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    NodeTreeItem *parentItem;

    if (!parent.isValid()) {
        parentItem = rootItem;
    } else {
        parentItem = static_cast<NodeTreeItem*>(parent.internalPointer());
    }

    NodeTreeItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex NodeTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    NodeTreeItem *childItem = static_cast<NodeTreeItem*>(index.internalPointer());
    NodeTreeItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int NodeTreeModel::rowCount(const QModelIndex &parent) const
{
    NodeTreeItem *parentItem;
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = rootItem;
    } else {
        parentItem = static_cast<NodeTreeItem*>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int NodeTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return static_cast<NodeTreeItem*>(parent.internalPointer())->columnCount();
    }
    return rootItem->columnCount();
}

QVariant NodeTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    NodeTreeItem *item = static_cast<NodeTreeItem*>(index.internalPointer());
    if (item->data(NodeItem::Roles::ItemType) == NodeItem::Type::RootItem) {
        return QVariant();
    }
    return item->data(static_cast<NodeItem::Roles>(role));
}

void NodeTreeModel::setNodeTree(QVector<NodeData> nodeData)
{
    beginResetModel();
    delete rootItem;
    auto hs = QHash<NodeItem::Roles, QVariant>{};
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::RootItem;
    rootItem = new NodeTreeItem(hs);
    appendAllNotesAndTrashButton(rootItem);
    appendFolderSeparator(rootItem);
    loadNodeTree(nodeData, rootItem);
    appendTagsSeparator(rootItem);
    endResetModel();
}

void NodeTreeModel::loadNodeTree(QVector<NodeData> nodeData, NodeTreeItem *rootNode)
{
    QHash<int, NodeTreeItem *> itemMap;
    itemMap[SpecialNodeID::RootFolder] = rootNode;
    for (const auto& node: nodeData) {
        if (node.id() != SpecialNodeID::RootFolder &&
                node.id() != SpecialNodeID::TrashFolder &&
                node.parentId() != SpecialNodeID::TrashFolder) {
            auto hs = QHash<NodeItem::Roles, QVariant>{};
            if (node.nodeType() == NodeData::Folder) {
                hs[NodeItem::Roles::ItemType] = NodeItem::Type::FolderItem;
            } else if (node.nodeType() == NodeData::Note) {
                hs[NodeItem::Roles::ItemType] = NodeItem::Type::NoteItem;
            } else {
                qDebug() << "Wrong node type";
                continue;
            }
            hs[NodeItem::Roles::DisplayText] = node.fullTitle();
            auto nodeItem = new NodeTreeItem(hs, rootNode);
            itemMap[node.id()] = nodeItem;
        }
    }

    for (const auto& node: nodeData) {
        if (node.id() != SpecialNodeID::RootFolder &&
                node.parentId() != -1 &&
                node.id() != SpecialNodeID::TrashFolder &&
                node.parentId() != SpecialNodeID::TrashFolder) {
            auto parentNode = itemMap.find(node.parentId());
            if (parentNode != itemMap.end()) {
                (*parentNode)->appendChild(itemMap[node.id()]);
            } else {
                qDebug() << "Can't find parent node";
            }
        }
    }
}

void NodeTreeModel::appendAllNotesAndTrashButton(NodeTreeItem *rootNode)
{
    {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::AllNoteButton;
        hs[NodeItem::Roles::DisplayText] = tr("All Notes");
        hs[NodeItem::Roles::Icon] = ":/images/trashCan_Regular.png";
        auto allNodeButton = new NodeTreeItem(hs, rootNode);
        rootNode->appendChild(allNodeButton);
    }
    {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::TrashButton;
        hs[NodeItem::Roles::DisplayText] = tr("Trash");
        hs[NodeItem::Roles::Icon] = ":/images/trashCan_Hovered.png";
        auto trashButton = new NodeTreeItem(hs, rootNode);
        rootNode->appendChild(trashButton);
    }
}

void NodeTreeModel::appendFolderSeparator(NodeTreeItem *rootNode)
{
    auto hs = QHash<NodeItem::Roles, QVariant>{};
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::FolderSeparator;
    hs[NodeItem::Roles::DisplayText] = tr("Folders");
    auto folderSepButton = new NodeTreeItem(hs, rootNode);
    rootNode->appendChild(folderSepButton);
}

void NodeTreeModel::appendTagsSeparator(NodeTreeItem *rootNode)
{
    auto hs = QHash<NodeItem::Roles, QVariant>{};
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::TagSeparator;
    hs[NodeItem::Roles::DisplayText] = tr("Tags");
    auto tagSepButton = new NodeTreeItem(hs, rootNode);
    rootNode->appendChild(tagSepButton);
}

Qt::ItemFlags NodeTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}


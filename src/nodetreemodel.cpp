#include "nodetreemodel.h"
#include <QDebug>
#include <QRegularExpression>

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

void NodeTreeItem::insertChild(int row, NodeTreeItem *child)
{
    m_childItems.insert(row, child);
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
    return 1;
}

QVariant NodeTreeItem::data(NodeItem::Roles role) const
{
    return m_itemData.value(role, QVariant());
}

NodeTreeItem *NodeTreeItem::parentItem()
{
    return m_parentItem;
}

void NodeTreeItem::setParentItem(NodeTreeItem *parentItem)
{
    m_parentItem = parentItem;
}

int NodeTreeItem::row() const
{
    if (m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<NodeTreeItem*>(this));
    }

    return 0;
}

NodeTreeModel::NodeTreeModel(QObject *parent) :
    QAbstractItemModel(parent),
    rootItem(nullptr)
{
    auto hs = QHash<NodeItem::Roles, QVariant>{};
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::RootItem;
    rootItem = new NodeTreeItem(hs);
    //    {
    //        auto hs = QHash<NodeItem::Roles, QVariant>{};
    //        hs[NodeItem::Roles::ItemType] = NodeItem::Type::TagItem;
    //        hs[NodeItem::Roles::DisplayText] = "Important";
    //        hs[NodeItem::Roles::TagColor] = "#f75a51";
    //        auto tagSepButton = new NodeTreeItem(hs, rootItem);
    //        rootItem->appendChild(tagSepButton);
    //    }
    //    {
    //        auto hs = QHash<NodeItem::Roles, QVariant>{};
    //        hs[NodeItem::Roles::ItemType] = NodeItem::Type::TagItem;
    //        hs[NodeItem::Roles::DisplayText] = "Free-time";
    //        hs[NodeItem::Roles::TagColor] = "#338df7";
    //        auto tagSepButton = new NodeTreeItem(hs, rootItem);
    //        rootItem->appendChild(tagSepButton);
    //    }
    //    {
    //        auto hs = QHash<NodeItem::Roles, QVariant>{};
    //        hs[NodeItem::Roles::ItemType] = NodeItem::Type::TagItem;
    //        hs[NodeItem::Roles::DisplayText] = "Needs-editing";
    //        hs[NodeItem::Roles::TagColor] = "#f7a233";
    //        auto tagSepButton = new NodeTreeItem(hs, rootItem);
    //        rootItem->appendChild(tagSepButton);
    //    }
    //    {
    //        auto hs = QHash<NodeItem::Roles, QVariant>{};
    //        hs[NodeItem::Roles::ItemType] = NodeItem::Type::TagItem;
    //        hs[NodeItem::Roles::DisplayText] = "When-home";
    //        hs[NodeItem::Roles::TagColor] = "#4bcf5f";
    //        auto tagSepButton = new NodeTreeItem(hs, rootItem);
    //        rootItem->appendChild(tagSepButton);
    //    }
}

NodeTreeModel::~NodeTreeModel()
{
    delete rootItem;
}

void NodeTreeModel::appendChildNodeToParent(const QModelIndex &parentIndex,
                                            const QHash<NodeItem::Roles, QVariant> &data)
{
    if (rootItem) {
        const auto type = static_cast<NodeItem::Type>(data[NodeItem::Roles::ItemType].toInt());
        if (type == NodeItem::Type::FolderItem) {
            auto parentItem = static_cast<NodeTreeItem*>(parentIndex.internalPointer());
            if (!parentItem || parentItem == rootItem) {
                parentItem = rootItem;
                // need to add folder to folder section
                // we will insert it before tag seperator
                int row = 0;
                for (int i = 0; i < parentItem->childCount(); ++i) {
                    auto childItem = parentItem->child(i);
                    auto childType = static_cast<NodeItem::Type>(childItem->data(NodeItem::Roles::ItemType).toInt());
                    if (childType == NodeItem::Type::TagSeparator) {
                        row = i;
                        break;
                    }
                }
                emit layoutAboutToBeChanged();
                beginInsertRows(parentIndex, row, row);
                auto nodeItem = new NodeTreeItem(data, parentItem);
                parentItem->insertChild(row, nodeItem);
                endInsertRows();
                emit layoutChanged();
                emit topLevelItemLayoutChanged();
            } else {
                beginInsertRows(parentIndex, parentIndex.row(), parentIndex.row());
                auto nodeItem = new NodeTreeItem(data, parentItem);
                parentItem->appendChild(nodeItem);
                endInsertRows();
            }
        } else if (type == NodeItem::Type::TagItem) {
            //tag always in root level
            auto parentItem = static_cast<NodeTreeItem*>(parentIndex.internalPointer());
            if (parentItem != rootItem) {
                qDebug() << "tag only go into root level";
                return;
            }
            beginInsertRows(parentIndex, parentIndex.row(), parentIndex.row() + 1);
            auto nodeItem = new NodeTreeItem(data, parentItem);
            parentItem->appendChild(nodeItem);
            endInsertRows();
        } else {
            qDebug() << "child type not supported with this function";
            return;
        }
    }
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
    if (childItem == rootItem) {
        return QModelIndex();
    }
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

QModelIndex NodeTreeModel::rootIndex() const
{
    return createIndex(0, 0, rootItem);
}

QString NodeTreeModel::getNewFolderPlaceholderName(const QModelIndex &parentIndex)
{
    QString result = "New Folder";
    if (parentIndex.isValid()) {
        auto parentItem = static_cast<NodeTreeItem*>(parentIndex.internalPointer());
        if (parentItem) {
            QRegularExpression reg(R"(^New Folder\s\((\d+)\))");
            int n = 0;
            for (int i = 0; i < parentItem->childCount(); ++i) {
                auto child = parentItem->child(i);
                auto title = child->data(NodeItem::Roles::DisplayText).toString();
                if (title.compare("New Folder", Qt::CaseInsensitive) == 0 && n == 0) {
                    n = 1;
                }
                auto match = reg.match(title);
                if (match.hasMatch()) {
                    auto cn = match.captured(1).toInt();
                    if (n <= cn) {
                        n = cn + 1;
                    }
                }
            }
            if (n != 0) {
                result = QStringLiteral("New Folder (%1)").arg(QString::number(n));
            }
        }
    }
    return result;
}

QVector<QModelIndex> NodeTreeModel::getSeparatorIndex()
{
    QVector<QModelIndex> result;
    if (rootItem) {
        for (int i = 0; i < rootItem->childCount(); ++i) {
            auto child = rootItem->child(i);
            auto type = static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());
            if (type == NodeItem::Type::FolderSeparator || type == NodeItem::Type::TagSeparator) {
                result.append(createIndex(i, 0, child));
            }
        }
    }
    return result;
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
            hs[NodeItem::Roles::NodeId] = node.id();
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
            auto nodeItem = itemMap.find(node.id());
            if (parentNode != itemMap.end() &&
                    nodeItem != itemMap.end()) {
                (*parentNode)->appendChild(*nodeItem);
                (*nodeItem)->setParentItem(*parentNode);
            } else {
                qDebug() << "Can't find node!";
                continue;
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


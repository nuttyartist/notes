#include "nodetreemodel.h"
#include <QDebug>
#include <QRegularExpression>
#include <QMimeData>

NodeTreeItem::NodeTreeItem(const QHash<NodeItem::Roles, QVariant> &data, NodeTreeItem *parent)
    : m_itemData(data), m_parentItem(parent)
{
}

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

void NodeTreeItem::removeChild(int row)
{
    if (row < 0 || row >= m_childItems.size()) {
        return;
    }
    delete m_childItems.takeAt(row);
}

NodeTreeItem *NodeTreeItem::takeChildAt(int row)
{
    if (row < 0 || row >= m_childItems.size()) {
        return nullptr;
    }
    return m_childItems.takeAt(row);
}

int NodeTreeItem::childCount() const
{
    return m_childItems.count();
}

int NodeTreeItem::columnCount() const
{
    return 1;
}

int NodeTreeItem::recursiveNodeCount() const
{
    int res = 1;
    for (const auto &child : m_childItems) {
        res += child->recursiveNodeCount();
    }
    return res;
}

void NodeTreeItem::recursiveUpdateFolderPath(const QString &oldP, const QString &newP)
{
    {
        auto type = static_cast<NodeItem::Type>(data(NodeItem::Roles::ItemType).toInt());
        if (type != NodeItem::Type::FolderItem) {
            return;
        }
        auto currP = data(NodeItem::Roles::AbsPath).toString();
        currP.replace(currP.indexOf(oldP), oldP.size(), newP);
        setData(NodeItem::Roles::AbsPath, currP);
    }
    for (auto &child : m_childItems) {
        child->recursiveUpdateFolderPath(oldP, newP);
    }
}

QVariant NodeTreeItem::data(NodeItem::Roles role) const
{
    return m_itemData.value(role, QVariant());
}

void NodeTreeItem::setData(NodeItem::Roles role, const QVariant &d)
{
    m_itemData[role] = d;
}

NodeTreeItem *NodeTreeItem::parentItem()
{
    return m_parentItem;
}

void NodeTreeItem::setParentItem(NodeTreeItem *parentItem)
{
    m_parentItem = parentItem;
}

void NodeTreeItem::moveChild(int from, int to)
{
    m_childItems.move(from, to);
}

void NodeTreeItem::recursiveSort()
{
    auto type = static_cast<NodeItem::Type>(data(NodeItem::Roles::ItemType).toInt());
    auto relPosComparator = [](const NodeTreeItem *a, const NodeTreeItem *b) {
        return a->data(NodeItem::Roles::RelPos).toInt() < b->data(NodeItem::Roles::RelPos).toInt();
    };
    if (type == NodeItem::Type::FolderItem) {
        std::sort(m_childItems.begin(), m_childItems.end(), relPosComparator);
        for (auto &child : m_childItems) {
            child->recursiveSort();
        }
    } else if (type == NodeItem::Type::RootItem) {
        QVector<NodeTreeItem *> allNoteButton, trashFolder, folderSep, folderItems, tagSep,
                tagItems;
        for (const auto child : qAsConst(m_childItems)) {
            auto childType =
                    static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());
            if (childType == NodeItem::Type::AllNoteButton) {
                allNoteButton.append(child);
            } else if (childType == NodeItem::Type::TrashButton) {
                trashFolder.append(child);
            } else if (childType == NodeItem::Type::FolderSeparator) {
                folderSep.append(child);
            } else if (childType == NodeItem::Type::FolderItem) {
                folderItems.append(child);
            } else if (childType == NodeItem::Type::TagSeparator) {
                tagSep.append(child);
            } else if (childType == NodeItem::Type::TagItem) {
                tagItems.append(child);
            } else {
                qDebug() << __FUNCTION__ << "wrong child type " << static_cast<int>(childType);
            }
        }
        m_childItems.clear();
        std::sort(folderItems.begin(), folderItems.end(), relPosComparator);
        for (auto &child : folderItems) {
            child->recursiveSort();
        }
        std::sort(tagItems.begin(), tagItems.end(), relPosComparator);
        m_childItems.append(allNoteButton);
        m_childItems.append(trashFolder);
        m_childItems.append(folderSep);
        m_childItems.append(folderItems);
        m_childItems.append(tagSep);
        m_childItems.append(tagItems);
    }
}

int NodeTreeItem::row() const
{
    if (m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<NodeTreeItem *>(this));
    }

    return 0;
}

NodeTreeModel::NodeTreeModel(QObject *parent) : QAbstractItemModel(parent), rootItem(nullptr)
{
    auto hs = QHash<NodeItem::Roles, QVariant>{};
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::RootItem;
    rootItem = new NodeTreeItem(hs);
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
            auto parentItem = static_cast<NodeTreeItem *>(parentIndex.internalPointer());
            if (!parentItem || parentItem == rootItem) {
                parentItem = rootItem;
                int row = 0;
                for (int i = 0; i < parentItem->childCount(); ++i) {
                    auto childItem = parentItem->child(i);
                    auto childType = static_cast<NodeItem::Type>(
                            childItem->data(NodeItem::Roles::ItemType).toInt());
                    if (childType == NodeItem::Type::FolderItem
                        && childItem->data(NodeItem::Roles::NodeId).toInt()
                                == SpecialNodeID::DefaultNotesFolder) {
                        row = i + 1;
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
                updateChildRelativePosition(parentItem, NodeItem::Type::FolderItem);
            } else {
                beginInsertRows(parentIndex, 0, 0);
                auto nodeItem = new NodeTreeItem(data, parentItem);
                parentItem->insertChild(0, nodeItem);
                endInsertRows();
                updateChildRelativePosition(parentItem, NodeItem::Type::FolderItem);
            }
        } else if (type == NodeItem::Type::TagItem) {
            // tag always in root level
            auto parentItem = static_cast<NodeTreeItem *>(parentIndex.internalPointer());
            if (parentItem != rootItem) {
                qDebug() << "tag only go into root level";
                return;
            }
            int row = 0;
            for (int i = 0; i < parentItem->childCount(); ++i) {
                auto childItem = parentItem->child(i);
                auto childType = static_cast<NodeItem::Type>(
                        childItem->data(NodeItem::Roles::ItemType).toInt());
                if (childType == NodeItem::Type::TagSeparator) {
                    row = i + 1;
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
            updateChildRelativePosition(parentItem, NodeItem::Type::TagItem);
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
        parentItem = static_cast<NodeTreeItem *>(parent.internalPointer());
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

    NodeTreeItem *childItem = static_cast<NodeTreeItem *>(index.internalPointer());
    if ((!childItem) || (childItem == rootItem)) {
        return QModelIndex();
    }
    NodeTreeItem *parentItem = childItem->parentItem();
    if ((!parentItem) || (parentItem == rootItem)) {
        return QModelIndex();
    }
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
        parentItem = static_cast<NodeTreeItem *>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int NodeTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return static_cast<NodeTreeItem *>(parent.internalPointer())->columnCount();
    }
    return rootItem->columnCount();
}

QVariant NodeTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    NodeTreeItem *item = static_cast<NodeTreeItem *>(index.internalPointer());
    if (static_cast<NodeItem::Roles>(role) == NodeItem::Roles::IsExpandable) {
        return item->childCount() > 0;
    }
    if (item->data(NodeItem::Roles::ItemType) == NodeItem::Type::RootItem) {
        return QVariant();
    }
    return item->data(static_cast<NodeItem::Roles>(role));
}

QModelIndex NodeTreeModel::rootIndex() const
{
    return createIndex(0, 0, rootItem);
}

QModelIndex NodeTreeModel::folderIndexFromIdPath(const NodePath &idPath)
{
    if (!rootItem) {
        return QModelIndex();
    }
    auto ps = idPath.separate();
    auto item = rootItem;
    for (const auto &ite : qAsConst(ps)) {
        bool ok = false;
        auto id = ite.toInt(&ok);
        if (!ok) {
            qDebug() << __FUNCTION__ << "Can't convert to id" << ite;
            return QModelIndex();
        }
        if (id == static_cast<int>(item->data(NodeItem::Roles::NodeId).toInt())) {
            continue;
        }
        bool foundChild = false;
        for (int i = 0; i < item->childCount(); ++i) {
            auto child = item->child(i);
            if (static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt())
                != NodeItem::FolderItem) {
                continue;
            }
            if (id == static_cast<int>(child->data(NodeItem::Roles::NodeId).toInt())) {
                item = child;
                foundChild = true;
                break;
            }
        }
        if (!foundChild) {
            //            qDebug() << __FUNCTION__ << "Can't find child id" << id << "inside parent"
            //                     << static_cast<int>(item->data(NodeItem::Roles::NodeId).toInt());
            return QModelIndex();
        }
    }
    return createIndex(item->row(), 0, item);
}

QModelIndex NodeTreeModel::tagIndexFromId(int id)
{
    for (int i = 0; i < rootItem->childCount(); ++i) {
        auto child = rootItem->child(i);
        if (static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt())
                    == NodeItem::Type::TagItem
            && static_cast<int>(child->data(NodeItem::Roles::NodeId).toInt() == id)) {
            return createIndex(i, 0, child);
        }
    }
    return QModelIndex();
}

QString NodeTreeModel::getNewFolderPlaceholderName(const QModelIndex &parentIndex)
{
    QString result = "New Folder";
    if (parentIndex.isValid()) {
        auto parentItem = static_cast<NodeTreeItem *>(parentIndex.internalPointer());
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

QString NodeTreeModel::getNewTagPlaceholderName()
{
    QString result = "New Tag";
    if (rootItem) {
        QRegularExpression reg(R"(^New Tag\s\((\d+)\))");
        int n = 0;
        for (int i = 0; i < rootItem->childCount(); ++i) {
            auto child = rootItem->child(i);
            auto title = child->data(NodeItem::Roles::DisplayText).toString();
            if (title.compare("New Tag", Qt::CaseInsensitive) == 0 && n == 0) {
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
            result = QStringLiteral("New Tag (%1)").arg(QString::number(n));
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

QModelIndex NodeTreeModel::getDefaultNotesIndex()
{
    if (rootItem) {
        for (int i = 0; i < rootItem->childCount(); ++i) {
            auto child = rootItem->child(i);
            auto type = static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());
            if (type == NodeItem::Type::FolderItem
                && child->data(NodeItem::Roles::NodeId).toInt()
                        == SpecialNodeID::DefaultNotesFolder) {
                return createIndex(i, 0, child);
            }
        }
    }
    return QModelIndex{};
}

QModelIndex NodeTreeModel::getAllNotesButtonIndex()
{
    if (rootItem) {
        for (int i = 0; i < rootItem->childCount(); ++i) {
            auto child = rootItem->child(i);
            auto type = static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());
            if (type == NodeItem::Type::AllNoteButton) {
                return createIndex(i, 0, child);
            }
        }
    }
    return QModelIndex{};
}

QModelIndex NodeTreeModel::getTrashButtonIndex()
{
    if (rootItem) {
        for (int i = 0; i < rootItem->childCount(); ++i) {
            auto child = rootItem->child(i);
            auto type = static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());
            if (type == NodeItem::Type::TrashButton) {
                return createIndex(i, 0, child);
            }
        }
    }
    return QModelIndex{};
}

void NodeTreeModel::deleteRow(const QModelIndex &rowIndex, const QModelIndex &parentIndex)
{
    auto type = static_cast<NodeItem::Type>(rowIndex.data(NodeItem::Roles::ItemType).toInt());
    auto id = rowIndex.data(NodeItem::Roles::NodeId).toInt();
    if (!((type == NodeItem::Type::FolderItem && id > SpecialNodeID::DefaultNotesFolder)
          || type == NodeItem::Type::TagItem)) {
        qDebug() << "Can not delete this row with id" << id;
        return;
    }
    auto parentItem = static_cast<NodeTreeItem *>(parentIndex.internalPointer());
    auto item = static_cast<NodeTreeItem *>(rowIndex.internalPointer());
    int row = item->row();

    setData(rowIndex, "deleted", NodeItem::DisplayText);
    if (parentItem == rootItem) {
        beginResetModel();
        parentItem->removeChild(row);
        endResetModel();
        emit topLevelItemLayoutChanged();
    } else {
        int count = item->recursiveNodeCount();
        beginRemoveRows(parentIndex, row, row + count - 1);
        parentItem->removeChild(row);
        endRemoveRows();
    }
}

void NodeTreeModel::setTreeData(const NodeTagTreeData &treeData)
{
    beginResetModel();
    delete rootItem;
    auto hs = QHash<NodeItem::Roles, QVariant>{};
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::RootItem;
    rootItem = new NodeTreeItem(hs);
    appendAllNotesAndTrashButton(rootItem);
    appendFolderSeparator(rootItem);
    loadNodeTree(treeData.nodeTreeData, rootItem);
    appendTagsSeparator(rootItem);
    loadTagList(treeData.tagTreeData, rootItem);
    rootItem->recursiveSort();
    endResetModel();
}

void NodeTreeModel::loadNodeTree(const QVector<NodeData> &nodeData, NodeTreeItem *rootNode)
{
    QHash<int, NodeTreeItem *> itemMap;
    itemMap[SpecialNodeID::RootFolder] = rootNode;
    for (const auto &node : nodeData) {
        if (node.id() != SpecialNodeID::RootFolder && node.id() != SpecialNodeID::TrashFolder
            && node.parentId() != SpecialNodeID::TrashFolder) {
            auto hs = QHash<NodeItem::Roles, QVariant>{};
            if (node.nodeType() == NodeData::Folder) {
                hs[NodeItem::Roles::ItemType] = NodeItem::Type::FolderItem;
                hs[NodeItem::Roles::AbsPath] = node.absolutePath();
                hs[NodeItem::Roles::RelPos] = node.relativePosition();
                hs[NodeItem::Roles::ChildCount] = node.childNotesCount();
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

    for (const auto &node : nodeData) {
        if (node.id() != SpecialNodeID::RootFolder && node.parentId() != -1
            && node.id() != SpecialNodeID::TrashFolder
            && node.parentId() != SpecialNodeID::TrashFolder) {
            auto parentNode = itemMap.find(node.parentId());
            auto nodeItem = itemMap.find(node.id());
            if (parentNode != itemMap.end() && nodeItem != itemMap.end()) {
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
        hs[NodeItem::Roles::Icon] = u8"\ue2c7"; // folder
        auto allNodeButton = new NodeTreeItem(hs, rootNode);
        rootNode->appendChild(allNodeButton);
    }
    {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::TrashButton;
        hs[NodeItem::Roles::DisplayText] = tr("Trash");
        hs[NodeItem::Roles::Icon] = u8"\uf1f8"; // fa-trash
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

void NodeTreeModel::loadTagList(const QVector<TagData> &tagData, NodeTreeItem *rootNode)
{
    for (const auto &tag : tagData) {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::TagItem;
        hs[NodeItem::Roles::DisplayText] = tag.name();
        hs[NodeItem::Roles::TagColor] = tag.color();
        hs[NodeItem::Roles::NodeId] = tag.id();
        hs[NodeItem::Roles::RelPos] = tag.relativePosition();
        hs[NodeItem::Roles::ChildCount] = tag.childNotesCount();

        auto tagItem = new NodeTreeItem(hs, rootNode);
        rootNode->appendChild(tagItem);
    }
}

void NodeTreeModel::updateChildRelativePosition(NodeTreeItem *parent, const NodeItem::Type type)
{
    int relId = 0;
    for (int i = 0; i < parent->childCount(); ++i) {
        auto child = parent->child(i);
        auto childType =
                static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());
        if (childType == type) {
            if (type == NodeItem::Type::FolderItem) {
                emit requestUpdateNodeRelativePosition(child->data(NodeItem::Roles::NodeId).toInt(),
                                                       relId);
                ++relId;
            } else if (type == NodeItem::Type::TagItem) {
                emit requestUpdateTagRelativePosition(child->data(NodeItem::Roles::NodeId).toInt(),
                                                      relId);
                ++relId;
            } else {
                qDebug() << __FUNCTION__ << "Wrong type";
                return;
            }
        }
    }
}

Qt::ItemFlags NodeTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::ItemIsDropEnabled;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool NodeTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid()) {
        if (role == NodeItem::Roles::DisplayText) {
            static_cast<NodeTreeItem *>(index.internalPointer())
                    ->setData(static_cast<NodeItem::Roles>(role), value);
            emit dataChanged(index, index, { role });
            return true;
        }
        if (role == NodeItem::Roles::TagColor) {
            static_cast<NodeTreeItem *>(index.internalPointer())
                    ->setData(static_cast<NodeItem::Roles>(role), value);
            emit dataChanged(index, index, { role });
            return true;
        }
        if (role == NodeItem::Roles::ChildCount) {
            static_cast<NodeTreeItem *>(index.internalPointer())
                    ->setData(static_cast<NodeItem::Roles>(role), value);
            emit dataChanged(index, index, { role });
            return true;
        }
    }
    return false;
}

Qt::DropActions NodeTreeModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions NodeTreeModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

QStringList NodeTreeModel::mimeTypes() const
{
    return QStringList() << FOLDER_MIME << TAG_MIME;
}

QMimeData *NodeTreeModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty()) {
        return nullptr;
    }
    auto itemType = static_cast<NodeItem::Type>(indexes[0].data(NodeItem::Roles::ItemType).toInt());
    if (itemType == NodeItem::Type::TagItem) {
        QStringList d;
        for (const auto &index : indexes) {
            auto type = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
            if (type != itemType) {
                continue;
            }
            auto id = index.data(NodeItem::Roles::NodeId).toInt();
            d.append(QString::number(id));
        }
        if (d.isEmpty()) {
            return nullptr;
        }
        QMimeData *mimeData = new QMimeData;
        mimeData->setData(TAG_MIME, d.join(QStringLiteral(PATH_SEPARATOR)).toUtf8());
        return mimeData;
    } else if (itemType == NodeItem::Type::FolderItem) {
        const auto &index = indexes[0];
        auto id = index.data(NodeItem::Roles::NodeId).toInt();
        if (id == SpecialNodeID::DefaultNotesFolder) {
            return nullptr;
        }
        auto absPath = index.data(NodeItem::Roles::AbsPath).toString();
        QMimeData *mimeData = new QMimeData;
        mimeData->setData(FOLDER_MIME, absPath.toUtf8());
        return mimeData;
    } else {
        return nullptr;
    }
}

bool NodeTreeModel::dropMimeData(const QMimeData *mime, Qt::DropAction action, int row, int column,
                                 const QModelIndex &parent)
{
    Q_UNUSED(column);
    if (!(mime->hasFormat(TAG_MIME) || mime->hasFormat(FOLDER_MIME)) && action == Qt::MoveAction) {
        return false;
    }
    if (mime->hasFormat(TAG_MIME)) {
        if (row == -1) {
            // valid index: drop onto item
            if (parent.isValid()) {
                row = parent.row();
            } else {
                // invalid index: append at bottom, after last toplevel
                row = rowCount(parent);
            }
        }
        auto sep = getSeparatorIndex();
        if ((sep.size() == 2) && (row <= sep[1].row())) {
            return false;
        }
        auto idl = QString::fromUtf8(mime->data(TAG_MIME)).split(QStringLiteral(PATH_SEPARATOR));
        beginResetModel();
        QSet<int> movedIds;
        for (const auto &id_s : qAsConst(idl)) {
            auto id = id_s.toInt();
            for (int i = 0; i < rootItem->childCount(); ++i) {
                auto child = rootItem->child(i);
                auto childType =
                        static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());
                if (childType == NodeItem::Type::TagItem
                    && child->data(NodeItem::Roles::NodeId).toInt() == id) {
                    if (row >= rootItem->childCount()) {
                        row = rootItem->childCount() - 1;
                    }
                    rootItem->moveChild(i, row);
                    movedIds.insert(id);
                    break;
                }
            }
        }
        endResetModel();
        emit topLevelItemLayoutChanged();
        emit dropTagsSuccessful(movedIds);
        updateChildRelativePosition(rootItem, NodeItem::Type::TagItem);
        return true;
    }
    if (mime->hasFormat(FOLDER_MIME)) {
        if (row == -1) {
            // valid index: drop onto item
            if (parent.isValid()) {
                row = 0;
            } else {
                // invalid index: append at bottom, after last toplevel
                row = rowCount(parent);
            }
        }
        auto absPath = QString::fromUtf8(mime->data(FOLDER_MIME));
        auto index = folderIndexFromIdPath(absPath);
        if (!index.isValid()) {
            return false;
        }
        NodeTreeItem *parentItem, *movingItem;
        if (!parent.isValid()) {
            parentItem = rootItem;
        } else {
            parentItem = static_cast<NodeTreeItem *>(parent.internalPointer());
        }
        auto parentType =
                static_cast<NodeItem::Type>(parentItem->data(NodeItem::Roles::ItemType).toInt());
        if (!(parentType == NodeItem::Type::FolderItem || parentType == NodeItem::Type::RootItem
              || parentType == NodeItem::Type::TrashButton)) {
            return false;
        }
        movingItem = static_cast<NodeTreeItem *>(index.internalPointer());
        if (parentType == NodeItem::Type::TrashButton) {
            auto abs = movingItem->data(NodeItem::Roles::AbsPath).toString();
            auto movingIndex = folderIndexFromIdPath(abs);
            emit requestMoveFolderToTrash(movingIndex);
            return false;
        }
        if (parentType == NodeItem::Type::RootItem) {
            auto sep = getSeparatorIndex();
            if ((sep.size() == 2) && ((row <= sep[0].row()) || (row > sep[1].row()))) {
                return false;
            }
            auto dfNote = getDefaultNotesIndex();
            if (row <= dfNote.row()) {
                return false;
            }
        }
        if (parent.data(NodeItem::Roles::NodeId).toInt() == SpecialNodeID::DefaultNotesFolder) {
            return false;
        }

        if (movingItem->parentItem() == parentItem) {
            beginResetModel();
            for (int i = 0; i < parentItem->childCount(); ++i) {
                auto child = parentItem->child(i);
                auto childType =
                        static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());
                if (childType == NodeItem::Type::FolderItem
                    && child->data(NodeItem::Roles::NodeId)
                            == movingItem->data(NodeItem::Roles::NodeId)) {
                    int targetRow = row;
                    if (row > i && row > 0) {
                        targetRow -= 1;
                    }
                    parentItem->moveChild(i, targetRow);
                    break;
                }
            }
            endResetModel();
            emit topLevelItemLayoutChanged();
            updateChildRelativePosition(parentItem, NodeItem::Type::FolderItem);
            emit dropFolderSuccessful(movingItem->data(NodeItem::Roles::AbsPath).toString());
        } else {
            auto movingParent = movingItem->parentItem();
            int r = -1;
            for (int i = 0; i < movingParent->childCount(); ++i) {
                auto child = movingParent->child(i);
                auto childType =
                        static_cast<NodeItem::Type>(child->data(NodeItem::Roles::ItemType).toInt());
                if ((childType == NodeItem::Type::FolderItem)
                    && (child->data(NodeItem::Roles::NodeId).toInt()
                        == movingItem->data(NodeItem::Roles::NodeId).toInt())) {
                    r = i;
                    break;
                }
            }
            if (r == -1) {
                return false;
            }
            movingItem = movingParent->child(r);
            auto oldAbsolutePath = movingItem->data(NodeItem::Roles::AbsPath).toString();
            QString newAbsolutePath = parentItem->data(NodeItem::Roles::AbsPath).toString()
                    + PATH_SEPARATOR
                    + QString::number(movingItem->data(NodeItem::Roles::NodeId).toInt());
            emit requestUpdateAbsPath(oldAbsolutePath, newAbsolutePath);
            beginResetModel();
            movingParent->takeChildAt(r);
            movingItem->setParentItem(parentItem);
            movingItem->recursiveUpdateFolderPath(oldAbsolutePath, newAbsolutePath);
            parentItem->insertChild(row, movingItem);
            endResetModel();
            emit topLevelItemLayoutChanged();
            emit requestExpand(parentItem->data(NodeItem::Roles::AbsPath).toString());
            emit requestMoveNode(movingItem->data(NodeItem::Roles::NodeId).toInt(),
                                 parentItem->data(NodeItem::Roles::NodeId).toInt());
            updateChildRelativePosition(parentItem, NodeItem::Type::FolderItem);
            emit dropFolderSuccessful(movingItem->data(NodeItem::Roles::AbsPath).toString());
        }
        return true;
    }
    return false;
}

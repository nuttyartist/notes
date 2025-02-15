#include "nodetreemodel.h"
#include <QDebug>
#include <QRegularExpression>
#include <QMimeData>

namespace {
auto constexpr COLUMN_COUNT = 1;
}

NodeTreeItem::NodeTreeItem(const QHash<NodeItem::Roles, QVariant> &data, NodeTreeItem *parentItem) : m_itemData(data), m_parentItem(parentItem) { }

NodeTreeItem::~NodeTreeItem()
{
    qDeleteAll(m_childItems);
}

void NodeTreeItem::appendChild(NodeTreeItem *child)
{
    m_childItems.append(child);
}

void NodeTreeItem::insertChild(int row, NodeTreeItem *child)
{
    m_childItems.insert(row, child);
}

NodeTreeItem *NodeTreeItem::getChild(int row) const
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

int NodeTreeItem::getChildCount() const
{
    return m_childItems.count();
}

int NodeTreeItem::getColumnCount()
{
    return COLUMN_COUNT;
}

int NodeTreeItem::recursiveNodeCount() const
{
    int res = 1;
    for (auto const *child : m_childItems) {
        res += child->recursiveNodeCount();
    }
    return res;
}

void NodeTreeItem::recursiveUpdateFolderPath(const QString &oldP, const QString &newP)
{
    {
        auto type = static_cast<NodeItem::Type>(getData(NodeItem::Roles::ItemType).toInt());
        if (type != NodeItem::Type::FolderItem) {
            return;
        }
        auto currP = getData(NodeItem::Roles::AbsPath).toString();
        currP.replace(currP.indexOf(oldP), oldP.size(), newP);
        setData(NodeItem::Roles::AbsPath, currP);
    }
    for (auto &child : m_childItems) {
        child->recursiveUpdateFolderPath(oldP, newP);
    }
}

QVariant NodeTreeItem::getData(NodeItem::Roles role) const
{
    return m_itemData.value(role, QVariant());
}

void NodeTreeItem::setData(NodeItem::Roles role, const QVariant &d)
{
    m_itemData[role] = d;
}

NodeTreeItem *NodeTreeItem::getParentItem() const
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
    auto type = static_cast<NodeItem::Type>(getData(NodeItem::Roles::ItemType).toInt());
    auto relPosComparator = [](const NodeTreeItem *a, const NodeTreeItem *b) {
        return a->getData(NodeItem::Roles::RelPos).toInt() < b->getData(NodeItem::Roles::RelPos).toInt();
    };
    if (type == NodeItem::Type::FolderItem) {
        std::sort(m_childItems.begin(), m_childItems.end(), relPosComparator);
        for (auto &child : m_childItems) {
            child->recursiveSort();
        }
    } else if (type == NodeItem::Type::RootItem) {
        QVector<NodeTreeItem *> allNoteButton, trashFolder, folderSep, folderItems, tagSep, tagItems;
        for (const auto child : std::as_const(m_childItems)) {
            auto childType = static_cast<NodeItem::Type>(child->getData(NodeItem::Roles::ItemType).toInt());
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

int NodeTreeItem::getRow() const
{
    if (m_parentItem != nullptr) {
        return m_parentItem->m_childItems.indexOf(const_cast<NodeTreeItem *>(this));
    }

    return 0;
}

NodeTreeModel::NodeTreeModel(QObject *parent) : QAbstractItemModel(parent), m_rootItem(nullptr)
{
    auto hs = QHash<NodeItem::Roles, QVariant>{};
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::RootItem;
    m_rootItem = new NodeTreeItem(hs);
}

NodeTreeModel::~NodeTreeModel()
{
    delete m_rootItem;
}

void NodeTreeModel::appendChildNodeToParent(const QModelIndex &parentIndex, const QHash<NodeItem::Roles, QVariant> &data)
{
    if (m_rootItem != nullptr) {
        const auto type = static_cast<NodeItem::Type>(data[NodeItem::Roles::ItemType].toInt());
        if (type == NodeItem::Type::FolderItem) {
            auto *parentItem = static_cast<NodeTreeItem *>(parentIndex.internalPointer());
            if ((parentItem == nullptr) || parentItem == m_rootItem) {
                parentItem = m_rootItem;
                int row = 0;
                for (int i = 0; i < parentItem->getChildCount(); ++i) {
                    auto const *childItem = parentItem->getChild(i);
                    auto childType = static_cast<NodeItem::Type>(childItem->getData(NodeItem::Roles::ItemType).toInt());
                    if (childType == NodeItem::Type::FolderItem && childItem->getData(NodeItem::Roles::NodeId).toInt() == DEFAULT_NOTES_FOLDER_ID) {
                        row = i + 1;
                        break;
                    }
                }
                emit layoutAboutToBeChanged();
                beginInsertRows(parentIndex, row, row);
                auto *nodeItem = new NodeTreeItem(data, parentItem);
                parentItem->insertChild(row, nodeItem);
                endInsertRows();
                emit layoutChanged();
                emit topLevelItemLayoutChanged();
                updateChildRelativePosition(parentItem, NodeItem::Type::FolderItem);
            } else {
                beginInsertRows(parentIndex, 0, 0);
                auto *nodeItem = new NodeTreeItem(data, parentItem);
                parentItem->insertChild(0, nodeItem);
                endInsertRows();
                updateChildRelativePosition(parentItem, NodeItem::Type::FolderItem);
            }
        } else if (type == NodeItem::Type::TagItem) {
            // tag always in root level
            auto *parentItem = static_cast<NodeTreeItem *>(parentIndex.internalPointer());
            if (parentItem != m_rootItem) {
                qDebug() << "tag only go into root level";
                return;
            }
            int row = 0;
            for (int i = 0; i < parentItem->getChildCount(); ++i) {
                auto const *childItem = parentItem->getChild(i);
                auto childType = static_cast<NodeItem::Type>(childItem->getData(NodeItem::Roles::ItemType).toInt());
                if (childType == NodeItem::Type::TagSeparator) {
                    row = i + 1;
                    break;
                }
            }
            emit layoutAboutToBeChanged();
            beginInsertRows(parentIndex, row, row);
            auto *nodeItem = new NodeTreeItem(data, parentItem);
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
        return {};
    }
    NodeTreeItem const *parentItem = parent.isValid() ? static_cast<NodeTreeItem *>(parent.internalPointer()) : m_rootItem;

    NodeTreeItem *childItem = parentItem->getChild(row);
    if (childItem != nullptr) {
        return createIndex(row, column, childItem);
    }
    return {};
}

QModelIndex NodeTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return {};
    }

    auto *childItem = static_cast<NodeTreeItem *>(index.internalPointer());
    if ((childItem == nullptr) || (childItem == m_rootItem)) {
        return {};
    }
    NodeTreeItem *parentItem = childItem->getParentItem();
    if ((parentItem == nullptr) || (parentItem == m_rootItem)) {
        return {};
    }
    return createIndex(parentItem->getRow(), 0, parentItem);
}

int NodeTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    NodeTreeItem const *parentItem = parent.isValid() ? static_cast<NodeTreeItem *>(parent.internalPointer()) : m_rootItem;
    return parentItem->getChildCount();
}

int NodeTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return static_cast<NodeTreeItem *>(parent.internalPointer())->getColumnCount();
    }
    return m_rootItem->getColumnCount();
}

QVariant NodeTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    auto const *item = static_cast<NodeTreeItem *>(index.internalPointer());
    if (static_cast<NodeItem::Roles>(role) == NodeItem::Roles::IsExpandable) {
        return item->getChildCount() > 0;
    }
    if (item->getData(NodeItem::Roles::ItemType) == NodeItem::Type::RootItem) {
        return {};
    }
    return item->getData(static_cast<NodeItem::Roles>(role));
}

QModelIndex NodeTreeModel::rootIndex() const
{
    return createIndex(0, 0, m_rootItem);
}

QModelIndex NodeTreeModel::folderIndexFromIdPath(const NodePath &idPath)
{
    if (m_rootItem == nullptr) {
        return {};
    }
    auto ps = idPath.separate();
    auto const *item = m_rootItem;
    for (const auto &ite : std::as_const(ps)) {
        bool ok = false;
        auto id = ite.toInt(&ok);
        if (!ok) {
            qDebug() << __FUNCTION__ << "Can't convert to id" << ite;
            return {};
        }
        if (id == item->getData(NodeItem::Roles::NodeId).toInt()) {
            continue;
        }
        bool foundChild = false;
        for (int i = 0; i < item->getChildCount(); ++i) {
            auto *child = item->getChild(i);
            if (static_cast<NodeItem::Type>(child->getData(NodeItem::Roles::ItemType).toInt()) != NodeItem::FolderItem) {
                continue;
            }
            if (id == child->getData(NodeItem::Roles::NodeId).toInt()) {
                item = child;
                foundChild = true;
                break;
            }
        }
        if (!foundChild) {
            return {};
        }
    }
    return createIndex(item->getRow(), 0, item);
}

QModelIndex NodeTreeModel::tagIndexFromId(int id)
{
    for (int i = 0; i < m_rootItem->getChildCount(); ++i) {
        auto *child = m_rootItem->getChild(i);
        if (static_cast<NodeItem::Type>(child->getData(NodeItem::Roles::ItemType).toInt()) == NodeItem::Type::TagItem
            && (static_cast<int>(child->getData(NodeItem::Roles::NodeId).toInt() == id) != 0)) {
            return createIndex(i, 0, child);
        }
    }
    return {};
}

QString NodeTreeModel::getNewFolderPlaceholderName(const QModelIndex &parentIndex)
{
    QString result = "New Folder";
    if (parentIndex.isValid()) {
        auto const *parentItem = static_cast<NodeTreeItem *>(parentIndex.internalPointer());
        if (parentItem != nullptr) {
            QRegularExpression reg(R"(^New Folder\s\((\d+)\))");
            int n = 0;
            for (int i = 0; i < parentItem->getChildCount(); ++i) {
                auto const *child = parentItem->getChild(i);
                QString title = child->getData(NodeItem::Roles::DisplayText).toString();
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
    if (m_rootItem != nullptr) {
        QRegularExpression reg(R"(^New Tag\s\((\d+)\))");
        int n = 0;
        for (int i = 0; i < m_rootItem->getChildCount(); ++i) {
            auto const *child = m_rootItem->getChild(i);
            auto title = child->getData(NodeItem::Roles::DisplayText).toString();
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
    if (m_rootItem != nullptr) {
        for (int i = 0; i < m_rootItem->getChildCount(); ++i) {
            auto *child = m_rootItem->getChild(i);
            auto type = static_cast<NodeItem::Type>(child->getData(NodeItem::Roles::ItemType).toInt());
            if (type == NodeItem::Type::FolderSeparator || type == NodeItem::Type::TagSeparator) {
                result.append(createIndex(i, 0, child));
            }
        }
    }
    return result;
}

QModelIndex NodeTreeModel::getDefaultNotesIndex()
{
    if (m_rootItem != nullptr) {
        for (int i = 0; i < m_rootItem->getChildCount(); ++i) {
            auto *child = m_rootItem->getChild(i);
            auto type = static_cast<NodeItem::Type>(child->getData(NodeItem::Roles::ItemType).toInt());
            if (type == NodeItem::Type::FolderItem && child->getData(NodeItem::Roles::NodeId).toInt() == DEFAULT_NOTES_FOLDER_ID) {
                return createIndex(i, 0, child);
            }
        }
    }
    return QModelIndex{};
}

QModelIndex NodeTreeModel::getAllNotesButtonIndex()
{
    if (m_rootItem != nullptr) {
        for (int i = 0; i < m_rootItem->getChildCount(); ++i) {
            auto *child = m_rootItem->getChild(i);
            auto type = static_cast<NodeItem::Type>(child->getData(NodeItem::Roles::ItemType).toInt());
            if (type == NodeItem::Type::AllNoteButton) {
                return createIndex(i, 0, child);
            }
        }
    }
    return QModelIndex{};
}

QModelIndex NodeTreeModel::getTrashButtonIndex()
{
    if (m_rootItem != nullptr) {
        for (int i = 0; i < m_rootItem->getChildCount(); ++i) {
            auto *child = m_rootItem->getChild(i);
            auto type = static_cast<NodeItem::Type>(child->getData(NodeItem::Roles::ItemType).toInt());
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
    if ((type != NodeItem::Type::FolderItem || id <= DEFAULT_NOTES_FOLDER_ID) && type != NodeItem::Type::TagItem) {
        qDebug() << "Can not delete this row with id" << id;
        return;
    }
    auto *parentItem = static_cast<NodeTreeItem *>(parentIndex.internalPointer());
    auto const *item = static_cast<NodeTreeItem *>(rowIndex.internalPointer());
    int row = item->getRow();

    setData(rowIndex, "deleted", NodeItem::DisplayText);
    if (parentItem == m_rootItem) {
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
    delete m_rootItem;
    auto hs = QHash<NodeItem::Roles, QVariant>{};
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::RootItem;
    m_rootItem = new NodeTreeItem(hs);
    appendAllNotesAndTrashButton(m_rootItem);
    appendFolderSeparator(m_rootItem);
    loadNodeTree(treeData.nodeTreeData, m_rootItem);
    appendTagsSeparator(m_rootItem);
    loadTagList(treeData.tagTreeData, m_rootItem);
    m_rootItem->recursiveSort();
    endResetModel();
}

void NodeTreeModel::loadNodeTree(const QVector<NodeData> &nodeData, NodeTreeItem *rootNode)
{
    QHash<int, NodeTreeItem *> itemMap;
    itemMap[ROOT_FOLDER_ID] = rootNode;
    for (const auto &node : nodeData) {
        if (node.id() != ROOT_FOLDER_ID && node.id() != TRASH_FOLDER_ID && node.parentId() != TRASH_FOLDER_ID) {
            auto hs = QHash<NodeItem::Roles, QVariant>{};
            if (node.nodeType() == NodeData::Type::Folder) {
                hs[NodeItem::Roles::ItemType] = NodeItem::Type::FolderItem;
                hs[NodeItem::Roles::AbsPath] = node.absolutePath();
                hs[NodeItem::Roles::RelPos] = node.relativePosition();
                hs[NodeItem::Roles::ChildCount] = node.childNotesCount();
            } else if (node.nodeType() == NodeData::Type::Note) {
                hs[NodeItem::Roles::ItemType] = NodeItem::Type::NoteItem;
            } else {
                qDebug() << "Wrong node type";
                continue;
            }
            hs[NodeItem::Roles::DisplayText] = node.fullTitle();
            hs[NodeItem::Roles::NodeId] = node.id();
            auto *nodeItem = new NodeTreeItem(hs, rootNode);
            itemMap[node.id()] = nodeItem;
        }
    }

    for (const auto &node : nodeData) {
        if (node.id() != ROOT_FOLDER_ID && node.parentId() != -1 && node.id() != TRASH_FOLDER_ID && node.parentId() != TRASH_FOLDER_ID) {
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
        auto *allNodeButton = new NodeTreeItem(hs, rootNode);
        rootNode->appendChild(allNodeButton);
    }
    {
        auto hs = QHash<NodeItem::Roles, QVariant>{};
        hs[NodeItem::Roles::ItemType] = NodeItem::Type::TrashButton;
        hs[NodeItem::Roles::DisplayText] = tr("Trash");
        hs[NodeItem::Roles::Icon] = u8"\uf1f8"; // fa-trash
        auto *trashButton = new NodeTreeItem(hs, rootNode);
        rootNode->appendChild(trashButton);
    }
}

void NodeTreeModel::appendFolderSeparator(NodeTreeItem *rootNode)
{
    auto hs = QHash<NodeItem::Roles, QVariant>{};
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::FolderSeparator;
    hs[NodeItem::Roles::DisplayText] = tr("Folders");
    auto *folderSepButton = new NodeTreeItem(hs, rootNode);
    rootNode->appendChild(folderSepButton);
}

void NodeTreeModel::appendTagsSeparator(NodeTreeItem *rootNode)
{
    auto hs = QHash<NodeItem::Roles, QVariant>{};
    hs[NodeItem::Roles::ItemType] = NodeItem::Type::TagSeparator;
    hs[NodeItem::Roles::DisplayText] = tr("Tags");
    auto *tagSepButton = new NodeTreeItem(hs, rootNode);
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

        auto *tagItem = new NodeTreeItem(hs, rootNode);
        rootNode->appendChild(tagItem);
    }
}

void NodeTreeModel::updateChildRelativePosition(NodeTreeItem *parent, const NodeItem::Type type)
{
    int relId = 0;
    for (int i = 0; i < parent->getChildCount(); ++i) {
        auto const *child = parent->getChild(i);
        auto childType = static_cast<NodeItem::Type>(child->getData(NodeItem::Roles::ItemType).toInt());
        if (childType == type) {
            if (type == NodeItem::Type::FolderItem) {
                emit requestUpdateNodeRelativePosition(child->getData(NodeItem::Roles::NodeId).toInt(), relId);
                ++relId;
            } else if (type == NodeItem::Type::TagItem) {
                emit requestUpdateTagRelativePosition(child->getData(NodeItem::Roles::NodeId).toInt(), relId);
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
            static_cast<NodeTreeItem *>(index.internalPointer())->setData(static_cast<NodeItem::Roles>(role), value);
            emit dataChanged(index, index, { role });
            return true;
        }
        if (role == NodeItem::Roles::TagColor) {
            static_cast<NodeTreeItem *>(index.internalPointer())->setData(static_cast<NodeItem::Roles>(role), value);
            emit dataChanged(index, index, { role });
            return true;
        }
        if (role == NodeItem::Roles::ChildCount) {
            static_cast<NodeTreeItem *>(index.internalPointer())->setData(static_cast<NodeItem::Roles>(role), value);
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
        for (const auto &idx : indexes) {
            auto type = static_cast<NodeItem::Type>(idx.data(NodeItem::Roles::ItemType).toInt());
            if (type != itemType) {
                continue;
            }
            auto id = idx.data(NodeItem::Roles::NodeId).toInt();
            d.append(QString::number(id));
        }
        if (d.isEmpty()) {
            return nullptr;
        }
        auto *qMimeData = new QMimeData;
        qMimeData->setData(TAG_MIME, d.join(PATH_SEPARATOR).toUtf8());
        return qMimeData;
    }
    if (itemType == NodeItem::Type::FolderItem) {
        const auto &idx = indexes[0];
        auto id = idx.data(NodeItem::Roles::NodeId).toInt();
        if (id == DEFAULT_NOTES_FOLDER_ID) {
            return nullptr;
        }
        auto absPath = idx.data(NodeItem::Roles::AbsPath).toString();
        auto *qMimeData = new QMimeData;
        qMimeData->setData(FOLDER_MIME, absPath.toUtf8());
        return qMimeData;
    }
    return nullptr;
}

bool NodeTreeModel::dropMimeData(const QMimeData *mime, Qt::DropAction action, int row, int column, const QModelIndex &parent)
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
        auto idl = QString::fromUtf8(mime->data(TAG_MIME)).split(PATH_SEPARATOR);
        beginResetModel();
        QSet<int> movedIds;
        for (const auto &idString : std::as_const(idl)) {
            auto id = idString.toInt();
            for (int i = 0; i < m_rootItem->getChildCount(); ++i) {
                auto const *child = m_rootItem->getChild(i);
                auto childType = static_cast<NodeItem::Type>(child->getData(NodeItem::Roles::ItemType).toInt());
                if (childType == NodeItem::Type::TagItem && child->getData(NodeItem::Roles::NodeId).toInt() == id) {
                    if (row >= m_rootItem->getChildCount()) {
                        row = m_rootItem->getChildCount() - 1;
                    }
                    m_rootItem->moveChild(i, row);
                    movedIds.insert(id);
                    break;
                }
            }
        }
        endResetModel();
        emit topLevelItemLayoutChanged();
        emit dropTagsSuccessful(movedIds);
        updateChildRelativePosition(m_rootItem, NodeItem::Type::TagItem);
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
        auto idx = folderIndexFromIdPath(absPath);
        if (!idx.isValid()) {
            return false;
        }
        NodeTreeItem *parentItem;
        NodeTreeItem *movingItem;
        if (!parent.isValid()) {
            parentItem = m_rootItem;
        } else {
            parentItem = static_cast<NodeTreeItem *>(parent.internalPointer());
        }
        auto parentType = static_cast<NodeItem::Type>(parentItem->getData(NodeItem::Roles::ItemType).toInt());
        if (parentType != NodeItem::Type::FolderItem && parentType != NodeItem::Type::RootItem && parentType != NodeItem::Type::TrashButton) {
            return false;
        }
        movingItem = static_cast<NodeTreeItem *>(idx.internalPointer());
        if (parentType == NodeItem::Type::TrashButton) {
            auto abs = movingItem->getData(NodeItem::Roles::AbsPath).toString();
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
        if (parent.data(NodeItem::Roles::NodeId).toInt() == DEFAULT_NOTES_FOLDER_ID) {
            return false;
        }

        if (movingItem->getParentItem() == parentItem) {
            beginResetModel();
            for (int i = 0; i < parentItem->getChildCount(); ++i) {
                auto const *child = parentItem->getChild(i);
                auto childType = static_cast<NodeItem::Type>(child->getData(NodeItem::Roles::ItemType).toInt());
                if (childType == NodeItem::Type::FolderItem && child->getData(NodeItem::Roles::NodeId) == movingItem->getData(NodeItem::Roles::NodeId)) {
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
            emit dropFolderSuccessful(movingItem->getData(NodeItem::Roles::AbsPath).toString());
        } else {
            auto *movingParent = movingItem->getParentItem();
            int r = -1;
            for (int i = 0; i < movingParent->getChildCount(); ++i) {
                auto const *child = movingParent->getChild(i);
                auto childType = static_cast<NodeItem::Type>(child->getData(NodeItem::Roles::ItemType).toInt());
                if ((childType == NodeItem::Type::FolderItem)
                    && (child->getData(NodeItem::Roles::NodeId).toInt() == movingItem->getData(NodeItem::Roles::NodeId).toInt())) {
                    r = i;
                    break;
                }
            }
            if (r == -1) {
                return false;
            }
            movingItem = movingParent->getChild(r);
            auto oldAbsolutePath = movingItem->getData(NodeItem::Roles::AbsPath).toString();
            QString newAbsolutePath = parentItem->getData(NodeItem::Roles::AbsPath).toString() + PATH_SEPARATOR
                    + QString::number(movingItem->getData(NodeItem::Roles::NodeId).toInt());
            emit requestUpdateAbsPath(oldAbsolutePath, newAbsolutePath);
            beginResetModel();
            movingParent->takeChildAt(r);
            movingItem->setParentItem(parentItem);
            movingItem->recursiveUpdateFolderPath(oldAbsolutePath, newAbsolutePath);
            parentItem->insertChild(row, movingItem);
            endResetModel();
            emit topLevelItemLayoutChanged();
            emit requestExpand(parentItem->getData(NodeItem::Roles::AbsPath).toString());
            emit requestMoveNode(movingItem->getData(NodeItem::Roles::NodeId).toInt(), parentItem->getData(NodeItem::Roles::NodeId).toInt());
            updateChildRelativePosition(parentItem, NodeItem::Type::FolderItem);
            emit dropFolderSuccessful(movingItem->getData(NodeItem::Roles::AbsPath).toString());
        }
        return true;
    }
    return false;
}

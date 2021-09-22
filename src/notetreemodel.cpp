#include "notetreemodel.h"

NoteTreeItem::NoteTreeItem(const QHash<NoteItem::Roles, QVariant> &data, NoteTreeItem *parent)
    : m_itemData(data), m_parentItem(parent)
{}

NoteTreeItem::~NoteTreeItem()
{
    qDeleteAll(m_childItems);
}

void NoteTreeItem::appendChild(NoteTreeItem *item)
{
    m_childItems.append(item);
}

NoteTreeItem *NoteTreeItem::child(int row)
{
    if (row < 0 || row >= m_childItems.size()) {
        return nullptr;
    }
    return m_childItems.at(row);
}

int NoteTreeItem::childCount() const
{
    return m_childItems.count();
}

int NoteTreeItem::columnCount() const
{
    return m_itemData.count();
}

QVariant NoteTreeItem::data(NoteItem::Roles role) const
{
    return m_itemData.value(role, QVariant());
}

NoteTreeItem *NoteTreeItem::parentItem()
{
    return m_parentItem;
}

int NoteTreeItem::row() const
{
    if (m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<NoteTreeItem*>(this));
    }

    return 0;
}

NoteTreeModel::NoteTreeModel(QObject *parent) : QAbstractItemModel(parent)
{
    auto hs = QHash<NoteItem::Roles, QVariant>{};
    hs[NoteItem::Roles::ItemType] = NoteItem::Type::RootItem;
    rootItem = new NoteTreeItem(hs);

    {
        auto hs = QHash<NoteItem::Roles, QVariant>{};
        hs[NoteItem::Roles::ItemType] = NoteItem::Type::AllNoteButton;
        hs[NoteItem::Roles::DisplayText] = tr("All Notes");
        hs[NoteItem::Roles::Icon] = ":/images/trashCan_Regular.png";
        auto allNodeButton = new NoteTreeItem(hs, rootItem);
        rootItem->appendChild(allNodeButton);
    }
    {
        auto hs = QHash<NoteItem::Roles, QVariant>{};
        hs[NoteItem::Roles::ItemType] = NoteItem::Type::TrashButton;
        hs[NoteItem::Roles::DisplayText] = tr("Trash");
        hs[NoteItem::Roles::Icon] = ":/images/trashCan_Hovered.png";
        auto trashButton = new NoteTreeItem(hs, rootItem);
        rootItem->appendChild(trashButton);
    }
    {
        auto hs = QHash<NoteItem::Roles, QVariant>{};
        hs[NoteItem::Roles::ItemType] = NoteItem::Type::FolderSeparator;
        hs[NoteItem::Roles::DisplayText] = tr("Folders");
        auto folderSepButton = new NoteTreeItem(hs, rootItem);
        rootItem->appendChild(folderSepButton);
    }
    {
        auto hs = QHash<NoteItem::Roles, QVariant>{};
        hs[NoteItem::Roles::ItemType] = NoteItem::Type::FolderItem;
        hs[NoteItem::Roles::DisplayText] = tr("Notes");
        auto notesFolder = new NoteTreeItem(hs, rootItem);
        rootItem->appendChild(notesFolder);
    }
    {
        auto hs = QHash<NoteItem::Roles, QVariant>{};
        hs[NoteItem::Roles::ItemType] = NoteItem::Type::FolderItem;
        hs[NoteItem::Roles::DisplayText] = tr("Poems");
        hs[NoteItem::Roles::Icon] = "";
        auto poemsFolder = new NoteTreeItem(hs, rootItem);
        rootItem->appendChild(poemsFolder);
        hs = QHash<NoteItem::Roles, QVariant>{};
        hs[NoteItem::Roles::ItemType] = NoteItem::Type::FolderItem;
        hs[NoteItem::Roles::DisplayText] = tr("Novels");
        hs[NoteItem::Roles::Icon] = "";
        auto novelsFolder = new NoteTreeItem(hs, poemsFolder);
        poemsFolder->appendChild(novelsFolder);
        hs = QHash<NoteItem::Roles, QVariant>{};
        hs[NoteItem::Roles::ItemType] = NoteItem::Type::NoteItem;
        hs[NoteItem::Roles::DisplayText] = tr("Alice's Adventure");
        hs[NoteItem::Roles::Icon] = "";
        auto aliceNote = new NoteTreeItem(hs, novelsFolder);
        novelsFolder->appendChild(aliceNote);
    }
    {
        auto hs = QHash<NoteItem::Roles, QVariant>{};
        hs[NoteItem::Roles::ItemType] = NoteItem::Type::TagSeparator;
        hs[NoteItem::Roles::DisplayText] = tr("Tags");        
        auto tagSepButton = new NoteTreeItem(hs, rootItem);
        rootItem->appendChild(tagSepButton);
    }
    {
        auto hs = QHash<NoteItem::Roles, QVariant>{};
        hs[NoteItem::Roles::ItemType] = NoteItem::Type::TagItem;
        hs[NoteItem::Roles::DisplayText] = "Important";
        hs[NoteItem::Roles::TagColor] = "#f75a51";
        auto tagSepButton = new NoteTreeItem(hs, rootItem);
        rootItem->appendChild(tagSepButton);
    }
    {
        auto hs = QHash<NoteItem::Roles, QVariant>{};
        hs[NoteItem::Roles::ItemType] = NoteItem::Type::TagItem;
        hs[NoteItem::Roles::DisplayText] = "Free-time";
        hs[NoteItem::Roles::TagColor] = "#338df7";
        auto tagSepButton = new NoteTreeItem(hs, rootItem);
        rootItem->appendChild(tagSepButton);
    }
    {
        auto hs = QHash<NoteItem::Roles, QVariant>{};
        hs[NoteItem::Roles::ItemType] = NoteItem::Type::TagItem;
        hs[NoteItem::Roles::DisplayText] = "Needs-editing";
        hs[NoteItem::Roles::TagColor] = "#f7a233";
        auto tagSepButton = new NoteTreeItem(hs, rootItem);
        rootItem->appendChild(tagSepButton);
    }
    {
        auto hs = QHash<NoteItem::Roles, QVariant>{};
        hs[NoteItem::Roles::ItemType] = NoteItem::Type::TagItem;
        hs[NoteItem::Roles::DisplayText] = "When-home";
        hs[NoteItem::Roles::TagColor] = "#4bcf5f";
        auto tagSepButton = new NoteTreeItem(hs, rootItem);
        rootItem->appendChild(tagSepButton);
    }
}

NoteTreeModel::~NoteTreeModel()
{
    delete rootItem;
}

QModelIndex NoteTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    NoteTreeItem *parentItem;

    if (!parent.isValid()) {
        parentItem = rootItem;
    } else {
        parentItem = static_cast<NoteTreeItem*>(parent.internalPointer());
    }

    NoteTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex NoteTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    NoteTreeItem *childItem = static_cast<NoteTreeItem*>(index.internalPointer());
    NoteTreeItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int NoteTreeModel::rowCount(const QModelIndex &parent) const
{
    NoteTreeItem *parentItem;
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<NoteTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int NoteTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return static_cast<NoteTreeItem*>(parent.internalPointer())->columnCount();
    }
    return rootItem->columnCount();
}

QVariant NoteTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    NoteTreeItem *item = static_cast<NoteTreeItem*>(index.internalPointer());
    if (item->data(NoteItem::Roles::ItemType) == NoteItem::Type::RootItem) {
        return QVariant();
    }
    return item->data(static_cast<NoteItem::Roles>(role));
}

Qt::ItemFlags NoteTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}


#ifndef NOTETREEMODEL_H
#define NOTETREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QHash>

namespace NoteItem {
    enum Roles {
        ItemType = Qt::UserRole,
        DisplayText = Qt::DisplayRole,
        Icon = Qt::DecorationRole,
        TagColor = Qt::UserRole + 1
    };
    enum Type {
        AllNoteButton = 1,
        // We store this enum inside QVariant,
        // and an invalid QVariant conversion return 0
        TrashButton,
        FolderSeparator,
        TagSeparator,
        FolderItem,
        NoteItem,
        TagItem,
        RootItem
    };
};

class NoteTreeItem {
public:
    explicit NoteTreeItem(const QHash<NoteItem::Roles, QVariant> &data, NoteTreeItem *parentItem = nullptr);
    ~NoteTreeItem();

    void appendChild(NoteTreeItem *child);

    NoteTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(NoteItem::Roles role) const;
    int row() const;
    NoteTreeItem *parentItem();

private:
    QVector<NoteTreeItem*> m_childItems;
    QHash<NoteItem::Roles, QVariant> m_itemData;
    NoteTreeItem *m_parentItem;
};

class NoteTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit NoteTreeModel(QObject *parent = nullptr);
    ~NoteTreeModel();
signals:


    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &index) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
private:
    NoteTreeItem *rootItem;
    void setupModelData(const QStringList &lines, NoteTreeItem *parent);

    // QAbstractItemModel interface
public:
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
};

#endif // NOTETREEMODEL_H

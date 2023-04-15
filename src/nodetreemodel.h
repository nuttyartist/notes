#ifndef NODETREEMODEL_H
#define NODETREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QHash>
#include <QVector>

#include "nodedata.h"
#include "dbmanager.h"
#include "nodepath.h"

namespace NodeItem {
enum Roles {
    ItemType = Qt::UserRole,
    DisplayText = Qt::DisplayRole,
    Icon = Qt::DecorationRole,
    TagColor = Qt::UserRole + 1,
    IsExpandable,
    AbsPath,
    RelPos,
    ChildCount,
    NodeId
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
} // namespace NodeItem

class NodeTreeItem
{
public:
    explicit NodeTreeItem(const QHash<NodeItem::Roles, QVariant> &data,
                          NodeTreeItem *parentItem = nullptr);
    ~NodeTreeItem();

    void appendChild(NodeTreeItem *child);
    void insertChild(int row, NodeTreeItem *child);
    NodeTreeItem *child(int row);
    void removeChild(int row);
    NodeTreeItem *takeChildAt(int row);
    int childCount() const;
    int columnCount() const;
    int recursiveNodeCount() const;
    void recursiveUpdateFolderPath(const QString &oldP, const QString &newP);
    QVariant data(NodeItem::Roles role) const;
    void setData(NodeItem::Roles role, const QVariant &d);
    int row() const;
    NodeTreeItem *parentItem();
    void setParentItem(NodeTreeItem *parentItem);
    void moveChild(int from, int to);
    void recursiveSort();

private:
    QVector<NodeTreeItem *> m_childItems;
    QHash<NodeItem::Roles, QVariant> m_itemData;
    NodeTreeItem *m_parentItem;
};

class NodeTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit NodeTreeModel(QObject *parent = nullptr);
    ~NodeTreeModel();

    void appendChildNodeToParent(const QModelIndex &parentIndex,
                                 const QHash<NodeItem::Roles, QVariant> &data);
    QModelIndex rootIndex() const;
    QModelIndex folderIndexFromIdPath(const NodePath &idPath);
    QModelIndex tagIndexFromId(int id);
    QString getNewFolderPlaceholderName(const QModelIndex &parentIndex);
    QString getNewTagPlaceholderName();
    QVector<QModelIndex> getSeparatorIndex();
    QModelIndex getDefaultNotesIndex();
    QModelIndex getAllNotesButtonIndex();
    QModelIndex getTrashButtonIndex();
    void deleteRow(const QModelIndex &rowIndex, const QModelIndex &parentIndex);

public slots:
    void setTreeData(const NodeTagTreeData &treeData);

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &index) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual Qt::DropActions supportedDropActions() const override;
    virtual Qt::DropActions supportedDragActions() const override;
    virtual QStringList mimeTypes() const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
    virtual bool dropMimeData(const QMimeData *mime, Qt::DropAction action, int row, int column,
                              const QModelIndex &parent) override;

signals:
    void topLevelItemLayoutChanged();
    void requestExpand(const QString &indexPath);
    void requestMoveNode(int nodeId, int targetId);
    void requestUpdateNodeRelativePosition(int nodeId, int relativePosition);
    void requestUpdateTagRelativePosition(int nodeId, int relativePosition);
    void requestUpdateAbsPath(const QString &oldPath, const QString &newPath);
    void dropFolderSuccessful(const QString &paths);
    void dropTagsSuccessful(const QSet<int> &ids);
    void requestMoveFolderToTrash(const QModelIndex &index);

private:
    NodeTreeItem *rootItem;
    void loadNodeTree(const QVector<NodeData> &nodeData, NodeTreeItem *rootNode);
    void appendAllNotesAndTrashButton(NodeTreeItem *rootNode);
    void appendFolderSeparator(NodeTreeItem *rootNode);
    void appendTagsSeparator(NodeTreeItem *rootNode);
    void loadTagList(const QVector<TagData> &tagData, NodeTreeItem *rootNode);
    void updateChildRelativePosition(NodeTreeItem *parent, const NodeItem::Type type);
};

#endif // NODETREEMODEL_H

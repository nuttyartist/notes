#include "nodetreeview.h"
#include "nodetreemodel.h"
#include <QMenu>
#include <QAction>

NodeTreeView::NodeTreeView(QWidget *parent) : QTreeView(parent)
{
    setHeaderHidden(true);
    setStyleSheet(R"(QTreeView {)"
                 R"(   border-style: none;)"
                 R"(   background-color: rgb(255, 255, 255);)"
                 R"(   selection-background-color: white;)"
                 R"(   selection-color: white;)"
                 R"(})"
                 R"()"
                 R"(QTreeView::branch{)"
                 R"(   border-image: url(none.png);)"
                 R"(})"
                );
    setRootIsDecorated(false);
    QObject::connect(
                this, SIGNAL(clicked(const QModelIndex &)),
                this, SLOT(onClicked(const QModelIndex &))
                );
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(onCustomContextMenu(const QPoint &)));
    contextMenu = new QMenu(this);
    QAction* addFolderAction = new QAction(tr("Add Subfolder"), this);
    connect(addFolderAction, &QAction::triggered, this, &NodeTreeView::addFolderRequested);
    contextMenu->addAction(addFolderAction);
}

void NodeTreeView::onClicked(const QModelIndex &index)
{
    auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
    switch (itemType) {
    case NodeItem::Type::RootItem:
    case NodeItem::Type::FolderSeparator:
    case NodeItem::Type::TagSeparator: {
        break;
    }
    case NodeItem::Type::AllNoteButton: {
        emit loadNotesRequested(SpecialNodeID::RootFolder, true);
        break;
    }
    case NodeItem::Type::TrashButton: {
        emit loadNotesRequested(SpecialNodeID::TrashFolder, true);
        break;
    }
    case NodeItem::Type::FolderItem: {
        expand(index);
        auto folderId = index.data(NodeItem::Roles::NodeId).toInt();
        emit loadNotesRequested(folderId, false);
        break;
    }
    case NodeItem::Type::NoteItem: {
        break;
    }
    case NodeItem::Type::TagItem: {
        break;
    }
    }
}

void NodeTreeView::onCustomContextMenu(const QPoint &point)
{
    QModelIndex index = indexAt(point);
    if (index.isValid()) {
        auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
        if (itemType == NodeItem::Type::FolderItem) {
            contextMenu->exec(viewport()->mapToGlobal(point));
        }
    }
}

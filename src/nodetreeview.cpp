#include "nodetreeview.h"
#include "nodetreemodel.h"

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
        break;
    }
    case NodeItem::Type::TrashButton: {
        break;
    }
    case NodeItem::Type::FolderItem: {
        expand(index);
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

#include "nodetreeview.h"
#include "nodetreemodel.h"
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QDebug>

NodeTreeView::NodeTreeView(QWidget *parent) :
    QTreeView(parent),
    m_isContextMenuOpened{false}
{
    setHeaderHidden(true);
    setStyleSheet(
                R"(QTreeView {)"
                R"(    border-style: none;)"
                R"(    background-color: rgb(255, 255, 255);)"
                R"(    selection-background-color: white;)"
                R"(    selection-color: white;)"
                R"(})"
                R"()"
                R"(QTreeView::branch{)"
                R"(    border-image: url(none.png);)"
                R"(})"
                R"(QScrollBar::handle:vertical:hover { background: rgb(170, 170, 171); } )"
                R"(QScrollBar::handle:vertical:pressed { background: rgb(149, 149, 149); } )"
                R"(QScrollBar::handle:vertical { border-radius: 4px; background: rgb(188, 188, 188); min-height: 20px; }  )"
                R"(QScrollBar::vertical {border-radius: 4px; width: 8px; color: rgba(255, 255, 255,0);} )"
                R"(QScrollBar {margin: 0; background: transparent;} )"
                R"(QScrollBar:hover { background-color: rgb(217, 217, 217);})"
                R"(QScrollBar::add-line:vertical { width:0px; height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  )"
                R"(QScrollBar::sub-line:vertical { width:0px; height: 0px; subcontrol-position: top; subcontrol-origin: margin; })"
                );
    setRootIsDecorated(false);
    setMouseTracking(true);
    connect(this, &QAbstractItemView::clicked, this, &NodeTreeView::onClicked);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested,
            this, &NodeTreeView::onCustomContextMenu);
    contextMenu = new QMenu(this);
    renameFolderAction = new QAction(tr("Rename Folder"), this);
    connect(renameFolderAction, &QAction::triggered, this, &NodeTreeView::renameFolderRequested);
    deleteFolderAction = new QAction(tr("Delete Folder"), this);
    connect(deleteFolderAction, &QAction::triggered, this, &NodeTreeView::deleteFolderRequested);
    addSubfolderAction = new QAction(tr("Add Subfolder"), this);
    connect(addSubfolderAction, &QAction::triggered, this, &NodeTreeView::addFolderRequested);
    connect(contextMenu, &QMenu::aboutToHide, this, [this] {
        m_isContextMenuOpened = false;
        closePersistentEditor(m_currentHoveringIndex);
        m_currentHoveringIndex = QModelIndex();
    });
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
        contextMenu->clear();
        if (itemType == NodeItem::Type::FolderItem) {
            auto id = index.data(NodeItem::Roles::NodeId).toInt();
            if (id != SpecialNodeID::DefaultNotesFolder) {
                m_isContextMenuOpened = true;
                contextMenu->addAction(renameFolderAction);
                contextMenu->addAction(deleteFolderAction);
                contextMenu->addSeparator();
                contextMenu->addAction(addSubfolderAction);
                contextMenu->exec(viewport()->mapToGlobal(point));
            }
        }
    }
}

void NodeTreeView::setTreeSeparator(const QVector<QModelIndex> &newTreeSeparator)
{
    for (const auto& sep : m_treeSeparator) {
        closePersistentEditor(sep);
    }
    m_treeSeparator = newTreeSeparator;
    for (const auto& sep : m_treeSeparator) {
        openPersistentEditor(sep);
    }
}

void NodeTreeView::mouseMoveEvent(QMouseEvent *event)
{
    auto index = indexAt(event->pos());
    if(indexAt(event->pos()) != m_currentHoveringIndex && !m_isContextMenuOpened) {
        auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
        if (itemType == NodeItem::Type::FolderItem || itemType == NodeItem::Type::NoteItem) {
            auto id = index.data(NodeItem::Roles::NodeId).toInt();
            if (id != SpecialNodeID::DefaultNotesFolder) {
                closePersistentEditor(m_currentHoveringIndex);
                openPersistentEditor(index);
                m_currentHoveringIndex = index;
            }
        } else {
            closePersistentEditor(m_currentHoveringIndex);
            m_currentHoveringIndex = QModelIndex();
        }
    }
    QTreeView::mouseMoveEvent(event);
}

void NodeTreeView::leaveEvent(QEvent *event)
{
    if (!m_isContextMenuOpened) {
        closePersistentEditor(m_currentHoveringIndex);
        m_currentHoveringIndex = QModelIndex();
        QTreeView::leaveEvent(event);
    }
}

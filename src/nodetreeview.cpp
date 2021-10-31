#include "nodetreeview.h"
#include "nodetreemodel.h"
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QDebug>

NodeTreeView::NodeTreeView(QWidget *parent) :
    QTreeView(parent),
    m_isContextMenuOpened{false},
    m_isEditing{false}
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
    connect(renameFolderAction, &QAction::triggered, this, [this] {
        setIsEditing(true);
        emit renameFolderRequested();
    });
    deleteFolderAction = new QAction(tr("Delete Folder"), this);
    connect(deleteFolderAction, &QAction::triggered, this, &NodeTreeView::onDeleteNodeAction);
    addSubfolderAction = new QAction(tr("Add Subfolder"), this);
    connect(addSubfolderAction, &QAction::triggered, this, &NodeTreeView::addFolderRequested);
    contextMenuTimer.setInterval(100);
    contextMenuTimer.setSingleShot(true);
    connect(&contextMenuTimer, &QTimer::timeout, this, [this] {
        if (!m_isEditing) {
            closeCurrentEditor();
        }
    });

    connect(contextMenu, &QMenu::aboutToHide, this, [this] {
        m_isContextMenuOpened = false;
        // this signal is emmited before QAction::triggered
        contextMenuTimer.start();
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
        emit loadNotesInFolderRequested(SpecialNodeID::RootFolder, true);
        break;
    }
    case NodeItem::Type::TrashButton: {
        emit loadNotesInFolderRequested(SpecialNodeID::TrashFolder, true);
        break;
    }
    case NodeItem::Type::FolderItem: {
        expand(index);
        auto folderId = index.data(NodeItem::Roles::NodeId).toInt();
        emit loadNotesInFolderRequested(folderId, false);
        break;
    }
    case NodeItem::Type::NoteItem: {
        break;
    }
    case NodeItem::Type::TagItem: {
        auto tagId = index.data(NodeItem::Roles::NodeId).toInt();
        emit loadNotesInTagRequested(tagId);
        break;
    }
    }
}

void NodeTreeView::onDeleteNodeAction()
{
    auto itemType = static_cast<NodeItem::Type>(m_currentEditingIndex.data(NodeItem::Roles::ItemType).toInt());
    auto id = m_currentEditingIndex.data(NodeItem::Roles::NodeId).toInt();
    if (itemType == NodeItem::Type::FolderItem || itemType == NodeItem::Type::NoteItem) {
        if (id > SpecialNodeID::DefaultNotesFolder) {
            auto index = m_currentEditingIndex;
            emit deleteNodeRequested(index);
        }
    }
}

void NodeTreeView::updateEditingIndex(QMouseEvent *event)
{
    auto index = indexAt(event->pos());
    if(indexAt(event->pos()) != m_currentEditingIndex && !m_isContextMenuOpened && !m_isEditing) {
        auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
        auto id = index.data(NodeItem::Roles::NodeId).toInt();
        if ((itemType == NodeItem::Type::FolderItem
             || itemType == NodeItem::Type::NoteItem)
                && id != SpecialNodeID::DefaultNotesFolder) {
            closePersistentEditor(m_currentEditingIndex);
            openPersistentEditor(index);
            m_currentEditingIndex = index;
        } else {
            closeCurrentEditor();
        }
    }
}


void NodeTreeView::closeCurrentEditor()
{
    closePersistentEditor(m_currentEditingIndex);
    m_currentEditingIndex = QModelIndex();
}

void NodeTreeView::setIsEditing(bool newIsEditing)
{
    m_isEditing = newIsEditing;
}

void NodeTreeView::onRenameFolderFinished(const QString &newName)
{
    if (m_currentEditingIndex.isValid()) {
        auto itemType = static_cast<NodeItem::Type>(m_currentEditingIndex.data(NodeItem::Roles::ItemType).toInt());
        if (itemType == NodeItem::Type::FolderItem) {
            QModelIndex index = m_currentEditingIndex;
            closeCurrentEditor();
            emit renameFolderInDatabase(index, newName);
        } else {
            qDebug() << __FUNCTION__ << "wrong type";
        }
    } else {
        qDebug() << __FUNCTION__ << "m_currentEditingIndex is not valid";
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
    updateEditingIndex(event);
    QTreeView::mouseMoveEvent(event);
}

void NodeTreeView::mousePressEvent(QMouseEvent *event)
{
    updateEditingIndex(event);
    QTreeView::mousePressEvent(event);
}

void NodeTreeView::leaveEvent(QEvent *event)
{
    if (!m_isContextMenuOpened && !m_isEditing) {
        closeCurrentEditor();
        QTreeView::leaveEvent(event);
    }
}
#include "nodetreeview.h"
#include "nodetreemodel.h"
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QDebug>
#include <QMimeData>
#include <QApplication>
#include <QFile>
#include <QScrollBar>
#include "nodetreeview_p.h"

NodeTreeView::NodeTreeView(QWidget *parent)
    : QTreeView(parent), m_isContextMenuOpened{ false }, m_isEditing{ false }, m_ignoreThisCurrentLoad{ false }, m_isLastSelectedFolder{ false }
{
    setHeaderHidden(true);

    QFile file(":/styles/nodetreeview.css");
    file.open(QFile::ReadOnly);
    setStyleSheet(file.readAll());

#if (defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)) || defined(Q_OS_WIN) || defined(Q_OS_WINDOWS)
    QFile scollBarStyleFile(QStringLiteral(":/styles/components/custom-scrollbar.css"));
    scollBarStyleFile.open(QFile::ReadOnly);
    QString scrollbarStyleSheet = QString::fromLatin1(scollBarStyleFile.readAll());
    verticalScrollBar()->setStyleSheet(scrollbarStyleSheet);
#endif

    setRootIsDecorated(false);
    setMouseTracking(true);
    setSelectionMode(QAbstractItemView::MultiSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &NodeTreeView::onCustomContextMenu);
    m_contextMenu = new QMenu(this);
    m_renameFolderAction = new QAction(tr("Rename Folder"), this);
    connect(m_renameFolderAction, &QAction::triggered, this, [this] {
        setIsEditing(true);
        emit renameFolderRequested();
    });
    m_deleteFolderAction = new QAction(tr("Delete Folder"), this);
    connect(m_deleteFolderAction, &QAction::triggered, this, &NodeTreeView::onDeleteNodeAction);
    m_addSubfolderAction = new QAction(tr("Add Subfolder"), this);
    connect(m_addSubfolderAction, &QAction::triggered, this, &NodeTreeView::addFolderRequested);

    m_renameTagAction = new QAction(tr("Rename Tag"), this);
    connect(m_renameTagAction, &QAction::triggered, this, [this] {
        setIsEditing(true);
        emit renameTagRequested();
    });
    m_changeTagColorAction = new QAction(tr("Change Tag Color"), this);
    connect(m_changeTagColorAction, &QAction::triggered, this, &NodeTreeView::onChangeTagColorAction);
    m_deleteTagAction = new QAction(tr("Delete Tag"), this);
    connect(m_deleteTagAction, &QAction::triggered, this, &NodeTreeView::onDeleteNodeAction);
    m_clearSelectionAction = new QAction(tr("Clear Selection"), this);
    connect(m_clearSelectionAction, &QAction::triggered, this, [this] {
        closeCurrentEditor();
        clearSelection();
        setCurrentIndexC(static_cast<NodeTreeModel *>(model())->getAllNotesButtonIndex());
    });

    m_contextMenuTimer.setInterval(100);
    m_contextMenuTimer.setSingleShot(true);
    connect(&m_contextMenuTimer, &QTimer::timeout, this, [this] {
        if (!m_isEditing) {
            closeCurrentEditor();
        }
    });

    connect(m_contextMenu, &QMenu::aboutToHide, this, [this] {
        m_isContextMenuOpened = false;
        // this signal is emitted before QAction::triggered
        m_contextMenuTimer.start();
    });
    connect(this, &NodeTreeView::expanded, this, &NodeTreeView::onExpanded);
    connect(this, &NodeTreeView::collapsed, this, &NodeTreeView::onCollapsed);

    setDragEnabled(true);
    setAcceptDrops(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void NodeTreeView::onDeleteNodeAction()
{
    auto itemType = static_cast<NodeItem::Type>(m_currentEditingIndex.data(NodeItem::Roles::ItemType).toInt());
    auto id = m_currentEditingIndex.data(NodeItem::Roles::NodeId).toInt();
    if (itemType == NodeItem::Type::FolderItem || itemType == NodeItem::Type::NoteItem) {
        if (id > DEFAULT_NOTES_FOLDER_ID) {
            auto index = m_currentEditingIndex;
            emit deleteNodeRequested(index);
        }
    } else if (itemType == NodeItem::Type::TagItem) {
        auto index = m_currentEditingIndex;
        emit deleteTagRequested(index);
    }
}

void NodeTreeView::onExpanded(const QModelIndex &index)
{
    m_expanded.push_back(index.data(NodeItem::Roles::AbsPath).toString());
    emit saveExpand(QStringList::fromVector(m_expanded));
}

void NodeTreeView::onCollapsed(const QModelIndex &index)
{
    m_expanded.removeAll(index.data(NodeItem::Roles::AbsPath).toString());
    emit saveExpand(QStringList::fromVector(m_expanded));
}

const QModelIndex &NodeTreeView::currentEditingIndex() const
{
    return m_currentEditingIndex;
}

void NodeTreeView::setIgnoreThisCurrentLoad(bool newIgnoreThisCurrentLoad)
{
    m_ignoreThisCurrentLoad = newIgnoreThisCurrentLoad;
}

void NodeTreeView::onFolderDropSuccessful(const QString &path)
{
    auto *nodeTreeModel = static_cast<NodeTreeModel *>(model());
    auto index = nodeTreeModel->folderIndexFromIdPath(path);
    if (index.isValid()) {
        setCurrentIndexC(index);
    } else {
        setCurrentIndexC(nodeTreeModel->getAllNotesButtonIndex());
    }
}

void NodeTreeView::onTagsDropSuccessful(const QSet<int> &ids)
{
    auto *nodeTreeModel = static_cast<NodeTreeModel *>(model());
    setCurrentIndex(QModelIndex());
    clearSelection();
    setSelectionMode(QAbstractItemView::MultiSelection);
    for (const auto &id : std::as_const(ids)) {
        auto index = nodeTreeModel->tagIndexFromId(id);
        if (index.isValid()) {
            setCurrentIndex(index);
            setSelectionMode(QAbstractItemView::MultiSelection);
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
        }
    }

    if (selectionModel()->selectedIndexes().isEmpty()) {
        setCurrentIndexC(nodeTreeModel->getAllNotesButtonIndex());
    }
}

Theme::Value NodeTreeView::theme() const
{
    return m_theme;
}

bool NodeTreeView::isDragging() const
{
    return state() == DraggingState;
}

void NodeTreeView::reExpandC()
{
    auto needExpand = std::move(m_expanded);
    QTreeView::reset();
    for (const auto &path : needExpand) {
        auto *m = static_cast<NodeTreeModel *>(model());
        auto index = m->folderIndexFromIdPath(path);
        if (index.isValid()) {
            expand(index);
        }
    }
}

void NodeTreeView::reExpandC(const QStringList &expanded)
{
    m_expanded = expanded.toVector();
    reExpandC();
}

void NodeTreeView::onChangeTagColorAction()
{
    auto itemType = static_cast<NodeItem::Type>(m_currentEditingIndex.data(NodeItem::Roles::ItemType).toInt());
    if (itemType == NodeItem::Type::TagItem) {
        auto index = m_currentEditingIndex;
        emit changeTagColorRequested(index);
    }
}

void NodeTreeView::onRequestExpand(const QString &folderPath)
{
    auto *nodeTreeModel = static_cast<NodeTreeModel *>(model());
    expand(nodeTreeModel->folderIndexFromIdPath(folderPath));
}

void NodeTreeView::onUpdateAbsPath(const QString &oldPath, const QString &newPath)
{
    std::transform(m_expanded.begin(), m_expanded.end(), m_expanded.begin(), [&](QString s) {
        s.replace(s.indexOf(oldPath), oldPath.size(), newPath);
        return s;
    });
}

void NodeTreeView::updateEditingIndex(QPoint pos)
{
    auto index = indexAt(pos);
    if (indexAt(pos) != m_currentEditingIndex && !m_isContextMenuOpened && !m_isEditing) {
        auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
        if (itemType == NodeItem::Type::FolderItem || itemType == NodeItem::Type::TagItem || itemType == NodeItem::Type::TrashButton
            || itemType == NodeItem::Type::AllNoteButton) {
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

void NodeTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);
    if (m_ignoreThisCurrentLoad) {
        return;
    }
    auto indexes = selectedIndexes();
    QSet<int> tagIds;
    for (const auto index : std::as_const(indexes)) {
        auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
        switch (itemType) {
        case NodeItem::Type::RootItem:
        case NodeItem::Type::FolderSeparator:
        case NodeItem::Type::TagSeparator:
        case NodeItem::Type::NoteItem: {
            return;
        }
        case NodeItem::Type::AllNoteButton: {
            auto folderPath = index.data(NodeItem::Roles::AbsPath).toString();
            m_lastSelectFolder = folderPath;
            m_isLastSelectedFolder = true;
            emit loadNotesInFolderRequested(ROOT_FOLDER_ID, true);
            emit saveSelected(true, NodePath::getAllNoteFolderPath(), {});
            return;
        }
        case NodeItem::Type::TrashButton: {
            auto folderPath = index.data(NodeItem::Roles::AbsPath).toString();
            m_lastSelectFolder = folderPath;
            m_isLastSelectedFolder = true;
            emit loadNotesInFolderRequested(TRASH_FOLDER_ID, true);
            emit saveSelected(true, NodePath::getTrashFolderPath(), {});
            return;
        }
        case NodeItem::Type::FolderItem: {
            auto folderId = index.data(NodeItem::Roles::NodeId).toInt();
            auto folderPath = index.data(NodeItem::Roles::AbsPath).toString();
            m_lastSelectFolder = folderPath;
            m_isLastSelectedFolder = true;
            emit saveSelected(true, folderPath, {});
            emit loadNotesInFolderRequested(folderId, false);
            return;
        }
        case NodeItem::Type::TagItem: {
            auto tagId = index.data(NodeItem::Roles::NodeId).toInt();
            tagIds.insert(tagId);
        }
        }
    }
    if (!tagIds.isEmpty()) {
        if (m_isLastSelectedFolder) {
            emit saveLastSelectedNote();
        }
        m_isLastSelectedFolder = false;
        emit saveSelected(false, {}, tagIds);
        emit loadNotesInTagsRequested(tagIds);
    }
}

void NodeTreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);
    auto itemType = static_cast<NodeItem::Type>(current.data(NodeItem::Roles::ItemType).toInt());
    if (itemType == NodeItem::Type::TagItem) {
        setSelectionMode(QAbstractItemView::MultiSelection);
    } else {
        clearSelection();
        setSelectionMode(QAbstractItemView::SingleSelection);
        selectionModel()->setCurrentIndex(current, QItemSelectionModel::Current);
    }
}

void NodeTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(NOTE_MIME)) {
        event->acceptProposedAction();
    } else {
        QTreeView::dragEnterEvent(event);
    }
}

void NodeTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(NOTE_MIME)) {
        auto index = indexAt(event->position().toPoint());
        auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
        if (itemType != NodeItem::Type::AllNoteButton) {
            updateEditingIndex(event->position().toPoint());
            event->acceptProposedAction();
        }
    } else {
        if (event->mimeData()->hasFormat(TAG_MIME)) {
            if (event->position().toPoint().y() < visualRect(m_treeSeparator[1]).y() + 25) {
                event->ignore();
                return;
            }
        } else if (event->mimeData()->hasFormat(FOLDER_MIME)) {
            auto trashRect = visualRect(static_cast<NodeTreeModel *>(model())->getTrashButtonIndex());
            if (event->position().toPoint().y() > (trashRect.y() + 5) && event->position().toPoint().y() < (trashRect.bottom() - 5)) {
                setDropIndicatorShown(true);
                QTreeView::dragMoveEvent(event);
                return;
            }
            if (event->position().toPoint().y() > visualRect(m_treeSeparator[1]).y()) {
                event->ignore();
                return;
            }
            if (event->position().toPoint().y() < visualRect(m_defaultNotesIndex).y() + 25) {
                event->ignore();
                return;
            }
        }
        setDropIndicatorShown(true);
        QTreeView::dragMoveEvent(event);
    }
}

void NodeTreeView::reset()
{
    closeCurrentEditor();
    reExpandC();
}

void NodeTreeView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat(NOTE_MIME)) {
        auto dropIndex = indexAt(event->position().toPoint());
        if (dropIndex.isValid()) {
            auto itemType = static_cast<NodeItem::Type>(dropIndex.data(NodeItem::Roles::ItemType).toInt());
            bool ok = false;
            auto idl = QString::fromUtf8(event->mimeData()->data(NOTE_MIME)).split(PATH_SEPARATOR);
            for (const auto &s : std::as_const(idl)) {
                auto nodeId = s.toInt(&ok);
                if (ok) {
                    if (itemType == NodeItem::Type::FolderItem) {
                        emit moveNodeRequested(nodeId, dropIndex.data(NodeItem::NodeId).toInt());
                        event->acceptProposedAction();
                    } else if (itemType == NodeItem::Type::TagItem) {
                        emit addNoteToTag(nodeId, dropIndex.data(NodeItem::NodeId).toInt());
                    } else if (itemType == NodeItem::Type::TrashButton) {
                        emit moveNodeRequested(nodeId, TRASH_FOLDER_ID);
                        event->acceptProposedAction();
                    }
                }
            }
        }
    } else {
        QTreeView::dropEvent(event);
    }
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
    } /*else {
        qDebug() << __FUNCTION__ << "m_currentEditingIndex is not valid";
    }*/
}

void NodeTreeView::onRenameTagFinished(const QString &newName)
{
    if (m_currentEditingIndex.isValid()) {
        auto itemType = static_cast<NodeItem::Type>(m_currentEditingIndex.data(NodeItem::Roles::ItemType).toInt());
        if (itemType == NodeItem::Type::TagItem) {
            QModelIndex index = m_currentEditingIndex;
            closeCurrentEditor();
            emit renameTagInDatabase(index, newName);
        } else {
            qDebug() << __FUNCTION__ << "wrong type";
        }
    } /*else {
        qDebug() << __FUNCTION__ << "m_currentEditingIndex is not valid";
    }*/
}

void NodeTreeView::setCurrentIndexC(const QModelIndex &index)
{
    setCurrentIndex(index);
    clearSelection();
    setSelectionMode(QAbstractItemView::SingleSelection);
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
}

void NodeTreeView::setCurrentIndexNC(const QModelIndex &index)
{
    setCurrentIndex(index);
}

void NodeTreeView::setTheme(Theme::Value theme)
{
    setCSSThemeAndUpdate(this, theme);
    m_theme = theme;
}

void NodeTreeView::onCustomContextMenu(QPoint point)
{
    QModelIndex index = indexAt(point);
    if (index.isValid()) {
        auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
        m_contextMenu->clear();
        if (itemType == NodeItem::Type::FolderItem) {
            auto id = index.data(NodeItem::Roles::NodeId).toInt();
            if (id != DEFAULT_NOTES_FOLDER_ID) {
                m_isContextMenuOpened = true;
                m_contextMenu->addAction(m_renameFolderAction);
                m_contextMenu->addAction(m_deleteFolderAction);
                m_contextMenu->addSeparator();
                m_contextMenu->addAction(m_addSubfolderAction);
                m_contextMenu->exec(viewport()->mapToGlobal(point));
            }
        } else if (itemType == NodeItem::Type::TagItem) {
            m_isContextMenuOpened = true;
            m_contextMenu->addAction(m_renameTagAction);
            m_contextMenu->addAction(m_changeTagColorAction);
            m_contextMenu->addAction(m_clearSelectionAction);
            m_contextMenu->addSeparator();
            m_contextMenu->addAction(m_deleteTagAction);
            m_contextMenu->exec(viewport()->mapToGlobal(point));
        }
    }
}

void NodeTreeView::setTreeSeparator(const QVector<QModelIndex> &newTreeSeparator, const QModelIndex &defaultNotesIndex)
{
    for (const auto &sep : std::as_const(m_treeSeparator)) {
        closePersistentEditor(sep);
    }
    m_treeSeparator = newTreeSeparator;
    for (const auto &sep : std::as_const(m_treeSeparator)) {
        openPersistentEditor(sep);
    }
    m_defaultNotesIndex = defaultNotesIndex;
}

void NodeTreeView::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(NodeTreeView);
    QPoint topLeft;
    QPoint bottomRight = event->position().toPoint();
    if (state() == ExpandingState || state() == CollapsingState) {
        return;
    }

    updateEditingIndex(event->position().toPoint());
    if (state() == DraggingState) {
        topLeft = d->pressedPosition - d->offset();
        if ((topLeft - bottomRight).manhattanLength() > QApplication::startDragDistance()) {
            d->pressedIndex = QModelIndex();
            startDrag(d->model->supportedDragActions());
            setState(NoState); // the startDrag will return when the dnd operation is done
            stopAutoScroll();
        }
        return;
    }
    if (d->pressedIndex.isValid() && (state() != DragSelectingState) && (event->buttons() != Qt::NoButton)) {
        setState(DraggingState);
        return;
    }
}

void NodeTreeView::mousePressEvent(QMouseEvent *event)
{
    Q_D(NodeTreeView);
    d->delayedAutoScroll.stop();
    {
        auto index = indexAt(event->position().toPoint());
        if (index.isValid()) {
            auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
            switch (itemType) {
            case NodeItem::Type::FolderItem: {
                auto rect = visualRect(index);
                auto iconRect = QRect(rect.x() + 5, rect.y() + ((rect.height() - 20) / 2), 20, 20);
                if (iconRect.contains(event->position().toPoint())) {
                    if (isExpanded(index)) {
                        collapse(index);
                    } else {
                        expand(index);
                    }
                    return;
                }
                setCurrentIndexC(index);
                break;
            }
            case NodeItem::Type::TagItem: {
                auto oldIndexes = selectionModel()->selectedIndexes();
                for (const auto &ix : std::as_const(oldIndexes)) {
                    auto selectedItemType = static_cast<NodeItem::Type>(ix.data(NodeItem::Roles::ItemType).toInt());
                    if (selectedItemType != NodeItem::Type::TagItem) {
                        setCurrentIndex(QModelIndex());
                        clearSelection();
                        break;
                    }
                }

                if (selectionModel()->isSelected(index)) {
                    m_needReleaseIndex = index;
                } else {
                    setCurrentIndex(index);
                    setSelectionMode(QAbstractItemView::MultiSelection);
                    selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
                }
                break;
            }
            case NodeItem::Type::AllNoteButton:
            case NodeItem::Type::TrashButton: {
                setCurrentIndexC(index);
                break;
            }
            default: {
                break;
            }
            }
        }
    }
    QPoint pos = event->position().toPoint();
    QPersistentModelIndex index = indexAt(pos);
    d->pressedAlreadySelected = d->selectionModel->isSelected(index);
    d->pressedIndex = index;
    d->pressedModifiers = event->modifiers();
    QPoint offset = d->offset();
    d->pressedPosition = pos + offset;
    updateEditingIndex(event->position().toPoint());
}

void NodeTreeView::leaveEvent(QEvent *event)
{
    if (!m_isContextMenuOpened && !m_isEditing) {
        closeCurrentEditor();
        QTreeView::leaveEvent(event);
    }
}

void NodeTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    Q_D(NodeTreeView);
    if (m_needReleaseIndex.isValid()) {
        selectionModel()->select(m_needReleaseIndex, QItemSelectionModel::Deselect);
        if (selectionModel()->selectedIndexes().isEmpty()) {
            if (!m_isLastSelectedFolder) {
                auto index = static_cast<NodeTreeModel *>(model())->folderIndexFromIdPath(m_lastSelectFolder);
                if (index.isValid()) {
                    emit requestLoadLastSelectedNote();
                    setCurrentIndexC(index);
                } else {
                    setCurrentIndexC(static_cast<NodeTreeModel *>(model())->getAllNotesButtonIndex());
                }
            } else {
                setCurrentIndexC(static_cast<NodeTreeModel *>(model())->getAllNotesButtonIndex());
            }
        }
    }
    m_needReleaseIndex = QModelIndex();
    setState(NoState);
    d->pressedIndex = QPersistentModelIndex();
}

void NodeTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    Q_D(NodeTreeView);
    d->pressedIndex = QModelIndex();
    m_needReleaseIndex = QModelIndex();
    //    QTreeView::mouseDoubleClickEvent(event);
}

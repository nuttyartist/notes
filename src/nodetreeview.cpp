#include "nodetreeview.h"
#include "nodetreemodel.h"
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QDebug>
#include <QMimeData>
#include <QApplication>
#include "nodetreeview_p.h"

NodeTreeView::NodeTreeView(QWidget *parent)
    : QTreeView(parent),
      m_isContextMenuOpened{ false },
      m_isEditing{ false },
      m_ignoreThisCurrentLoad{ false },
      m_isLastSelectedFolder{ false }
{
    setHeaderHidden(true);
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
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
            R"(QScrollBar::sub-line:vertical { width:0px; height: 0px; subcontrol-position: top; subcontrol-origin: margin; })");
#else
    setStyleSheet(R"(QTreeView {)"
                  R"(    border-style: none;)"
                  R"(    background-color: rgb(255, 255, 255);)"
                  R"(    selection-background-color: white;)"
                  R"(    selection-color: white;)"
                  R"(})"
                  R"()"
                  R"(QTreeView::branch{)"
                  R"(    border-image: url(none.png);)"
                  R"(})");
#endif

    setRootIsDecorated(false);
    setMouseTracking(true);
    setSelectionMode(QAbstractItemView::MultiSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &NodeTreeView::onCustomContextMenu);
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

    renameTagAction = new QAction(tr("Rename Tag"), this);
    connect(renameTagAction, &QAction::triggered, this, [this] {
        setIsEditing(true);
        emit renameTagRequested();
    });
    changeTagColorAction = new QAction(tr("Change Tag Color"), this);
    connect(changeTagColorAction, &QAction::triggered, this, &NodeTreeView::onChangeTagColorAction);
    deleteTagAction = new QAction(tr("Delete Tag"), this);
    connect(deleteTagAction, &QAction::triggered, this, &NodeTreeView::onDeleteNodeAction);
    clearSelectionAction = new QAction(tr("Clear Selection"), this);
    connect(clearSelectionAction, &QAction::triggered, this, [this] {
        closeCurrentEditor();
        clearSelection();
        setCurrentIndexC(dynamic_cast<NodeTreeModel *>(model())->getAllNotesButtonIndex());
    });

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
    connect(this, &NodeTreeView::expanded, this, &NodeTreeView::onExpanded);
    connect(this, &NodeTreeView::collapsed, this, &NodeTreeView::onCollapsed);

    setDragEnabled(true);
    setAcceptDrops(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void NodeTreeView::onDeleteNodeAction()
{
    auto itemType = static_cast<NodeItem::Type>(
            m_currentEditingIndex.data(NodeItem::Roles::ItemType).toInt());
    auto id = m_currentEditingIndex.data(NodeItem::Roles::NodeId).toInt();
    if (itemType == NodeItem::Type::FolderItem || itemType == NodeItem::Type::NoteItem) {
        if (id > SpecialNodeID::DefaultNotesFolder) {
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

void NodeTreeView::onFolderDropSuccessfull(const QString &path)
{
    auto m_model = dynamic_cast<NodeTreeModel *>(model());
    auto index = m_model->folderIndexFromIdPath(path);
    if (index.isValid()) {
        setCurrentIndexC(index);
    } else {
        setCurrentIndexC(m_model->getAllNotesButtonIndex());
    }
}

void NodeTreeView::onTagsDropSuccessfull(const QSet<int> &ids)
{
    auto m_model = dynamic_cast<NodeTreeModel *>(model());
    setCurrentIndex(QModelIndex());
    clearSelection();
    setSelectionMode(QAbstractItemView::MultiSelection);
    for (const auto &id : qAsConst(ids)) {
        auto index = m_model->tagIndexFromId(id);
        if (index.isValid()) {
            setCurrentIndex(index);
            setSelectionMode(QAbstractItemView::MultiSelection);
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
        }
    }

    if (selectionModel()->selectedIndexes().isEmpty()) {
        setCurrentIndexC(m_model->getAllNotesButtonIndex());
    }
}

Theme NodeTreeView::theme() const
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
    m_expanded.clear();
    QTreeView::reset();
    for (const auto &path : needExpand) {
        auto m_model = dynamic_cast<NodeTreeModel *>(model());
        auto index = m_model->folderIndexFromIdPath(path);
        if (index.isValid()) {
            expand(index);
        }
    }
}

void NodeTreeView::reExpandC(const QStringList &expanded)
{
    m_expanded.clear();
    m_expanded = expanded.toVector();
    reExpandC();
}

void NodeTreeView::onChangeTagColorAction()
{
    auto itemType = static_cast<NodeItem::Type>(
            m_currentEditingIndex.data(NodeItem::Roles::ItemType).toInt());
    if (itemType == NodeItem::Type::TagItem) {
        auto index = m_currentEditingIndex;
        emit changeTagColorRequested(index);
    }
}

void NodeTreeView::onRequestExpand(const QString &folderPath)
{
    auto m_model = dynamic_cast<NodeTreeModel *>(model());
    expand(m_model->folderIndexFromIdPath(folderPath));
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
        if (itemType == NodeItem::Type::FolderItem || itemType == NodeItem::Type::TagItem
            || itemType == NodeItem::Type::TrashButton
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

void NodeTreeView::selectionChanged(const QItemSelection &selected,
                                    const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);
    if (m_ignoreThisCurrentLoad) {
        return;
    }
    auto indexes = selectedIndexes();
    QSet<int> tagIds;
    for (const auto index : qAsConst(indexes)) {
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
            emit loadNotesInFolderRequested(SpecialNodeID::RootFolder, true);
            emit saveSelected(true, NodePath::getAllNoteFolderPath(), {});
            return;
        }
        case NodeItem::Type::TrashButton: {
            auto folderPath = index.data(NodeItem::Roles::AbsPath).toString();
            m_lastSelectFolder = folderPath;
            m_isLastSelectedFolder = true;
            emit loadNotesInFolderRequested(SpecialNodeID::TrashFolder, true);
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
        auto index = indexAt(event->pos());
        auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
        if (itemType != NodeItem::Type::AllNoteButton) {
            updateEditingIndex(event->pos());
            event->acceptProposedAction();
        }
    } else {
        if (event->mimeData()->hasFormat(TAG_MIME)) {
            if (event->pos().y() < visualRect(m_treeSeparator[1]).y() + 25) {
                event->ignore();
                return;
            }
        } else if (event->mimeData()->hasFormat(FOLDER_MIME)) {
            auto trashRect =
                    visualRect(dynamic_cast<NodeTreeModel *>(model())->getTrashButtonIndex());
            if (event->pos().y() > (trashRect.y() + 5)
                && event->pos().y() < (trashRect.bottom() - 5)) {
                setDropIndicatorShown(true);
                QTreeView::dragMoveEvent(event);
                return;
            }
            if (event->pos().y() > visualRect(m_treeSeparator[1]).y()) {
                event->ignore();
                return;
            }
            if (event->pos().y() < visualRect(m_defaultNotesIndex).y() + 25) {
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
        auto dropIndex = indexAt(event->pos());
        if (dropIndex.isValid()) {
            auto itemType =
                    static_cast<NodeItem::Type>(dropIndex.data(NodeItem::Roles::ItemType).toInt());
            bool ok = false;
            auto idl = QString::fromUtf8(event->mimeData()->data(NOTE_MIME))
                               .split(QStringLiteral(PATH_SEPERATOR));
            for (const auto &s : qAsConst(idl)) {
                auto nodeId = s.toInt(&ok);
                if (ok) {
                    if (itemType == NodeItem::Type::FolderItem) {
                        emit moveNodeRequested(nodeId, dropIndex.data(NodeItem::NodeId).toInt());
                        event->acceptProposedAction();
                    } else if (itemType == NodeItem::Type::TagItem) {
                        emit addNoteToTag(nodeId, dropIndex.data(NodeItem::NodeId).toInt());
                    } else if (itemType == NodeItem::Type::TrashButton) {
                        emit moveNodeRequested(nodeId, SpecialNodeID::TrashFolder);
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
        auto itemType = static_cast<NodeItem::Type>(
                m_currentEditingIndex.data(NodeItem::Roles::ItemType).toInt());
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
        auto itemType = static_cast<NodeItem::Type>(
                m_currentEditingIndex.data(NodeItem::Roles::ItemType).toInt());
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

void NodeTreeView::setTheme(Theme theme)
{
    m_theme = theme;
#if !defined(Q_OS_MACOS)
    QString ss = QStringLiteral(
            R"(QTreeView {)"
            R"(    border-style: none;)"
            R"(    background-color: %1;)"
            R"(    selection-background-color: %1;)"
            R"(    selection-color: white;)"
            R"(})"
            R"()"
            R"(QTreeView::branch{)"
            R"(    border-image: url(none.png);)"
            R"(})"
            R"(QScrollBar::handle:vertical:hover { background: rgba(40, 40, 40, 0.5); } )"
            R"(QScrollBar::handle:vertical:pressed { background: rgba(40, 40, 40, 0.5); } )"
            R"(QScrollBar::handle:vertical { border-radius: 4px; background: rgba(100, 100, 100, 0.5); min-height: 20px; }  )"
            R"(QScrollBar::vertical {border-radius: 6px; width: 10px; color: rgba(255, 255, 255,0);} )"
            R"(QScrollBar {margin-right: 2px; background: transparent;} )"
            R"(QScrollBar::add-line:vertical { width:0px; height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  )"
            R"(QScrollBar::sub-line:vertical { width:0px; height: 0px; subcontrol-position: top; subcontrol-origin: margin; })");
#else
    QString ss = QStringLiteral(R"(QTreeView {)"
                                R"(    border-style: none;)"
                                R"(    background-color: %1;)"
                                R"(    selection-background-color: %1;)"
                                R"(    selection-color: white;)"
                                R"(})"
                                R"()"
                                R"(QTreeView::branch{)"
                                R"(    border-image: url(none.png);)"
                                R"(})");
#endif

    switch (theme) {
    case Theme::Light: {
        setStyleSheet(ss.arg(QColor(247, 247, 247).name()));
        break;
    }
    case Theme::Dark: {
        setStyleSheet(ss.arg(QColor(26, 26, 26).name()));
        break;
    }
    case Theme::Sepia: {
        setStyleSheet(ss.arg(QColor(251, 240, 217).name()));
        break;
    }
    }
}

void NodeTreeView::onCustomContextMenu(QPoint point)
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
        } else if (itemType == NodeItem::Type::TagItem) {
            m_isContextMenuOpened = true;
            contextMenu->addAction(renameTagAction);
            contextMenu->addAction(changeTagColorAction);
            contextMenu->addAction(clearSelectionAction);
            contextMenu->addSeparator();
            contextMenu->addAction(deleteTagAction);
            contextMenu->exec(viewport()->mapToGlobal(point));
        }
    }
}

void NodeTreeView::setTreeSeparator(const QVector<QModelIndex> &newTreeSeparator,
                                    const QModelIndex &defaultNotesIndex)
{
    for (const auto &sep : qAsConst(m_treeSeparator)) {
        closePersistentEditor(sep);
    }
    m_treeSeparator = newTreeSeparator;
    for (const auto &sep : qAsConst(m_treeSeparator)) {
        openPersistentEditor(sep);
    }
    m_defaultNotesIndex = defaultNotesIndex;
}

void NodeTreeView::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(NodeTreeView);
    QPoint topLeft;
    QPoint bottomRight = event->pos();
    if (state() == ExpandingState || state() == CollapsingState) {
        return;
    }

    updateEditingIndex(event->pos());
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
    if (d->pressedIndex.isValid() && (state() != DragSelectingState)
        && (event->buttons() != Qt::NoButton)) {
        setState(DraggingState);
        return;
    }
}

void NodeTreeView::mousePressEvent(QMouseEvent *event)
{
    Q_D(NodeTreeView);
    d->delayedAutoScroll.stop();
    {
        auto index = indexAt(event->pos());
        if (index.isValid()) {
            auto itemType =
                    static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
            switch (itemType) {
            case NodeItem::Type::FolderItem: {
                auto rect = visualRect(index);
                auto iconRect = QRect(rect.x() + 5, rect.y() + (rect.height() - 20) / 2, 20, 20);
                if (iconRect.contains(event->pos())) {
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
                for (const auto &ix : qAsConst(oldIndexes)) {
                    auto selectedItemType =
                            static_cast<NodeItem::Type>(ix.data(NodeItem::Roles::ItemType).toInt());
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
    QPoint pos = event->pos();
    QPersistentModelIndex index = indexAt(pos);
    d->pressedAlreadySelected = d->selectionModel->isSelected(index);
    d->pressedIndex = index;
    d->pressedModifiers = event->modifiers();
    QPoint offset = d->offset();
    d->pressedPosition = pos + offset;
    updateEditingIndex(event->pos());
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
                auto index = dynamic_cast<NodeTreeModel *>(model())->folderIndexFromIdPath(
                        m_lastSelectFolder);
                if (index.isValid()) {
                    emit requestLoadLastSelectedNote();
                    setCurrentIndexC(index);
                } else {
                    setCurrentIndexC(
                            dynamic_cast<NodeTreeModel *>(model())->getAllNotesButtonIndex());
                }
            } else {
                setCurrentIndexC(dynamic_cast<NodeTreeModel *>(model())->getAllNotesButtonIndex());
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

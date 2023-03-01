#include "notelistview.h"
#include "notelistdelegate.h"
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QAbstractItemView>
#include <QPaintEvent>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QScrollBar>
#include <QMenu>
#include <QAction>
#include <QDrag>
#include <QMimeData>
#include <QWindow>
#include <QMetaObject>
#include "tagpool.h"
#include "notelistmodel.h"
#include "nodepath.h"
#include "dbmanager.h"
#include "notelistview_p.h"
#include "notelistdelegateeditor.h"

NoteListView::NoteListView(QWidget *parent)
    : QListView(parent),
      m_isScrollBarHidden(true),
      m_animationEnabled(true),
      m_isMousePressed(false),
      m_mousePressHandled(false),
      m_rowHeight(38),
      m_currentBackgroundColor(247, 247, 247),
      m_tagPool(nullptr),
      m_dbManager(nullptr),
      m_currentFolderId{ SpecialNodeID::InvalidNodeId },
      m_isInTrash{ false },
      m_isDragging{ false },
      m_isDraggingPinnedNotes{ false },
      m_isPinnedNotesCollapsed{ false },
      m_isDraggingInsidePinned{ false }
{
    setAttribute(Qt::WA_MacShowFocusRect, false);

    QTimer::singleShot(0, this, SLOT(init()));
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &NoteListView::onCustomContextMenu);
    contextMenu = new QMenu(this);

    deleteNoteAction = new QAction(tr("Delete Note"), this);
    connect(deleteNoteAction, &QAction::triggered, this, [this] {
        auto indexes = selectedIndexes();
        emit deleteNoteRequested(indexes);
    });
    restoreNoteAction = new QAction(tr("Restore Note"), this);
    connect(restoreNoteAction, &QAction::triggered, this, [this] {
        auto indexes = selectedIndexes();
        emit restoreNoteRequested(indexes);
    });
    pinNoteAction = new QAction(tr("Pin Note"), this);
    connect(pinNoteAction, &QAction::triggered, this, [this] {
        auto indexes = selectedIndexes();
        emit setPinnedNoteRequested(indexes, true);
    });
    unpinNoteAction = new QAction(tr("Unpin Note"), this);
    connect(unpinNoteAction, &QAction::triggered, this, [this] {
        auto indexes = selectedIndexes();
        emit setPinnedNoteRequested(indexes, false);
    });

    newNoteAction = new QAction(tr("New Note"), this);
    connect(newNoteAction, &QAction::triggered, this, [this] { emit newNoteRequested(); });

    m_dragPixmap.load("qrc:/images/notes_icon.icns");
    setDragEnabled(true);
    setAcceptDrops(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

NoteListView::~NoteListView()
{
    // Make sure any editors are closed before the view is destroyed
    closeAllEditor();
}

void NoteListView::animateAddedRow(const QModelIndexList &indexes)
{
    NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
    if (delegate)
        delegate->setState(NoteListState::Insert, indexes);
}

bool NoteListView::isPinnedNotesCollapsed() const
{
    return m_isPinnedNotesCollapsed;
}

void NoteListView::setIsPinnedNotesCollapsed(bool newIsPinnedNotesCollapsed)
{
    m_isPinnedNotesCollapsed = newIsPinnedNotesCollapsed;
    for (int i = 0; i < model()->rowCount(); ++i) {
        auto index = model()->index(i, 0);
        if (index.isValid()) {
            emit itemDelegate()->sizeHintChanged(index);
        }
    }
    update();
    emit pinnedCollapseChanged();
}

void NoteListView::setCurrentIndexC(const QModelIndex &index)
{
    setCurrentIndex(index);
    clearSelection();
    setSelectionMode(QAbstractItemView::SingleSelection);
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
}

QModelIndexList NoteListView::selectedIndex() const
{
    return selectedIndexes();
}

void NoteListView::onRemoveRowRequested(const QModelIndexList &indexes)
{
    if (!indexes.isEmpty()) {
        for (const auto index : qAsConst(indexes)) {
            m_needRemovedNotes.push_back(index.data(NoteListModel::NoteID).toInt());
        }
        NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
        if (delegate) {
            if (m_animationEnabled) {
                delegate->setState(NoteListState::Remove, indexes);
            } else {
                delegate->setState(NoteListState::Normal, indexes);
            }
        }
    }
}

bool NoteListView::isDragging() const
{
    return m_isDragging;
}

void NoteListView::setListViewInfo(const ListViewInfo &newListViewInfo)
{
    m_listViewInfo = newListViewInfo;
}

void NoteListView::setCurrentFolderId(int newCurrentFolderId)
{
    m_currentFolderId = newCurrentFolderId;
}

void NoteListView::openPersistentEditorC(const QModelIndex &index)
{
    if (index.isValid()) {
        auto isHaveTag = dynamic_cast<NoteListModel *>(model())->noteIsHaveTag(index);
        if (isHaveTag) {
            auto id = index.data(NoteListModel::NoteID).toInt();
            m_openedEditor[id] = {};
            openPersistentEditor(index);
        }
    }
}

void NoteListView::closePersistentEditorC(const QModelIndex &index)
{
    if (index.isValid()) {
        auto id = index.data(NoteListModel::NoteID).toInt();
        closePersistentEditor(index);
        m_openedEditor.remove(id);
    }
}

void NoteListView::setEditorWidget(int noteId, QWidget *w)
{
    if (m_openedEditor.contains(noteId)) {
        m_openedEditor[noteId].push_back(w);
    } else {
        qDebug() << __FUNCTION__ << "Error: note id" << noteId << "is not in opened editor list";
    }
}

void NoteListView::unsetEditorWidget(int noteId, QWidget *w)
{
    if (m_openedEditor.contains(noteId)) {
        m_openedEditor[noteId].removeAll(w);
    }
}

void NoteListView::closeAllEditor()
{
    for (const auto &id : m_openedEditor.keys()) {
        auto index = dynamic_cast<NoteListModel *>(model())->getNoteIndex(id);
        closePersistentEditor(index);
    }
    m_openedEditor.clear();
}

void NoteListView::setDbManager(DBManager *newDbManager)
{
    m_dbManager = newDbManager;
}

void NoteListView::setIsInTrash(bool newIsInTrash)
{
    m_isInTrash = newIsInTrash;
}

void NoteListView::setTagPool(TagPool *newTagPool)
{
    m_tagPool = newTagPool;
}

void NoteListView::rowsAboutToBeMoved(const QModelIndexList &source)
{
    NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
    if (delegate) {
        if (m_animationEnabled) {
            delegate->setState(NoteListState::MoveOut, source);
        } else {
            delegate->setState(NoteListState::Normal, source);
        }
    }
}

void NoteListView::rowsMoved(const QModelIndexList &dest)
{
    NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
    if (delegate) {
        if (m_animationEnabled) {
            delegate->setState(NoteListState::Insert, dest);
        } else {
            delegate->setState(NoteListState::Normal, dest);
        }
    }
}

void NoteListView::onRowsInserted(const QModelIndexList &rows)
{
    animateAddedRow(rows);
}

void NoteListView::init()
{
    setMouseTracking(true);
    setUpdatesEnabled(true);
    viewport()->setAttribute(Qt::WA_Hover);

    setupStyleSheet();
    setupSignalsSlots();
}

bool NoteListView::isDraggingInsidePinned() const
{
    return m_isDraggingInsidePinned;
}

void NoteListView::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_isMousePressed) {
        QListView::mouseMoveEvent(event);
        return;
    }
    if (event->buttons() & Qt::LeftButton) {
        if ((event->pos() - m_dragStartPosition).manhattanLength()
            >= QApplication::startDragDistance()) {
            startDrag(Qt::MoveAction);
        }
    }
    //    QListView::mouseMoveEvent(event);
}

void NoteListView::mousePressEvent(QMouseEvent *e)
{
    Q_D(NoteListView);
    m_isMousePressed = true;
    auto index = indexAt(e->pos());
    if (!index.isValid()) {
        emit noteListViewClicked();
        return;
    }
    auto model = dynamic_cast<NoteListModel *>(this->model());
    if (model && model->isFirstPinnedNote(index)) {
        auto rect = visualRect(index);
        auto iconRect = QRect(rect.right() - 25, rect.y() + 2, 20, 20);
        if (iconRect.contains(e->pos())) {
            setIsPinnedNotesCollapsed(!isPinnedNotesCollapsed());
            m_mousePressHandled = true;
            return;
        }
    }

    if (e->button() == Qt::LeftButton) {
        m_dragStartPosition = e->pos();
        auto oldIndexes = selectionModel()->selectedIndexes();
        if (!oldIndexes.contains(index)) {
            if (e->modifiers() == Qt::ControlModifier) {
                setSelectionMode(QAbstractItemView::MultiSelection);
                setCurrentIndex(index);
                selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
                auto selectedIndexes = selectionModel()->selectedIndexes();
                emit notePressed(selectedIndexes);
            } else {
                setCurrentIndexC(index);
                emit notePressed({ index });
            }
            m_mousePressHandled = true;
        }
    } else if (e->button() == Qt::RightButton) {
        auto oldIndexes = selectionModel()->selectedIndexes();
        if (!oldIndexes.contains(index)) {
            setCurrentIndexC(index);
            emit notePressed({ index });
        }
    }
    QPoint offset = d->offset();
    d->pressedPosition = e->pos() + offset;
}

void NoteListView::mouseReleaseEvent(QMouseEvent *e)
{
    m_isMousePressed = false;
    auto index = indexAt(e->pos());
    if (!index.isValid()) {
        return;
    }
    if (e->button() == Qt::LeftButton && !m_mousePressHandled) {
        if (e->modifiers() == Qt::ControlModifier) {
            setSelectionMode(QAbstractItemView::MultiSelection);
            auto oldIndexes = selectionModel()->selectedIndexes();
            if (oldIndexes.contains(index) && oldIndexes.size() > 1) {
                selectionModel()->select(index, QItemSelectionModel::Deselect);
            } else {
                setCurrentIndex(index);
                selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
            }
            auto selectedIndexes = selectionModel()->selectedIndexes();
            emit notePressed(selectedIndexes);
        } else {
            setCurrentIndexC(index);
            emit notePressed({ index });
        }
    }
    m_mousePressHandled = false;
    QListView::mouseReleaseEvent(e);
}

bool NoteListView::viewportEvent(QEvent *e)
{
    if (model()) {
        switch (e->type()) {
        case QEvent::Leave: {
            QPoint pt = mapFromGlobal(QCursor::pos());
            QModelIndex index = indexAt(QPoint(10, pt.y()));
            if (index.row() > 0) {
                index = model()->index(index.row() - 1, 0);
                NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
                if (delegate) {
                    delegate->setHoveredIndex(QModelIndex());
                    viewport()->update(visualRect(index));
                }
            }
            break;
        }
        default:
            break;
        }
    }

    return QListView::viewportEvent(e);
}

void NoteListView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(NOTE_MIME)) {
        event->acceptProposedAction();
    } else {
        QListView::dragEnterEvent(event);
    }
}

void NoteListView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(NOTE_MIME)) {
        auto index = indexAt(event->pos());
        auto isPinned = index.data(NoteListModel::NoteIsPinned).toBool();
        if (!index.isValid()) {
            event->ignore();
            return;
        }
        if (!m_isDraggingPinnedNotes && !isPinned) {
            event->ignore();
            return;
        }
        m_isDraggingInsidePinned = isPinned;
        event->acceptProposedAction();
        setDropIndicatorShown(true);
        QListView::dragMoveEvent(event);
        return;
    } else {
        event->ignore();
    }
}

void NoteListView::scrollContentsBy(int dx, int dy)
{
    QListView::scrollContentsBy(dx, dy);
    auto m_listModel = dynamic_cast<NoteListModel *>(model());
    if (!m_listModel) {
        return;
    }
    for (int i = 0; i < m_listModel->rowCount(); ++i) {
        auto index = m_listModel->index(i, 0);
        if (index.isValid()) {
            auto id = index.data(NoteListModel::NoteID).toInt();
            if (m_openedEditor.contains(id)) {
                auto y = visualRect(index).y();
                auto range = abs(viewport()->height());
                if ((y < -range) || (y > 2 * range)) {
                    m_openedEditor.remove(id);
                    closePersistentEditor(index);
                }
            } else {
                auto y = visualRect(index).y();
                auto range = abs(viewport()->height());
                if (y < -range) {
                    continue;
                } else if (y > 2 * range) {
                    break;
                }
                openPersistentEditorC(index);
            }
        }
    }
}

void NoteListView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);
    Q_D(NoteListView);
    auto indexes = selectedIndexes();
    QMimeData *mimeData = d->model->mimeData(indexes);
    if (!mimeData) {
        return;
    }
    QRect rect;
    QPixmap pixmap;
    if (indexes.size() == 1) {
        auto current = indexes[0];
        auto id = current.data(NoteListModel::NoteID).toInt();
        if (m_openedEditor.contains(id)) {
            QItemViewPaintPairs paintPairs = d->draggablePaintPairs(indexes, &rect);
            Q_UNUSED(paintPairs);
            auto wl = m_openedEditor[id];
            if (!wl.empty()) {
                pixmap = wl.first()->grab();
            } else {
                qDebug() << __FUNCTION__ << "Dragging row" << current.row()
                         << "is in opened editor list but editor widget is null";
            }
        } else {
            pixmap = d->renderToPixmap(indexes, &rect);
        }
        auto model = dynamic_cast<NoteListModel *>(this->model());
        if (model && model->hasPinnedNote()
            && (model->isFirstPinnedNote(current) || model->isFirstUnpinnedNote(current))) {
            QRect r(0, 25, rect.width(), rect.height() - 25);
            pixmap = pixmap.copy(r);
            rect.setHeight(rect.height() - 25);
        }
        rect.adjust(horizontalOffset(), verticalOffset(), 0, 0);
    } else {
        pixmap.load(":/images/notes_icon.ico");
        pixmap = pixmap.scaled(pixmap.width() / 3, pixmap.height() / 3, Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);
#ifdef __APPLE__
        QFont m_displayFont(QFont(QStringLiteral("SF Pro Text")).exactMatch()
                                    ? QStringLiteral("SF Pro Text")
                                    : QStringLiteral("Roboto"));
#elif _WIN32
        QFont m_displayFont(QFont(QStringLiteral("Segoe UI")).exactMatch()
                                    ? QStringLiteral("Segoe UI")
                                    : QStringLiteral("Roboto"));
#else
        QFont m_displayFont(QStringLiteral("Roboto"));
#endif
        m_displayFont.setPixelSize(16);
        QFontMetrics fmContent(m_displayFont);
        QString sz = QString::number(indexes.size());
        QRect szRect = fmContent.boundingRect(sz);
        QPixmap px(pixmap.width() + szRect.width(), pixmap.height());
        px.fill(Qt::transparent);

        QRect nameRect(px.rect());
        QPainter painter(&px);
        painter.setPen(Qt::red);
        painter.drawPixmap(0, 0, pixmap);
        painter.setFont(m_displayFont);
        painter.drawText(nameRect, Qt::AlignRight | Qt::AlignBottom, sz);
        painter.end();
        std::swap(pixmap, px);
        rect = px.rect();
    }
    m_isDraggingPinnedNotes = false;
    for (const auto &index : qAsConst(indexes)) {
        if (index.data(NoteListModel::NoteIsPinned).toBool()) {
            m_isDraggingPinnedNotes = true;
            break;
        }
    }
    QDrag *drag = new QDrag(this);
    drag->setPixmap(pixmap);
    drag->setMimeData(mimeData);
    if (indexes.size() == 1) {
        drag->setHotSpot(d->pressedPosition - rect.topLeft());
    } else {
        drag->setHotSpot({ 0, 0 });
    }
    auto openedEditors = m_openedEditor.keys();
    m_isDragging = true;
    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
    /// Delete later, if there is no drop event.
    if (dropAction == Qt::IgnoreAction) {
        drag->deleteLater();
        mimeData->deleteLater();
    }
#if QT_VERSION > QT_VERSION_CHECK(5, 15, 0)
    d->dropEventMoved = false;
#endif
    m_isDragging = false;
    // Reset the drop indicator
    d->dropIndicatorRect = QRect();
    d->dropIndicatorPosition = OnItem;
    closeAllEditor();
    for (const auto &id : qAsConst(openedEditors)) {
        auto index = dynamic_cast<NoteListModel *>(model())->getNoteIndex(id);
        openPersistentEditorC(index);
    }
    scrollContentsBy(0, 0);
}

void NoteListView::setCurrentRowActive(bool isActive)
{
    NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
    if (!delegate)
        return;

    delegate->setActive(isActive);
    viewport()->update(visualRect(currentIndex()));
}

void NoteListView::setAnimationEnabled(bool isEnabled)
{
    m_animationEnabled = isEnabled;
}

void NoteListView::setupSignalsSlots()
{
    // remove/add separator
    // current selectected row changed
    connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex &current, const QModelIndex &previous) {
                if (model()) {
                    if (current.row() < previous.row()) {
                        if (current.row() > 0) {
                            QModelIndex prevIndex = model()->index(current.row() - 1, 0);
                            viewport()->update(visualRect(prevIndex));
                        }
                    }

                    if (current.row() > 1) {
                        QModelIndex prevPrevIndex = model()->index(current.row() - 2, 0);
                        viewport()->update(visualRect(prevPrevIndex));
                    }
                }
            });

    // row was entered
    connect(this, &NoteListView::entered, this, [this](const QModelIndex &index) {
        if (model()) {
            if (index.row() > 1) {
                QModelIndex prevPrevIndex = model()->index(index.row() - 2, 0);
                viewport()->update(visualRect(prevPrevIndex));

                QModelIndex prevIndex = model()->index(index.row() - 1, 0);
                viewport()->update(visualRect(prevIndex));

            } else if (index.row() > 0) {
                QModelIndex prevIndex = model()->index(index.row() - 1, 0);
                viewport()->update(visualRect(prevIndex));
            }

            NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
            if (delegate)
                delegate->setHoveredIndex(index);
        }
    });

    // viewport was entered
    connect(this, &NoteListView::viewportEntered, this, [this]() {
        if (model() && model()->rowCount() > 1) {
            NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
            if (delegate)
                delegate->setHoveredIndex(QModelIndex());

            QModelIndex lastIndex = model()->index(model()->rowCount() - 2, 0);
            viewport()->update(visualRect(lastIndex));
        }
    });

    // remove/add offset right side
    connect(verticalScrollBar(), &QScrollBar::rangeChanged, this, [this](int min, int max) {
        Q_UNUSED(min)

        NoteListDelegate *delegate = dynamic_cast<NoteListDelegate *>(itemDelegate());
        if (delegate) {
            if (max > 0) {
                delegate->setRowRightOffset(2);
            } else {
                delegate->setRowRightOffset(0);
            }
            viewport()->update();
        }
    });
}

/**
 * @brief setup styleSheet
 */
void NoteListView::setupStyleSheet()
{
#if !defined(Q_OS_MACOS)
    QString ss =
            QString("QListView {background-color: %1;} "
                    "QScrollBar::handle:vertical:hover { background: rgba(40, 40, 40, 0.5); }"
                    "QScrollBar::handle:vertical:pressed { background: rgba(40, 40, 40, 0.5); }"
                    "QScrollBar::handle:vertical { border-radius: 4px; background: rgba(100, 100, "
                    "100, 0.5); min-height: 20px; }"
                    "QScrollBar::vertical {border-radius: 6px; width: 10px; color: rgba(255, 255, "
                    "255,0);}"
                    "QScrollBar {margin-right: 2px; background: transparent;}"
                    "QScrollBar::add-line:vertical { width:0px; height: 0px; subcontrol-position: "
                    "bottom; subcontrol-origin: margin; }"
                    "QScrollBar::sub-line:vertical { width:0px; height: 0px; subcontrol-position: "
                    "top; subcontrol-origin: margin; }")
                    .arg(m_currentBackgroundColor.name());
#else
    QString ss = QString("QListView {background-color: %1;} ").arg(m_currentBackgroundColor.name());
#endif
    setStyleSheet(ss);
}

void NoteListView::addNotesToTag(QSet<int> notesId, int tagId)
{
    for (const auto &id : qAsConst(notesId)) {
        auto model = dynamic_cast<NoteListModel *>(this->model());
        if (model) {
            auto index = model->getNoteIndex(id);
            if (index.isValid()) {
                emit addTagRequested(index, tagId);
            }
        }
    }
}

void NoteListView::removeNotesFromTag(QSet<int> notesId, int tagId)
{
    for (const auto &id : qAsConst(notesId)) {
        auto model = dynamic_cast<NoteListModel *>(this->model());
        if (model) {
            auto index = model->getNoteIndex(id);
            if (index.isValid()) {
                emit removeTagRequested(index, tagId);
            }
        }
    }
}

void NoteListView::selectionChanged(const QItemSelection &selected,
                                    const QItemSelection &deselected)
{
    QListView::selectionChanged(selected, deselected);
    QSet<int> ids;
    for (const auto &index : selectedIndexes()) {
        ids.insert(index.data(NoteListModel::NoteID).toInt());
    }
    emit saveSelectedNote(ids);
}

/**
 * @brief Set theme color for noteView
 */
void NoteListView::setTheme(Theme theme)
{
    switch (theme) {
    case Theme::Light: {
        m_currentBackgroundColor = QColor(247, 247, 247);
        break;
    }
    case Theme::Dark: {
        m_currentBackgroundColor = QColor(30, 30, 30);
        break;
    }
    case Theme::Sepia: {
        m_currentBackgroundColor = QColor(251, 240, 217);
        break;
    }
    }

    setupStyleSheet();
}

void NoteListView::onCustomContextMenu(QPoint point)
{
    QModelIndex index = indexAt(point);
    if (index.isValid()) {
        auto indexList = selectionModel()->selectedIndexes();
        if (!indexList.contains(index)) {
            setCurrentIndexC(index);
            indexList = selectionModel()->selectedIndexes();
        }
        QSet<int> notes;
        for (const auto &idx : qAsConst(indexList)) {
            notes.insert(idx.data(NoteListModel::NoteID).toInt());
        }
        contextMenu->clear();
        if (m_tagPool) {
            m_tagsMenu = contextMenu->addMenu("Tags ...");
            for (auto action : qAsConst(m_noteTagActions)) {
                delete action;
            }
            m_noteTagActions.clear();
            auto createTagIcon = [](const QString &color) -> QIcon {
                QPixmap pix{ 32, 32 };
                pix.fill(Qt::transparent);
                QPainter painter{ &pix };
                painter.setRenderHint(QPainter::Antialiasing);
                auto iconRect = QRect((pix.width() - 30) / 2, (pix.height() - 30) / 2, 30, 30);
                painter.setBrush(QColor(color));
                painter.setPen(QColor(color));
                painter.drawEllipse(iconRect);
                return QIcon{ pix };
            };
            QSet<int> tagInNote;
            const auto tagIds = m_tagPool->tagIds();
            for (const auto &id : tagIds) {
                bool all = true;
                for (const auto &selectedIndex : qAsConst(indexList)) {
                    auto tags = selectedIndex.data(NoteListModel::NoteTagsList).value<QSet<int>>();
                    if (!tags.contains(id)) {
                        all = false;
                        break;
                    }
                }
                if (all) {
                    tagInNote.insert(id);
                }
            }
            for (auto id : qAsConst(tagInNote)) {
                auto tag = m_tagPool->getTag(id);
                auto tagAction = new QAction(QString("✓ Remove tag ") + tag.name(), this);
                connect(tagAction, &QAction::triggered, this,
                        [this, id, notes] { removeNotesFromTag(notes, id); });
                tagAction->setIcon(createTagIcon(tag.color()));
                m_tagsMenu->addAction(tagAction);
                m_noteTagActions.append(tagAction);
            }
            m_tagsMenu->addSeparator();
            for (auto id : tagIds) {
                if (tagInNote.contains(id)) {
                    continue;
                }
                auto tag = m_tagPool->getTag(id);
                auto tagAction = new QAction(QString(" ") + tag.name(), this);
                connect(tagAction, &QAction::triggered, this,
                        [this, id, notes] { addNotesToTag(notes, id); });
                tagAction->setIcon(createTagIcon(tag.color()));
                m_tagsMenu->addAction(tagAction);
                m_noteTagActions.append(tagAction);
            }
        }
        if (m_isInTrash) {
            if (notes.size() > 1) {
                restoreNoteAction->setText(tr("Restore Notes"));
            } else {
                restoreNoteAction->setText(tr("Restore Note"));
            }
            contextMenu->addAction(restoreNoteAction);
        }
        if (notes.size() > 1) {
            deleteNoteAction->setText(tr("Delete Notes"));
        } else {
            deleteNoteAction->setText(tr("Delete Note"));
        }
        contextMenu->addAction(deleteNoteAction);
        if ((!m_listViewInfo.isInTag)
            && (m_listViewInfo.parentFolderId != SpecialNodeID::TrashFolder)) {
            contextMenu->addSeparator();
            if (notes.size() > 1) {
                pinNoteAction->setText(tr("Pin Notes"));
                unpinNoteAction->setText(tr("Unpin Notes"));
                enum class ShowAction { NotInit, ShowPin, ShowBoth, ShowUnpin };
                ShowAction a = ShowAction::NotInit;
                for (const auto &idx : qAsConst(indexList)) {
                    if (idx.data(NoteListModel::NoteIsPinned).toBool()) {
                        if (a == ShowAction::ShowPin) {
                            a = ShowAction::ShowBoth;
                            break;
                        } else {
                            a = ShowAction::ShowUnpin;
                        }
                    } else {
                        if (a == ShowAction::ShowUnpin) {
                            a = ShowAction::ShowBoth;
                            break;
                        } else {
                            a = ShowAction::ShowPin;
                        }
                    }
                }
                switch (a) {
                case ShowAction::ShowPin:
                    contextMenu->addAction(pinNoteAction);
                    break;
                case ShowAction::ShowUnpin:
                    contextMenu->addAction(unpinNoteAction);
                    break;
                default:
                    contextMenu->addAction(pinNoteAction);
                    contextMenu->addAction(unpinNoteAction);
                }
            } else {
                pinNoteAction->setText(tr("Pin Note"));
                unpinNoteAction->setText(tr("Unpin Note"));
                auto isPinned = index.data(NoteListModel::NoteIsPinned).toBool();
                if (!isPinned) {
                    contextMenu->addAction(pinNoteAction);
                } else {
                    contextMenu->addAction(unpinNoteAction);
                }
            }
        }
        contextMenu->addSeparator();
        if (m_dbManager) {
            for (auto action : qAsConst(m_folderActions)) {
                delete action;
            }
            m_folderActions.clear();
            auto m = contextMenu->addMenu("Move to");
            FolderListType folders;
            QMetaObject::invokeMethod(m_dbManager, "getFolderList", Qt::BlockingQueuedConnection,
                                      Q_RETURN_ARG(FolderListType, folders));
            for (const auto &id : folders.keys()) {
                if (id == m_currentFolderId) {
                    continue;
                }
                auto action = new QAction(folders[id], this);
                connect(action, &QAction::triggered, this, [this, id] {
                    auto indexes = selectedIndexes();
                    for (const auto &selectedIndex : qAsConst(indexes)) {
                        if (selectedIndex.isValid()) {
                            emit moveNoteRequested(
                                    selectedIndex.data(NoteListModel::NoteID).toInt(), id);
                        }
                    }
                });
                m->addAction(action);
                m_folderActions.append(action);
            }
            contextMenu->addSeparator();
        }
        contextMenu->addAction(newNoteAction);
        contextMenu->exec(viewport()->mapToGlobal(point));
    }
}

void NoteListView::onAnimationFinished(NoteListState state)
{
    if (state == NoteListState::Remove) {
        auto model = dynamic_cast<NoteListModel *>(this->model());
        if (model) {
            for (const auto id : qAsConst(m_needRemovedNotes)) {
                auto index = model->getNoteIndex(id);
                model->removeRow(index.row());
            }
            m_needRemovedNotes.clear();
        }
    }
}

QPixmap NoteListViewPrivate::renderToPixmap(const QModelIndexList &indexes, QRect *r) const
{
    Q_ASSERT(r);
    QItemViewPaintPairs paintPairs = draggablePaintPairs(indexes, r);
    if (paintPairs.isEmpty())
        return QPixmap();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    qreal scale = 1.0f;
    Q_Q(const QAbstractItemView);
    QWidget *window = q->window();
    if (window) {
        QWindow *windowHandle = window->windowHandle();
        if (windowHandle)
            scale = windowHandle->devicePixelRatio();
    }
#else
    QWindow *window = windowHandle(WindowHandleMode::Closest);
    const qreal scale = window ? window->devicePixelRatio() : qreal(1);
#endif

    QPixmap pixmap(r->size() * scale);
    pixmap.setDevicePixelRatio(scale);

    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option = viewOptionsV1();
    option.state |= QStyle::State_Selected;
    for (int j = 0; j < paintPairs.count(); ++j) {
        option.rect = paintPairs.at(j).rect.translated(-r->topLeft());
        const QModelIndex &current = paintPairs.at(j).index;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        Q_Q(const QAbstractItemView);
        adjustViewOptionsForIndex(&option, current);
        q->itemDelegateForIndex(current)->paint(&painter, option, current);
#else
        delegateForIndex(current)->paint(&painter, option, current);
#endif
    }
    return pixmap;
}

QStyleOptionViewItem NoteListViewPrivate::viewOptionsV1() const
{
    Q_Q(const NoteListView);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QStyleOptionViewItem option;
    q->initViewItemOption(&option);
    return option;
#else
    return q->viewOptions();
#endif
}

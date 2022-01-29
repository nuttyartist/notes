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
#include <QMetaObject>
#include "tagpool.h"
#include "notelistmodel.h"
#include "nodepath.h"
#include "dbmanager.h"
#include "notelistview_p.h"

NoteListView::NoteListView(QWidget *parent)
    : QListView( parent ),
      m_isScrollBarHidden(true),
      m_animationEnabled(true),
      m_isMousePressed(false),
      m_rowHeight(38),
      m_currentBackgroundColor(247, 247, 247),
      m_tagPool(nullptr),
      m_dbManager(nullptr),
      m_currentFolderId{SpecialNodeID::InvalidNodeId},
      m_isInTrash{false}
{
    this->setAttribute(Qt::WA_MacShowFocusRect, 0);

    QTimer::singleShot(0, this, SLOT(init()));
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested,
            this, &NoteListView::onCustomContextMenu);
    contextMenu = new QMenu(this);
    tagsMenu = new QMenu(this);
    addToTagAction = new QAction(tr("Tags..."), this);
    connect(addToTagAction, &QAction::triggered, this, [this] {
        onTagsMenu(visualRect(currentIndex()).bottomLeft());
    });

    deleteNoteAction = new QAction(tr("Delete Note"), this);
    connect(deleteNoteAction, &QAction::triggered, this, [this] {
        auto index = currentIndex();
        if (index.isValid()) {
            emit deleteNoteRequested(index);
        }
    });
    restoreNoteAction = new QAction(tr("Restore Note"), this);
    connect(restoreNoteAction, &QAction::triggered, this, [this] {
        auto index = currentIndex();
        if (index.isValid()) {
            emit restoreNoteRequested(index);
        }
    });
    pinNoteAction = new QAction(tr("Pin Note"), this);
    connect(pinNoteAction, &QAction::triggered, this, [this] {
        auto index = currentIndex();
        if (index.isValid()) {
            auto id = index.data(NoteListModel::NoteID).toInt();
            emit setPinnedNoteRequested(id, true);
        }
    });
    unpinNoteAction = new QAction(tr("Unpin Note"), this);
    connect(unpinNoteAction, &QAction::triggered, this, [this] {
        auto index = currentIndex();
        if (index.isValid()) {
            auto id = index.data(NoteListModel::NoteID).toInt();
            emit setPinnedNoteRequested(id, false);
        }
    });

    newNoteAction = new QAction(tr("New Note"), this);
    connect(newNoteAction, &QAction::triggered, this, [this] {
        emit newNoteRequested();
    });

    m_dragPixmap.load("qrc:/images/notes_icon.icns");
    setDragEnabled(true);
    setAcceptDrops(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

NoteListView::~NoteListView()
{
}

void NoteListView::animateAddedRow(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(end)

    QModelIndex idx = model()->index(start,0);
    // Note: this line add flikering, seen when the animation runs slow
    selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);

    NoteListDelegate* delegate = static_cast<NoteListDelegate*>(itemDelegate());
    if(delegate != Q_NULLPTR){
        delegate->setState( NoteListDelegate::Insert, idx);

        // TODO find a way to finish this function till the animation stops
        while(delegate->animationState() == QTimeLine::Running){
            qApp->processEvents();
        }
    }
}

/**
 * @brief Reimplemented from QWidget::paintEvent()
 */
void NoteListView::paintEvent(QPaintEvent *e)
{
    NoteListDelegate* delegate = static_cast<NoteListDelegate*>(itemDelegate());
    if(delegate != Q_NULLPTR)
        delegate->setCurrentSelectedIndex(currentIndex());

    QListView::paintEvent(e);
}

/**
 * @brief Reimplemented from QAbstractItemView::rowsInserted().
 */
void NoteListView::rowsInserted(const QModelIndex &parent, int start, int end)
{

    if(start == end && m_animationEnabled)
        animateAddedRow(parent, start, end);

    QListView::rowsInserted(parent, start, end);
}

/**
 * @brief Reimplemented from QAbstractItemView::rowsAboutToBeRemoved().
 */
void NoteListView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    if(start == end){
        NoteListDelegate* delegate = static_cast<NoteListDelegate*>(itemDelegate());
        if(delegate != Q_NULLPTR){
            QModelIndex idx = model()->index(start,0);
            delegate->setCurrentSelectedIndex(QModelIndex());

            if(m_animationEnabled){
                delegate->setState( NoteListDelegate::Remove, idx);
            }else{
                delegate->setState( NoteListDelegate::Normal, idx);
            }

            // TODO find a way to finish this function till the animation stops
            while(delegate->animationState() == QTimeLine::Running){
                qApp->processEvents();
            }
        }
    }

    QListView::rowsAboutToBeRemoved(parent, start, end);
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
    openPersistentEditor(index);
    m_openedEditor.insert(index);
}

void NoteListView::closePersistentEditorC(const QModelIndex &index)
{
    closePersistentEditor(index);
    m_openedEditor.remove(index);
}

void NoteListView::closeAllEditor()
{
    for (const auto& index : QT_AS_CONST(m_openedEditor)) {
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

void NoteListView::rowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                                      const QModelIndex &destinationParent, int destinationRow)
{
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destinationParent)
    Q_UNUSED(destinationRow)

    if(model() != Q_NULLPTR){
        QModelIndex idx = model()->index(sourceStart,0);
        NoteListDelegate* delegate = static_cast<NoteListDelegate*>(itemDelegate());
        if(delegate != Q_NULLPTR){

            if(m_animationEnabled){
                delegate->setState( NoteListDelegate::MoveOut, idx);
            }else{
                delegate->setState( NoteListDelegate::Normal, idx);
            }

            // TODO find a way to finish this function till the animation stops
            while(delegate->animationState() == QTimeLine::Running){
                qApp->processEvents();
            }
        }
    }
}

void NoteListView::rowsMoved(const QModelIndex &parent, int start, int end,
                             const QModelIndex &destination, int row)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    Q_UNUSED(destination)

    QModelIndex idx = model()->index(row,0);
    setCurrentIndex(idx);

    NoteListDelegate* delegate = static_cast<NoteListDelegate*>(itemDelegate());
    if(delegate == Q_NULLPTR)
        return;

    if(m_animationEnabled){
        delegate->setState( NoteListDelegate::MoveIn, idx );
    }else{
        delegate->setState( NoteListDelegate::Normal, idx);
    }

    // TODO find a way to finish this function till the animation stops
    while(delegate->animationState() == QTimeLine::Running){
        qApp->processEvents();
    }
}

void NoteListView::init()
{
    setMouseTracking(true);
    setUpdatesEnabled(true);
    viewport()->setAttribute(Qt::WA_Hover);

    setupStyleSheet();
    setupSignalsSlots();
}

void NoteListView::mouseMoveEvent(QMouseEvent* event)
{
    if(!m_isMousePressed) {
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

void NoteListView::mousePressEvent(QMouseEvent* e)
{
    m_isMousePressed = true;
    if (e->button() == Qt::LeftButton) {
        m_dragStartPosition = e->pos();
    }
    QListView::mousePressEvent(e);
}

void NoteListView::mouseReleaseEvent(QMouseEvent*e)
{
    m_isMousePressed = false;
    QListView::mouseReleaseEvent(e);
}

bool NoteListView::viewportEvent(QEvent*e)
{
    if(model() != Q_NULLPTR){
        switch (e->type()) {
        case QEvent::Leave:{
            QPoint pt = mapFromGlobal(QCursor::pos());
            QModelIndex index = indexAt(QPoint(10, pt.y()));
            if(index.row() > 0){
                index = model()->index(index.row()-1, 0);
                NoteListDelegate* delegate = static_cast<NoteListDelegate*>(itemDelegate());
                if(delegate != Q_NULLPTR){
                    delegate->setHoveredIndex(QModelIndex());
                    viewport()->update(visualRect(index));
                }
            }
            break;
        }
        case QEvent::MouseButtonPress:{
            QPoint pt = mapFromGlobal(QCursor::pos());
            QModelIndex index = indexAt(QPoint(10, pt.y()));
            if(!index.isValid()) {
                emit viewportPressed();
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
        bool ok = false;
        auto nodeId = QString::fromUtf8(
                    event->mimeData()->data(NOTE_MIME)).toInt(&ok);
        if (ok) {
            auto model = dynamic_cast<NoteListModel*>(this->model());
            if (model) {
                auto noteIndex = model->getNoteIndex(nodeId);
                if (noteIndex.data(NoteListModel::NoteIsPinned).toBool()) {
                    auto firstUnpinned = model->firstUnpinnedIndex();
                    if (event->pos().y() <= visualRect(firstUnpinned).y() - 25) {
                        event->acceptProposedAction();
                        setDropIndicatorShown(true);
                        QListView::dragMoveEvent(event);
                        return;
                    }
                }
            }
        }
    } else {
        event->ignore();
    }
}

void NoteListView::scrollContentsBy(int dx, int dy)
{
    QListView::scrollContentsBy(dx, dy);
    auto m_listModel = dynamic_cast<NoteListModel*>(model());
    if (!m_listModel) {
        return;
    }
    for (int i = 0; i < m_listModel->rowCount(); ++i) {
        auto index = m_listModel->index(i, 0);
        if (m_openedEditor.contains(index)) {
            auto y = visualRect(index).y();
            auto range = abs(viewport()->height());
            if ((y < -range) || (y > 2 * range)) {
                m_openedEditor.remove(index);
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

void NoteListView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);
    Q_D(NoteListView);
    auto current = currentIndex();
    if (current.isValid()) {
        auto indexes = QModelIndexList{current};
        QMimeData *data = d->model->mimeData(QModelIndexList{indexes});
        if (!data) {
            return;
        }
        QRect rect;
        QPixmap pixmap = d->renderToPixmap(indexes, &rect);
        rect.adjust(horizontalOffset(), verticalOffset(), 0, 0);
        QDrag *drag = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setMimeData(data);
        drag->setHotSpot(d->pressedPosition - rect.topLeft());
        Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
        /// Delete later, if there is no drop event.
        if(dropAction == Qt::IgnoreAction){
            drag->deleteLater();
            data->deleteLater();
        }

        d->dropEventMoved = false;
        // Reset the drop indicator
        d->dropIndicatorRect = QRect();
        d->dropIndicatorPosition = OnItem;
    }
}

void NoteListView::setCurrentRowActive(bool isActive)
{
    NoteListDelegate* delegate = static_cast<NoteListDelegate*>(itemDelegate());
    if(delegate == Q_NULLPTR)
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
    connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this]
            (const QModelIndex & current, const QModelIndex & previous){

        if(model() != Q_NULLPTR){
            if(current.row() < previous.row()){
                if(current.row() > 0){
                    QModelIndex prevIndex = model()->index(current.row()-1, 0);
                    viewport()->update(visualRect(prevIndex));
                }
            }

            if(current.row() > 1){
                QModelIndex prevPrevIndex = model()->index(current.row()-2, 0);
                viewport()->update(visualRect(prevPrevIndex));
            }
        }
    });

    // row was entered
    connect(this, &NoteListView::entered, this, [this](QModelIndex index){
        if(model() != Q_NULLPTR){
            if(index.row() > 1){
                QModelIndex prevPrevIndex = model()->index(index.row()-2, 0);
                viewport()->update(visualRect(prevPrevIndex));

                QModelIndex prevIndex = model()->index(index.row()-1, 0);
                viewport()->update(visualRect(prevIndex));

            }else if(index.row() > 0){
                QModelIndex prevIndex = model()->index(index.row()-1, 0);
                viewport()->update(visualRect(prevIndex));
            }

            NoteListDelegate* delegate = static_cast<NoteListDelegate *>(itemDelegate());
            if(delegate != Q_NULLPTR)
                delegate->setHoveredIndex(index);
        }
    });

    // viewport was entered
    connect(this, &NoteListView::viewportEntered, this, [this](){
        if(model() != Q_NULLPTR && model()->rowCount() > 1){
            NoteListDelegate* delegate = static_cast<NoteListDelegate *>(itemDelegate());
            if(delegate != Q_NULLPTR)
                delegate->setHoveredIndex(QModelIndex());

            QModelIndex lastIndex = model()->index(model()->rowCount()-2, 0);
            viewport()->update(visualRect(lastIndex));
        }
    });

    // remove/add offset right side
    connect(this->verticalScrollBar(), &QScrollBar::rangeChanged, this, [this](int min, int max){
        Q_UNUSED(min)

        NoteListDelegate* delegate = static_cast<NoteListDelegate*>(itemDelegate());
        if(delegate != Q_NULLPTR){
            if(max > 0){
                delegate->setRowRightOffset(2);
            }else{
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
#if defined(Q_OS_LINUX)
    QString ss = QString("QListView {background-color: %1;} "
                         "QScrollBar::handle:vertical:hover { background: rgb(170, 170, 171); } "
                         "QScrollBar::handle:vertical:pressed { background: rgb(149, 149, 149); } "
                         "QScrollBar::handle:vertical { border-radius: 4px; background: rgb(188, 188, 188); min-height: 20px; }  "
                         "QScrollBar::vertical {border-radius: 4px; width: 8px; color: rgba(255, 255, 255,0);} "
                         "QScrollBar {margin: 0; background: transparent;} "
                         "QScrollBar:hover { background-color: rgb(217, 217, 217);}"
                         "QScrollBar::add-line:vertical { width:0px; height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  "
                         "QScrollBar::sub-line:vertical { width:0px; height: 0px; subcontrol-position: top; subcontrol-origin: margin; }"
                         ).arg(m_currentBackgroundColor.name());
#else
    QString ss = QString("QListView {background-color: %1;} "
                         ).arg(m_currentBackgroundColor.name());
#endif
    setStyleSheet(ss);
}

void NoteListView::addCurrentNoteToTag(int tagId)
{
    auto current = currentIndex();
    if (current.isValid()) {
        emit addTagRequested(current, tagId);
    }
}

void NoteListView::removeCurrentNoteFromTag(int tagId)
{
    auto current = currentIndex();
    if (current.isValid()) {
        emit removeTagRequested(current, tagId);
    }
}

/**
 * @brief Set theme color for noteView
 */
void NoteListView::setTheme(Theme theme)
{
    switch(theme){
    case Theme::Light:
    {
        m_currentBackgroundColor = QColor(247, 247, 247);
        break;
    }
    case Theme::Dark:
    {
        m_currentBackgroundColor = QColor(26, 26, 26);
        break;
    }
    case Theme::Sepia:
    {
        m_currentBackgroundColor = QColor(251, 240, 217);
        break;
    }
    }

    setupStyleSheet();
}

void NoteListView::onCustomContextMenu(const QPoint &point)
{
    QModelIndex index = indexAt(point);
    if (index.isValid()) {
        contextMenu->clear();
        if (m_tagPool) {
            contextMenu->addAction(addToTagAction);
            contextMenu->addSeparator();
        }
        if (m_isInTrash) {
            contextMenu->addAction(restoreNoteAction);
        }
        contextMenu->addAction(deleteNoteAction);
        if ((!m_listViewInfo.isInTag) && (m_listViewInfo.parentFolderId != SpecialNodeID::TrashFolder)) {
            contextMenu->addSeparator();
            auto isPinned = index.data(NoteListModel::NoteIsPinned).toBool();
            if (!isPinned) {
                contextMenu->addAction(pinNoteAction);
            } else {
                contextMenu->addAction(unpinNoteAction);
            }
        }
        contextMenu->addSeparator();
        if (m_dbManager) {
            for (auto action : QT_AS_CONST(m_folderActions)) {
                delete action;
            }
            m_folderActions.clear();
            auto m = contextMenu->addMenu("Move to");
            FolderListType folders;
            QMetaObject::invokeMethod(m_dbManager, "getFolderList", Qt::BlockingQueuedConnection,
                                      Q_RETURN_ARG(FolderListType, folders)
                                      );
            for (const auto& id: folders.keys()) {
                if (id == m_currentFolderId) {
                    continue;
                }
                auto action = new QAction(folders[id], this);
                connect(action, &QAction::triggered, this, [this, id] {
                    auto index = currentIndex();
                    if (index.isValid()) {
                        emit moveNoteRequested(index.data(NoteListModel::NoteID).toInt(), id);
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

void NoteListView::onTagsMenu(const QPoint &point)
{
    tagsMenu->clear();
    for (auto action : QT_AS_CONST(m_noteTagActions)) {
        delete action;
    }
    m_noteTagActions.clear();
    auto current = currentIndex();
    auto createTagIcon = [](const QString& color) -> QIcon{
        QPixmap pix{32, 32};
        pix.fill(Qt::transparent);
        QPainter painter{&pix};
        painter.setRenderHint(QPainter::Antialiasing);
        auto iconRect = QRect((pix.width() - 30) / 2,
                              (pix.height() - 30) / 2, 30, 30);
        painter.setBrush(QColor(color));
        painter.setPen(QColor(color));
        painter.drawEllipse(iconRect);
        return QIcon{pix};
    };
    if (current.isValid()) {
        if (m_tagPool) {
            auto tagInNote = current.data(NoteListModel::NoteTagsList).value<QSet<int>>();
            for (auto id : QT_AS_CONST(tagInNote)) {
                auto tag = m_tagPool->getTag(id);
                auto tagAction = new QAction(QString("Remove tag ") + tag.name(), this);
                connect(tagAction, &QAction::triggered, this, [this, id] {
                    removeCurrentNoteFromTag(id);
                });
                tagAction->setIcon(createTagIcon(tag.color()));
                tagsMenu->addAction(tagAction);
                m_noteTagActions.append(tagAction);
            }
            tagsMenu->addSeparator();
            const auto tagIds = m_tagPool->tagIds();
            for (auto id : tagIds) {
                if (tagInNote.contains(id)) {
                    continue;
                }
                auto tag = m_tagPool->getTag(id);
                auto tagAction = new QAction(QString("Assign tag ") + tag.name(), this);
                connect(tagAction, &QAction::triggered, this, [this, id] {
                    addCurrentNoteToTag(id);
                });
                tagAction->setIcon(createTagIcon(tag.color()));
                tagsMenu->addAction(tagAction);
                m_noteTagActions.append(tagAction);
            }
            tagsMenu->exec(viewport()->mapToGlobal(point));
        } else {
            qDebug() << __FUNCTION__ << "tag pool is not init yet";
        }
    }
}

QPixmap NoteListViewPrivate::renderToPixmap(const QModelIndexList &indexes, QRect *r) const
{
    Q_ASSERT(r);
    QItemViewPaintPairs paintPairs = draggablePaintPairs(indexes, r);
    if (paintPairs.isEmpty())
        return QPixmap();

    QWindow *window = windowHandle(WindowHandleMode::Closest);
    const qreal scale = window ? window->devicePixelRatio() : qreal(1);

    QPixmap pixmap(r->size() * scale);
    pixmap.setDevicePixelRatio(scale);

    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option = viewOptionsV1();
    option.state |= QStyle::State_Selected;
    for (int j = 0; j < paintPairs.count(); ++j) {
        option.rect = paintPairs.at(j).rect.translated(-r->topLeft());
        const QModelIndex &current = paintPairs.at(j).index;
        adjustViewOptionsForIndex(&option, current);
        delegateForIndex(current)->paint(&painter, option, current);
    }
    return pixmap;
}

QStyleOptionViewItem NoteListViewPrivate::viewOptionsV1() const
{
    Q_Q(const NoteListView);
    return q->viewOptions();
}

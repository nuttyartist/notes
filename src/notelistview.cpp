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
#include "tagpool.h"
#include "notelistmodel.h"
#include "nodepath.h"

NoteListView::NoteListView(QWidget *parent)
    : QListView( parent ),
      m_isScrollBarHidden(true),
      m_animationEnabled(true),
      m_isMousePressed(false),
      m_rowHeight(38),
      m_currentBackgroundColor(255, 255, 255),
      m_tagPool(nullptr),
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
    m_dragPixmap.load("qrc:/images/notes_icon.icns");
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
    }
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - m_dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    auto current = currentIndex();
    mimeData->setData(NOTE_MIME, QString::number(
                          current.data(NoteListModel::NoteID).toInt()).toUtf8());
    drag->setMimeData(mimeData);
    drag->setPixmap(m_dragPixmap);
    drag->exec(Qt::MoveAction);
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
        m_currentBackgroundColor = QColor(255, 255, 255);
        break;
    }
    case Theme::Dark:
    {
        m_currentBackgroundColor = QColor(16, 16, 16);
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
        contextMenu->addAction(addToTagAction);
        contextMenu->addSeparator();
        if (m_isInTrash) {
            contextMenu->addAction(restoreNoteAction);
        }
        contextMenu->addAction(deleteNoteAction);
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

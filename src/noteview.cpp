#include "noteview.h"
#include "notewidgetdelegate.h"
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QAbstractItemView>
#include <QPaintEvent>
#include <QSortFilterProxyModel>
#include <QTimer>

NoteView::NoteView(QWidget *parent)
    : QListView( parent ),
      m_isScrollBarHidden(true),
      m_isSearching(false),
      m_isMousePressed(false),
      rowHeight(38)
{
    setMouseTracking(true);
    setUpdatesEnabled(true);
    viewport()->setAttribute(Qt::WA_Hover);

    setupStyleSheet();
    QTimer::singleShot(0, this, SLOT(setupSignalsSlots()));
}

NoteView::~NoteView()
{
}

void NoteView::animateAddedRow(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(end)

    QModelIndex idx = model()->index(start,0);
    selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
    NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate(idx));
    delegate->setState( NoteWidgetDelegate::Insert, idx);

    // TODO find a way to finish this function till the animation stops
    while(delegate->animationState() == QTimeLine::Running){
        qApp->processEvents();
    }
}

/**
 * @brief Reimplemented from QWidget::paintEvent()
 */
void NoteView::paintEvent(QPaintEvent *e)
{
    NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate *>(itemDelegate());
    delegate->setCurrentSelectedIndex(currentIndex());

    QPoint pos = mapFromGlobal(QCursor::pos());
    QModelIndex index = indexAt(pos);
    delegate->setHoveredIndex(index);

    QListView::paintEvent(e);
}

/**
 * @brief Reimplemented from QAbstractItemView::rowsInserted().
 */
void NoteView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QSortFilterProxyModel *proxyModel = static_cast<QSortFilterProxyModel *>(model());

    if(start == end && !m_isSearching)
        animateAddedRow(parent, start, end);

    m_isSearching = !proxyModel->filterRegExp().pattern().isEmpty();

    QListView::rowsInserted(parent, start, end);
}

/**
 * @brief Reimplemented from QAbstractItemView::rowsAboutToBeRemoved().
 */
void NoteView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    QSortFilterProxyModel *proxyModel = static_cast<QSortFilterProxyModel *>(model());
    m_isSearching = !proxyModel->filterRegExp().pattern().isEmpty();

    if(start == end){
        QModelIndex idx = model()->index(start,0);
        NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate(idx));
        delegate->setCurrentSelectedIndex(QModelIndex());

        if(!m_isSearching){
            delegate->setState( NoteWidgetDelegate::Remove, idx);
        }else{
            delegate->setState( NoteWidgetDelegate::Normal, idx);
        }

        // TODO find a way to finish this function till the animation stops
        while(delegate->animationState() == QTimeLine::Running){
            qApp->processEvents();
        }
    }

    QListView::rowsAboutToBeRemoved(parent, start, end);
}

void NoteView::rowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                                  const QModelIndex &destinationParent, int destinationRow)
{
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destinationParent)
    Q_UNUSED(destinationRow)

    QModelIndex idx = model()->index(sourceStart,0);
    NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate(idx));
    delegate->setState( NoteWidgetDelegate::MoveOut, idx);

    // TODO find a way to finish this function till the animation stops
    while(delegate->animationState() == QTimeLine::Running){
        qApp->processEvents();
    }
}

void NoteView::rowsMoved(const QModelIndex &parent, int start, int end,
                         const QModelIndex &destination, int row)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    Q_UNUSED(destination)

    QModelIndex idx = model()->index(row,0);
    setCurrentIndex(idx);

    NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate(idx));
    delegate->setState( NoteWidgetDelegate::MoveIn, idx );

    // TODO find a way to finish this function till the animation stops
    while(delegate->animationState() == QTimeLine::Running){
        qApp->processEvents();
    }
}

void NoteView::mouseMoveEvent(QMouseEvent*e)
{
    if(m_isMousePressed){

    }else{
        QListView::mouseMoveEvent(e);
    }
}

void NoteView::mousePressEvent(QMouseEvent*e)
{
    m_isMousePressed = true;

    QListView::mousePressEvent(e);
}

void NoteView::mouseReleaseEvent(QMouseEvent*e)
{
    m_isMousePressed = false;
    QListView::mouseReleaseEvent(e);
}

void NoteView::setupSignalsSlots()
{
    connect(this, &NoteView::entered,[this](QModelIndex index){
        QModelIndex prevPrevIndex = model()->index(index.row()-2, 0);
        viewport()->update(visualRect(prevPrevIndex));
        QModelIndex prevIndex = model()->index(index.row()-1, 0);
        viewport()->update(visualRect(prevIndex));
    });

    connect(this, &NoteView::viewportEntered,[this](){
        QModelIndex lastIndex = model()->index(model()->rowCount()-2, 0);
        viewport()->update(visualRect(lastIndex));
    });
}

/**
 * @brief setup styleSheet
 */
void NoteView::setupStyleSheet()
{
    QString ss = QString("QListView QWidget{background-color:white;} "
                         "QScrollBar {margin-right: 2px; background: transparent;} "
                         "QScrollBar:hover { background-color: rgb(217, 217, 217);}"
                         "QScrollBar:handle:vertical:hover { background: rgb(170, 170, 171); } "
                         "QScrollBar:handle:vertical:pressed { background: rgb(149, 149, 149);}"
                         "QScrollBar:vertical { border: none; width: 10px; border-radius: 4px;} "
                         "QScrollBar::handle:vertical { border-radius: 4px; background: rgb(188, 188, 188); min-height: 20px; }  "
                         "QScrollBar::add-line:vertical { height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  "
                         "QScrollBar::sub-line:vertical { height: 0px; subcontrol-position: top; subcontrol-origin: margin; }"
                         );

    setStyleSheet(ss);
}

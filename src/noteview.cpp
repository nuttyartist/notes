#include "noteview.h"
#include "notewidgetdelegate.h"
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QAbstractItemView>
#include <QPaintEvent>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QScrollBar>

NoteView::NoteView(QWidget *parent)
    : QListView( parent ),
      m_isScrollBarHidden(true),
      m_isSearching(false),
      m_isMousePressed(false),
      m_rowHeight(38)
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
    // Note: this line add flikering, seen when the animation runs slow
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
    if(start == end && !m_isSearching)
        animateAddedRow(parent, start, end);

    QListView::rowsInserted(parent, start, end);
}

/**
 * @brief Reimplemented from QAbstractItemView::rowsAboutToBeRemoved().
 */
void NoteView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    if(start == end && !m_isSearching){
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

void NoteView::setSearching(bool isSearching)
{
    m_isSearching = isSearching;
}

void NoteView::setupSignalsSlots()
{
    // remove/add separator
    connect(selectionModel(), &QItemSelectionModel::currentRowChanged, [this]
            (const QModelIndex & current, const QModelIndex & previous){

        QModelIndex prevPrevIndex = model()->index(current.row()-2, 0);
        QModelIndex prevIndex = model()->index(current.row()-1, 0);

        if(current.row() < previous.row()){
            viewport()->update(visualRect(prevPrevIndex));
            viewport()->update(visualRect(prevIndex));
        }else{
            viewport()->update(visualRect(prevPrevIndex));
        }
    });

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

    // remove/add offset
    connect(this->verticalScrollBar(), &QScrollBar::rangeChanged,[this](int min, int max){
        Q_UNUSED(min)
        NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate());
        if(max > 0){
            delegate->setRowRightOffset(2);
        }else{
            delegate->setRowRightOffset(0);
        }
        viewport()->update();
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

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
      m_animationEnabled(true),
      m_isMousePressed(false),
      m_rowHeight(38),
      m_currentBackgroundColor(255, 255, 255)
{
    this->setAttribute(Qt::WA_MacShowFocusRect, 0);

    QTimer::singleShot(0, this, SLOT(init()));
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

    NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate());
    if(delegate != Q_NULLPTR){
        delegate->setState( NoteWidgetDelegate::Insert, idx);

        // TODO find a way to finish this function till the animation stops
        while(delegate->animationState() == QTimeLine::Running){
            qApp->processEvents();
        }
    }
}

/**
 * @brief Reimplemented from QWidget::paintEvent()
 */
void NoteView::paintEvent(QPaintEvent *e)
{
    NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate());
    if(delegate != Q_NULLPTR)
        delegate->setCurrentSelectedIndex(currentIndex());

    QListView::paintEvent(e);
}

/**
 * @brief Reimplemented from QAbstractItemView::rowsInserted().
 */
void NoteView::rowsInserted(const QModelIndex &parent, int start, int end)
{

    if(start == end && m_animationEnabled)
        animateAddedRow(parent, start, end);

    QListView::rowsInserted(parent, start, end);
}

/**
 * @brief Reimplemented from QAbstractItemView::rowsAboutToBeRemoved().
 */
void NoteView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    if(start == end){
        NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate());
        if(delegate != Q_NULLPTR){
            QModelIndex idx = model()->index(start,0);
            delegate->setCurrentSelectedIndex(QModelIndex());

            if(m_animationEnabled){
                delegate->setState( NoteWidgetDelegate::Remove, idx);
            }else{
                delegate->setState( NoteWidgetDelegate::Normal, idx);
            }

            // TODO find a way to finish this function till the animation stops
            while(delegate->animationState() == QTimeLine::Running){
                qApp->processEvents();
            }
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

    if(model() != Q_NULLPTR){
        QModelIndex idx = model()->index(sourceStart,0);
        NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate());
        if(delegate != Q_NULLPTR){

            if(m_animationEnabled){
                delegate->setState( NoteWidgetDelegate::MoveOut, idx);
            }else{
                delegate->setState( NoteWidgetDelegate::Normal, idx);
            }

            // TODO find a way to finish this function till the animation stops
            while(delegate->animationState() == QTimeLine::Running){
                qApp->processEvents();
            }
        }
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

    NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate());
    if(delegate == Q_NULLPTR)
        return;

    if(m_animationEnabled){
        delegate->setState( NoteWidgetDelegate::MoveIn, idx );
    }else{
        delegate->setState( NoteWidgetDelegate::Normal, idx);
    }

    // TODO find a way to finish this function till the animation stops
    while(delegate->animationState() == QTimeLine::Running){
        qApp->processEvents();
    }
}

void NoteView::init()
{
    setMouseTracking(true);
    setUpdatesEnabled(true);
    viewport()->setAttribute(Qt::WA_Hover);

    setupStyleSheet();
    setupSignalsSlots();
}

void NoteView::mouseMoveEvent(QMouseEvent*e)
{
    if(!m_isMousePressed)
        QListView::mouseMoveEvent(e);
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

bool NoteView::viewportEvent(QEvent*e)
{
    if(model() != Q_NULLPTR){
        switch (e->type()) {
        case QEvent::Leave:{
            QPoint pt = mapFromGlobal(QCursor::pos());
            QModelIndex index = indexAt(QPoint(10, pt.y()));
            if(index.row() > 0){
                index = model()->index(index.row()-1, 0);
                NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate());
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
            if(!index.isValid())
                emit viewportPressed();

            break;
        }
        default:
            break;
        }
    }

    return QListView::viewportEvent(e);
}

void NoteView::setCurrentRowActive(bool isActive)
{
    NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate());
    if(delegate == Q_NULLPTR)
        return;

    delegate->setActive(isActive);
    viewport()->update(visualRect(currentIndex()));
}

void NoteView::setAnimationEnabled(bool isEnabled)
{
    m_animationEnabled = isEnabled;
}

void NoteView::setupSignalsSlots()
{
    // remove/add separator
    // current selectected row changed
    connect(selectionModel(), &QItemSelectionModel::currentRowChanged, [this]
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
    connect(this, &NoteView::entered,[this](QModelIndex index){
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

            NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate *>(itemDelegate());
            if(delegate != Q_NULLPTR)
                delegate->setHoveredIndex(index);
        }
    });

    // viewport was entered
    connect(this, &NoteView::viewportEntered,[this](){
        if(model() != Q_NULLPTR && model()->rowCount() > 1){
            NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate *>(itemDelegate());
            if(delegate != Q_NULLPTR)
                delegate->setHoveredIndex(QModelIndex());

            QModelIndex lastIndex = model()->index(model()->rowCount()-2, 0);
            viewport()->update(visualRect(lastIndex));
        }
    });

    // remove/add offset right side
    connect(this->verticalScrollBar(), &QScrollBar::rangeChanged,[this](int min, int max){
        Q_UNUSED(min)

        NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate());
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
void NoteView::setupStyleSheet()
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

/**
 * @brief Set theme color for noteView
 */
void NoteView::setTheme(Theme theme)
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

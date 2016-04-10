#include "noteview.h"
#include "notewidgetdelegate.h"
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QAbstractItemView>

NoteView::NoteView(QWidget *parent)
    : QListView( parent ),
      m_isScrollBarHidden(true),
      rowHeight(38)
{
    setMouseTracking(true);
    setUpdatesEnabled(true);
    viewport()->setAttribute(Qt::WA_Hover);

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

    updateStyleSheet();
}

NoteView::~NoteView()
{
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

    // update the stylesheet
    int rc = model()->rowCount();
    int contentHeight =  rc * rowHeight;

    if((contentHeight > height()
        && m_isScrollBarHidden)
            ||(contentHeight < height()
               && !m_isScrollBarHidden)){

        updateStyleSheet();
    }
}


/**
 * @brief Reimplemented from QAbstractItemView::rowsInserted().
 */
void NoteView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    if(start == end){
        QModelIndex idx = model()->index(start,0);
        NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate(idx));
        delegate->setState( NoteWidgetDelegate::Insert, idx );
    }

    QListView::rowsInserted(parent, start, end);
}

/**
 * @brief Reimplemented from QAbstractItemView::rowsAboutToBeRemoved().
 */
void NoteView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    QModelIndex idx = model()->index(start,0);
    NoteWidgetDelegate* delegate = static_cast<NoteWidgetDelegate*>(itemDelegate(idx));
    delegate->setState( NoteWidgetDelegate::Remove, idx);

    // TODO find a way to finish this function till the animation stops
    while(delegate->animationState() == QTimeLine::Running){
        qApp->processEvents();
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

/**
 * @brief Reimplemented from QWidget::resizeEvent()
 */
void NoteView::resizeEvent(QResizeEvent *e)
{
    QListView::resizeEvent(e);

    int rc = model()->rowCount();
    int contentHeight =  rc * rowHeight;

    if((contentHeight > height()
        && m_isScrollBarHidden)
            ||(contentHeight < height()
               && !m_isScrollBarHidden)){

        updateStyleSheet();
    }
}

/**
 * @brief NoteView::updateStyleSheet updates the styleSheet of the vertical scrollbar
 * hide the scrollbar if they are less notes than what visible area can show
 */
void NoteView::updateStyleSheet()
{
    QString verticalBg = "background-color: transparent";
    QString handleBg = "background-color: transparent";

    if(model() != Q_NULLPTR){
        int ctHeight = rowHeight * model()->rowCount();

        m_isScrollBarHidden = ctHeight < height();

        verticalBg = ctHeight > height() ? "" : "background-color: transparent";
        handleBg = ctHeight > height() ? "background: rgb(188, 188, 188)"
                                       : "background-color: transparent";
    }

    QString ss = QString("QListView QWidget{background-color:white;} "
                         "QScrollBar {margin-right: 2px; background: transparent;} "
                         "QScrollBar:hover { background-color: rgb(217, 217, 217);}"
                         "QScrollBar:handle:vertical:hover { background: rgb(170, 170, 171); } "
                         "QScrollBar:handle:vertical:pressed { background: rgb(149, 149, 149);}"
                         "QScrollBar:vertical { border: none; width: 10px; border-radius: 4px;%1;} "
                         "QScrollBar::handle:vertical { border-radius: 4px; %2; min-height: 20px; }  "
                         "QScrollBar::add-line:vertical { height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  "
                         "QScrollBar::sub-line:vertical { height: 0px; subcontrol-position: top; subcontrol-origin: margin; }"
                         )
            .arg(verticalBg)
            .arg(handleBg);

    setStyleSheet(ss);
}

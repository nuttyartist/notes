#include "notewidgetdelegate.h"
#include "noteview.h"
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include "notemodel.h"

NoteWidgetDelegate::NoteWidgetDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
      m_titleFont(QFont(QStringLiteral("Liberation Sans"), 10,QFont::Bold)),
      m_dateFont(QFont(QStringLiteral("Liberation Sans"), 8)),
      m_titleColor(0, 0, 0),
      m_dateColor(132, 132, 132),
      m_focusColor(254, 206, 9),
      m_noFocusColor(255, 235, 80),
      m_hoverColor(207, 207, 207),
      m_separatorColor(221, 221, 221),
      m_defaultColor(255,255,255),
      m_rowHeight(38),
      m_maxFrame(200),
      m_state(Normal)
{
    m_timeLine = new QTimeLine(300, this);
    m_timeLine->setFrameRange(0,m_maxFrame);
    m_timeLine->setUpdateInterval(10);
    m_timeLine->setCurveShape(QTimeLine::EaseInOutCurve);

    connect( m_timeLine, &QTimeLine::frameChanged, [this](){
        emit sizeHintChanged(m_animatedIndex);
    });

    connect(m_timeLine, &QTimeLine::finished, [this](){
        m_animatedIndex = QModelIndex();
        m_state = Normal;
    });
}

void NoteWidgetDelegate::setState(States NewState, QModelIndex index)
{
    m_animatedIndex = index;

    auto startAnimation = [this](QTimeLine::Direction diretion, int duration){
        m_timeLine->setDirection(diretion);
        m_timeLine->setDuration(duration);
        m_timeLine->start();
    };

    switch ( NewState ){
    case Insert:
        startAnimation(QTimeLine::Forward, m_maxFrame);
        break;
    case Remove:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case MoveOut:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case MoveIn:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case Normal:
        m_animatedIndex = QModelIndex();
        break;
    }

    m_state = NewState;
}

void NoteWidgetDelegate::setAnimationDuration(const int duration)
{
    m_timeLine->setDuration(duration);
}

void NoteWidgetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    QStyleOptionViewItem opt = option;
    opt.rect.setWidth(option.rect.width()-2);

    int currentFrame = m_timeLine->currentFrame();
    double rate = (currentFrame/(double)m_maxFrame);
    double height = m_rowHeight * rate;

    switch(m_state){
    case Insert:
    case Remove:
    case MoveOut:
        if(index == m_animatedIndex){
            opt.rect.setHeight(height);
            opt.backgroundBrush.setColor(m_noFocusColor);
        }
        break;
    case MoveIn:
        if(index == m_animatedIndex){
            opt.rect.setY(height);
        }
        break;
    case Normal:
        break;
    }

    paintBackground(painter, opt, index);
    paintTitle(painter, option, index);
    paintDateTime(painter, option, index);

    painter->restore();
}

QSize NoteWidgetDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize result = QStyledItemDelegate::sizeHint(option, index);
    if(index == m_animatedIndex){
        if(m_state == MoveIn){
            result.setHeight(m_rowHeight);
        }else{
            double rate = m_timeLine->currentFrame()/(double)m_maxFrame;
            double height = m_rowHeight * rate;
            result.setHeight(height);
        }
    }else{
        result.setHeight(m_rowHeight);
    }

    return result;
}

QTimeLine::State NoteWidgetDelegate::animationState()
{
    return m_timeLine->state();
}

void NoteWidgetDelegate::paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    if((option.state & QStyle::State_Selected) == QStyle::State_Selected){
        if((option.state & QStyle::State_HasFocus)== QStyle::State_HasFocus){
            painter->fillRect(option.rect, QBrush(m_focusColor));
        }else{
            painter->fillRect(option.rect, QBrush(m_noFocusColor));
        }
    }else if((option.state & QStyle::State_MouseOver) == QStyle::State_MouseOver){
        painter->fillRect(option.rect, QBrush(m_hoverColor));
    }else if((index.row() !=  m_currentSelectedIndex.row() -1)
             && (index.row() !=  m_hoveredIndex.row() -1)){

        painter->fillRect(option.rect, QBrush(m_defaultColor));
        paintSeparator(painter, option, index);
    }
    painter->restore();
}

void NoteWidgetDelegate::paintTitle(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    QString title{index.data(NoteModel::NoteFullTitle).toString()};
    QFontMetrics fontMetrics(m_titleFont);
    title = fontMetrics.elidedText(title,Qt::ElideRight, option.rect.width() - 20);

    painter->setPen(m_titleColor);
    painter->setFont(m_titleFont);

    double rate = m_timeLine->currentFrame()/(double)m_maxFrame;
    int posX = option.rect.x() + 10;
    int posY = option.rect.y() + 15;

    if(index.row() == m_animatedIndex.row()){
        if(m_state == MoveIn){
            int posYAnim = posY + m_rowHeight * rate;
            painter->drawText(QPoint(posX, posYAnim), title);
        }else{
            int posYAnim = posY + m_rowHeight * (rate - 1);
            painter->drawText(QPoint(posX, posYAnim), title);
        }
    }else{
        painter->drawText(QPoint(posX, posY), title);
    }
    painter->restore();
}

void NoteWidgetDelegate::paintDateTime(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    QString date = parseDateTime(index.data(NoteModel::NoteLastModificationDateTime).toDateTime());
    painter->setPen(m_dateColor);
    painter->setFont(m_dateFont);

    double rate = m_timeLine->currentFrame()/(double)m_maxFrame;
    int posX = option.rect.x() + 10;
    int posY = option.rect.y() + 31;

    if(index.row() == m_animatedIndex.row()){
        if(m_state == MoveIn){
            int posYAnim = posY + m_rowHeight * rate;
            painter->drawText(QPoint(posX, posYAnim), date);
        }else{
            int posYAnim = posY + m_rowHeight * (rate - 1);
            painter->drawText(QPoint(posX, posYAnim), date);
        }
    }else{
        painter->drawText(QPoint(posX, posY), date);
    }
    painter->restore();
}

void NoteWidgetDelegate::paintSeparator(QPainter*painter, const QStyleOptionViewItem&option, const QModelIndex&index) const
{
    Q_UNUSED(index)

    painter->setPen(QPen(m_separatorColor));

    int posX1 = option.rect.x() + 10;
    int posX2 = option.rect.x() + option.rect.width()-11;
    int posY = option.rect.y() + option.rect.height()-1;


    painter->drawLine(QPoint(posX1, posY),
                      QPoint(posX2, posY));
}

QString NoteWidgetDelegate::parseDateTime(const QDateTime &dateTime) const
{
    QLocale usLocale(QLocale("en_US"));

    auto currDateTime = QDateTime::currentDateTime();

    if(dateTime.date() == currDateTime.date()){
        return usLocale.toString(dateTime.time(),"h:mm A");
    }else if(dateTime.daysTo(currDateTime) == 1){
        return "Yesterday";
    }else if(dateTime.daysTo(currDateTime) >= 2 &&
             dateTime.daysTo(currDateTime) <= 7){
        return usLocale.toString(dateTime.date(), "dddd");
    }

    return dateTime.date().toString("M/d/yy");
}

void NoteWidgetDelegate::setHoveredIndex(const QModelIndex &hoveredIndex)
{
    m_hoveredIndex = hoveredIndex;
}

void NoteWidgetDelegate::setCurrentSelectedIndex(const QModelIndex &currentSelectedIndex)
{
    m_currentSelectedIndex = currentSelectedIndex;
}

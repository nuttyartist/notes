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
      m_state(Normal)
{
    m_timeLine = new QTimeLine(300, this);
    m_timeLine->setFrameRange(0,38);
    m_timeLine->setCurveShape(QTimeLine::LinearCurve);

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

    switch ( NewState ){
    case NewNote:
        m_timeLine->stop();
        m_timeLine->setDirection(QTimeLine::Forward);
        m_timeLine->start();
        break;
    case RemoveNote:
        m_timeLine->stop();
        m_timeLine->setDirection(QTimeLine::Backward);
        m_timeLine->start();
        break;
    case Normal:
        m_timeLine->stop();
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
    QStyleOptionViewItem opt = option;
    opt.rect.setWidth(option.rect.width()-2);

    switch(m_state){
    case NewNote:
    case RemoveNote:
        if(index == m_animatedIndex){
            opt.rect.setHeight(m_timeLine->currentFrame());
            painter->fillRect(opt.rect, m_noFocusColor);
        }else{
            paintBackground(painter, opt, index);
        }
        break;
    case MovingNote:
//        opt.rect.setY(m_timeLine->currentFrame());
        paintBackground(painter, opt, index);
        break;
    case Normal:
        paintBackground(painter, opt, index);
        break;
    }

    paintTitle(painter, option, index);
    paintDateTime(painter, option, index);
}

QSize NoteWidgetDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize result = QStyledItemDelegate::sizeHint(option, index);
    if(index == m_animatedIndex){
        result.setHeight(m_timeLine->currentFrame());
    }else{
        result.setHeight(38);
    }

    return result;
}

QTimeLine::State NoteWidgetDelegate::animationState()
{
    return m_timeLine->state();
}

void NoteWidgetDelegate::paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
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
        painter->setPen(QPen(m_separatorColor));

        painter->drawLine(QPoint(option.rect.x() + 9, option.rect.y() + option.rect.height()-1),
                          QPoint(option.rect.x() + option.rect.width()-1, option.rect.y() + option.rect.height()-1));
    }
}

void NoteWidgetDelegate::paintTitle(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString title{index.data(NoteModel::NoteFullTitle).toString()};
    QFontMetrics fontMetrics(m_titleFont);
    title = fontMetrics.elidedText(title,Qt::ElideRight, option.rect.width()-20);

    painter->setPen(m_titleColor);
    painter->setFont(m_titleFont);
    painter->setOpacity(1);

    if(index == m_animatedIndex){
        painter->setOpacity(m_timeLine->currentFrame()/38.0);
        painter->drawText(QPoint(option.rect.x() + 9, option.rect.y()+15-38+m_timeLine->currentFrame()),title);
    }else{
        painter->drawText(QPoint(option.rect.x() + 9, option.rect.y()+15),title);
    }
}

void NoteWidgetDelegate::paintDateTime(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString date = parseDateTime(index.data(NoteModel::NoteDateTime).toDateTime());
    painter->setPen(m_dateColor);
    painter->setFont(m_dateFont);
    painter->setOpacity(1);


    if(index.row() == m_animatedIndex.row()){
        painter->setOpacity(m_timeLine->currentFrame()/38.0);
        painter->drawText(QPoint(option.rect.x() + 9, option.rect.y()+31-38+m_timeLine->currentFrame()), date);
    }else{
        painter->drawText(QPoint(option.rect.x() + 9, option.rect.y()+31), date);
    }

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

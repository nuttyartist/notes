#include "notewidgetdelegate.h"
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include "notemodel.h"

NoteWidgetDelegate::NoteWidgetDelegate(QWidget *parent)
    : QStyledItemDelegate(parent),
      m_titleColor(QColor(qRgb(0, 0, 0))),
      m_dateColor(QColor(qRgb(132, 132, 132))),
      m_titleFont(QFont(QStringLiteral("Liberation Sans"), 10,QFont::Bold)),
      m_dateFont(QFont(QStringLiteral("Liberation Sans"), 8, QFont::Bold))
{
}

void NoteWidgetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if((option.state & QStyle::State_Selected)== QStyle::State_Selected){
        if((option.state & QStyle::State_HasFocus)== QStyle::State_HasFocus){
            painter->fillRect(option.rect, QBrush(qRgb(254, 206, 9)));
        }else{
            painter->fillRect(option.rect, QBrush(qRgb(255, 235, 80)));
        }
    }else if((option.state & QStyle::State_MouseOver) == QStyle::State_MouseOver){
        painter->fillRect(option.rect, QBrush(qRgb(207, 207, 207)));
    }else{

        painter->fillRect(option.rect, QBrush(qRgb(255,255,255)));
        painter->setPen(QPen(qRgb(221, 221, 221)));
        painter->drawLine(QPoint(option.rect.x()+9,option.rect.y()+option.rect.height()-1),
                          QPoint(option.rect.x()+option.rect.width()-11,option.rect.y()+option.rect.height()-1));
    }

    paintTitle(painter, option, index);
    paintDateTime(painter, option, index);

}

QSize NoteWidgetDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize result = QStyledItemDelegate::sizeHint(option, index);
    result.setHeight(38);
    return result;
}

void NoteWidgetDelegate::paintTitle(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString title{index.data(NoteModel::NoteFullTitle).toString()};
    QFontMetrics fontMetrics(m_titleFont);
    title = fontMetrics.elidedText(title,Qt::ElideRight, option.rect.width()-20);

    painter->setPen(m_titleColor);
    painter->setFont(m_titleFont);
    painter->drawText(QPoint(option.rect.x() + 9, option.rect.y()+15),title);
}

void NoteWidgetDelegate::paintDateTime(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString date = parseDateTime(index.data(NoteModel::NoteDateTime).toDateTime());
    painter->setPen(m_dateColor);
    painter->setFont(m_dateFont);
    painter->drawText(QPoint(option.rect.x() + 9, option.rect.y()+31), date);

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

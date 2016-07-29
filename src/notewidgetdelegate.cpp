#include "notewidgetdelegate.h"
#include "noteview.h"
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include <QFontDatabase>
#include "notemodel.h"

NoteWidgetDelegate::NoteWidgetDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
      m_titleFont(),
      m_dateFont(),
      m_titleColor(26, 26, 26),
      m_dateColor(132, 132, 132),
      m_ActiveColor(218, 233, 239),
      m_notActiveColor(175, 212, 228),
      m_hoverColor(207, 207, 207),
      m_applicationInactiveColor(207, 207, 207),
      m_separatorColor(221, 221, 221),
      m_defaultColor(247, 247, 247),
      m_rowHeight(40),
      m_maxFrame(200),
      m_rowRightOffset(0),
      m_state(Normal),
      m_isActive(false)
{
    int id = QFontDatabase::addApplicationFont(":/fonts/roboto-hinted/Roboto-Medium.ttf");
    QString robotoFontMedium = QFontDatabase::applicationFontFamilies(id).at(0);
    m_titleFont = QFont(robotoFontMedium, 10, 60);
    m_dateFont = QFont(robotoFontMedium, 10);

    m_timeLine = new QTimeLine(300, this);
    m_timeLine->setFrameRange(0,m_maxFrame);
    m_timeLine->setUpdateInterval(10);
    m_timeLine->setCurveShape(QTimeLine::EaseInCurve);

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
    QStyleOptionViewItem opt = option;
    opt.rect.setWidth(option.rect.width() - m_rowRightOffset);

    int currentFrame = m_timeLine->currentFrame();
    double rate = (currentFrame/(m_maxFrame * 1.0));
    double height = m_rowHeight * rate;

    switch(m_state){
    case Insert:
    case Remove:
    case MoveOut:
        if(index == m_animatedIndex){
            opt.rect.setHeight(int(height));
            opt.backgroundBrush.setColor(m_notActiveColor);
        }
        break;
    case MoveIn:
        if(index == m_animatedIndex){
            opt.rect.setY(int(height));
        }
        break;
    case Normal:
        break;
    }

    paintBackground(painter, opt, index);
    paintTitle(painter, option, index);
    paintDateTime(painter, option, index);
}

QSize NoteWidgetDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize result = QStyledItemDelegate::sizeHint(option, index);
    if(index == m_animatedIndex){
        if(m_state == MoveIn){
            result.setHeight(m_rowHeight);
        }else{
            double rate = m_timeLine->currentFrame()/(m_maxFrame * 1.0);
            double height = m_rowHeight * rate;
            result.setHeight(int(height));
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
    if((option.state & QStyle::State_Selected) == QStyle::State_Selected){
        if(qApp->applicationState() == Qt::ApplicationActive){
            if(m_isActive){
                painter->fillRect(option.rect, QBrush(m_ActiveColor));
            }else{
                painter->fillRect(option.rect, QBrush(m_notActiveColor));
            }
        }else if(qApp->applicationState() == Qt::ApplicationInactive){
            painter->fillRect(option.rect, QBrush(m_applicationInactiveColor));
        }
    }else if((option.state & QStyle::State_MouseOver) == QStyle::State_MouseOver){
        painter->fillRect(option.rect, QBrush(m_hoverColor));
    }else if((index.row() !=  m_currentSelectedIndex.row() - 1)
             && (index.row() !=  m_hoveredIndex.row() - 1)){

        painter->fillRect(option.rect, QBrush(m_defaultColor));
        paintSeparator(painter, option, index);
    }
}

void NoteWidgetDelegate::paintTitle(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const int leftOffsetX = 10;
    const int topOffsetY = 4;

    QString title{index.data(NoteModel::NoteFullTitle).toString()};
    QFontMetrics fm(m_titleFont);
    QRect fmRect = fm.boundingRect(title);
    title = fm.elidedText(title,Qt::ElideRight, option.rect.width() - 2 * leftOffsetX);
    painter->setPen(m_titleColor);
    painter->setFont(m_titleFont);

    double rowRate = m_timeLine->currentFrame()/(m_maxFrame * 1.0);
    int rowPosX = option.rect.x();
    int rowPosY = option.rect.y();
    int rowWidth = option.rect.width();
    int textRectPosX = rowPosX + leftOffsetX;
    int textRectPosY = rowPosY;
    int textRectWidth = rowWidth - 2 * leftOffsetX;
    int textRectHeight = fmRect.height() + topOffsetY;

    if(index.row() == m_animatedIndex.row()){
        if(m_state == MoveIn){
            textRectPosY = rowPosY + int(m_rowHeight * rowRate);
        }else{
            double currRowHeight = m_rowHeight * rowRate;
            if(currRowHeight >= textRectHeight){
                double rateTitle = (currRowHeight - textRectHeight)/(m_rowHeight - textRectHeight);
                textRectHeight = int(textRectHeight * rateTitle);
            }else{
                textRectHeight = 0;
            }
        }
    }

    QRect rect(textRectPosX, textRectPosY, textRectWidth, textRectHeight);
    painter->drawText(rect, Qt::AlignBottom, title);
}

void NoteWidgetDelegate::paintDateTime(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString date = parseDateTime(index.data(NoteModel::NoteLastModificationDateTime).toDateTime());
    painter->setPen(m_dateColor);
    painter->setFont(m_dateFont);
    QFontMetrics fm(m_dateFont);

    const int leftOffsetX = 10;
    const int topOffsetY = 0;
    const int bottomOffset = 6;

    int rowPosX = option.rect.x();
    int rowPosY = option.rect.y();
    int rowWidth = option.rect.width();
    double rowHeightRate = m_timeLine->currentFrame()/(m_maxFrame * 1.0);
    double currRowHeight = m_rowHeight * rowHeightRate;
    int textRectWidth = rowWidth - 2 * leftOffsetX;
    int textRectHeight = fm.height() + topOffsetY;
    int textRectPosX = rowPosX + leftOffsetX;
    int textRectPosY = rowPosY + m_rowHeight - (textRectHeight + bottomOffset);

    if(index.row() == m_animatedIndex.row()){
        if(m_state == MoveIn){
            const int titleHeight = 20;
            textRectPosY = rowPosY + titleHeight + topOffsetY + int(currRowHeight);
            textRectHeight = fm.boundingRect(date).height();
        }else{
            textRectPosY = rowPosY + int(currRowHeight) - (textRectHeight + bottomOffset);

            if(currRowHeight <= (textRectHeight + bottomOffset)){
                textRectPosY = rowPosY;
                textRectHeight = int(currRowHeight) - bottomOffset;
            }
        }
    }

    QRect rect(textRectPosX, textRectPosY, textRectWidth, textRectHeight);
    painter->drawText(rect, Qt::AlignBottom, date);
}

void NoteWidgetDelegate::paintSeparator(QPainter*painter, const QStyleOptionViewItem&option, const QModelIndex&index) const
{
    Q_UNUSED(index)

    painter->setPen(QPen(m_separatorColor));
    const int leftOffsetX = 11;
    int posX1 = option.rect.x() + leftOffsetX;
    int posX2 = option.rect.x() + option.rect.width() - leftOffsetX - 1;
    int posY = option.rect.y() + option.rect.height() - 1;

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

void NoteWidgetDelegate::setActive(bool isActive)
{
    m_isActive = isActive;
}

void NoteWidgetDelegate::setRowRightOffset(int rowRightOffset)
{
    m_rowRightOffset = rowRightOffset;
}

void NoteWidgetDelegate::setHoveredIndex(const QModelIndex &hoveredIndex)
{
    m_hoveredIndex = hoveredIndex;
}

void NoteWidgetDelegate::setCurrentSelectedIndex(const QModelIndex &currentSelectedIndex)
{
    m_currentSelectedIndex = currentSelectedIndex;
}

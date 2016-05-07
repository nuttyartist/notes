#include "notewidgetdelegate.h"
#include "noteview.h"
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include "notemodel.h"

NoteWidgetDelegate::NoteWidgetDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
      m_titleFont(QFont(QStringLiteral("Liberation Sans"), 10,QFont::Bold)),
      m_dateFont(QFont(QStringLiteral("Liberation Sans"), 8)),
      m_titleColor(0, 0, 0),
      m_dateColor(132, 132, 132),
      m_ActiveColor(255, 235, 80),
      m_notActiveColor(254, 206, 9),
      m_hoverColor(207, 207, 207),
      m_applicationInactiveColor(207, 207, 207),
      m_separatorColor(221, 221, 221),
      m_defaultColor(255,255,255),
      m_rowHeight(38),
      m_maxFrame(200),
      m_rowRightOffset(0),
      m_state(Normal),
      m_isActive(false)
{
#ifdef __APPLE__
    m_titleFont.setPointSize(13);
    m_dateFont.setPointSize(10);
#elif _WIN32
    m_titleFont = QFont(QStringLiteral("Arial"), 10, QFont::Bold);
    m_dateFont = QFont(QStringLiteral("Arial"), 8);
#endif

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
    double rate = (currentFrame/(double)m_maxFrame);
    double height = m_rowHeight * rate;

    switch(m_state){
    case Insert:
    case Remove:
    case MoveOut:
        if(index == m_animatedIndex){
            opt.rect.setHeight(height);
            opt.backgroundBrush.setColor(m_notActiveColor);
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
    }else if((index.row() !=  m_currentSelectedIndex.row() -1)
             && (index.row() !=  m_hoveredIndex.row() -1)){

        painter->fillRect(option.rect, QBrush(m_defaultColor));
        paintSeparator(painter, option, index);
    }
}

void NoteWidgetDelegate::paintTitle(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString title{index.data(NoteModel::NoteFullTitle).toString()};
    QFontMetrics fm(m_titleFont);
    QRect fmRect = fm.boundingRect(title);
    title = fm.elidedText(title,Qt::ElideRight, option.rect.width() - 20);
    painter->setPen(m_titleColor);
    painter->setFont(m_titleFont);

    double rowRate = m_timeLine->currentFrame()/(double)m_maxFrame;
    int rowPosX = option.rect.x();
    int rowPosY = option.rect.y();
    int rowWidth = option.rect.width();
    int textRectPosX = rowPosX + 10;
    int textRectPosY = rowPosY;
    double textRectWidth = rowWidth - textRectPosX - 10;
    double textRectHeight = fmRect.height() + 4.0;

    auto textRect = [&](double heightRate){
        return QRectF(textRectPosX, textRectPosY, textRectWidth, textRectHeight*heightRate);
    };

    if(index.row() == m_animatedIndex.row()){
        if(m_state == MoveIn){
            int posX = option.rect.x() + 10;
            int posY = option.rect.y() + 16;
            int posYAnim = posY + m_rowHeight * rowRate;
            painter->drawText(QPoint(posX, posYAnim), title);
        }else{
            double currRowHeight = m_rowHeight * rowRate;
            if(currRowHeight >= textRectHeight){
                double rateTitle = (currRowHeight - textRectHeight)/(m_rowHeight - textRectHeight);
                painter->drawText(textRect(rateTitle), Qt::AlignBottom, title);
            }
        }
    }else{
        painter->drawText(textRect(1), Qt::AlignBottom, title);
    }
}

void NoteWidgetDelegate::paintDateTime(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString date = parseDateTime(index.data(NoteModel::NoteLastModificationDateTime).toDateTime());
    painter->setPen(m_dateColor);
    painter->setFont(m_dateFont);
    QFontMetrics fm(m_dateFont);

    int rowPosX = option.rect.x();
    int rowPosY = option.rect.y();
    int rowWidth = option.rect.width();
    double rowHeightRate = m_timeLine->currentFrame()/(double)m_maxFrame;
    double currRowHeight = m_rowHeight * rowHeightRate;
    double textRectPosX = rowPosX + 10;
    double textRectWidth = rowWidth - textRectPosX - 10;

    if(index.row() == m_animatedIndex.row()){
        if(m_state == MoveIn){
            int textPosX = rowPosX + 10;
            int textPosY = rowPosY + 32;
            int posYAnim = textPosY + currRowHeight;
            painter->drawText(QPoint(textPosX, posYAnim), date);
        }else{

            double textRectHeight = fm.height() + 2;
            double textRectPosY = rowPosY + currRowHeight - (textRectHeight + 3);

            if(currRowHeight <= (textRectHeight + 3)){
                textRectPosY = rowPosY;
                textRectHeight = currRowHeight - 3;
            }

            QRectF rect(textRectPosX, textRectPosY, textRectWidth, textRectHeight);
            painter->drawText(rect, Qt::AlignBottom, date);
        }
    }else{
        double textRectHeight = fm.height() + 2;
        double textRectPosY = rowPosY + m_rowHeight - (textRectHeight+3);
        QRectF rect(QPoint(textRectPosX, textRectPosY), QSize(textRectWidth, textRectHeight));
        painter->drawText(rect, Qt::AlignBottom, date);
    }
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

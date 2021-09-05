#include "notewidgetdelegate.h"
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include <QFontDatabase>
#include <QtMath>
#include "notemodel.h"

NoteWidgetDelegate::NoteWidgetDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
#ifdef __APPLE__
      m_displayFont(QFont(QStringLiteral("SF Pro Text")).exactMatch() ? QStringLiteral("SF Pro Text") : QStringLiteral("Roboto")),
#elif _WIN32
      m_displayFont(QFont(QStringLiteral("Segoe UI")).exactMatch() ? QStringLiteral("Segoe UI") : QStringLiteral("Roboto")),
#else
      m_displayFont(QStringLiteral("Roboto")),
#endif

#ifdef __APPLE__
      m_titleFont(m_displayFont, 13, 65),
      m_titleSelectedFont(m_displayFont, 13),
      m_dateFont(m_displayFont, 13),
#else
      m_titleFont(m_displayFont, 10, 60),
      m_titleSelectedFont(m_displayFont, 10),
      m_dateFont(m_displayFont, 10),
#endif
      m_titleColor(26, 26, 26),
      m_dateColor(132, 132, 132),
      m_ActiveColor(218, 233, 239),
      m_notActiveColor(175, 212, 228),
      m_hoverColor(207, 207, 207),
      m_applicationInactiveColor(207, 207, 207),
      m_separatorColor(221, 221, 221),
      m_defaultColor(255, 255, 255),
      m_rowHeight(42),
      m_maxFrame(200),
      m_rowRightOffset(0),
      m_state(Normal),
      m_isActive(false)
{
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
    paintLabels(painter, option, index);
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

void NoteWidgetDelegate::paintLabels(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int leftOffsetX = 10;
    const int topOffsetY = 5;   // space on top of title
    const int spaceY = 1;       // space between title and date

    QString title{index.data(NoteModel::NoteFullTitle).toString()};
    QFont titleFont = (option.state & QStyle::State_Selected) == QStyle::State_Selected ? m_titleSelectedFont : m_titleFont;
    QFontMetrics fmTitle(titleFont);
    QRect fmRectTitle = fmTitle.boundingRect(title);

    QString date = parseDateTime(index.data(NoteModel::NoteLastModificationDateTime).toDateTime());
    QFontMetrics fmDate(m_dateFont);
    QRect fmRectDate = fmDate.boundingRect(title);

    double rowPosX = option.rect.x();
    double rowPosY = option.rect.y();
    double rowWidth = option.rect.width();

    double titleRectPosX = rowPosX + leftOffsetX;
    double titleRectPosY = rowPosY;
    double titleRectWidth = rowWidth - 2.0 * leftOffsetX;
    double titleRectHeight = fmRectTitle.height() + topOffsetY;

    double dateRectPosX = rowPosX + leftOffsetX;
    double dateRectPosY = rowPosY + fmRectTitle.height() + topOffsetY;
    double dateRectWidth = rowWidth - 2.0 * leftOffsetX;
    double dateRectHeight = fmRectDate.height() + spaceY;

    double rowRate = m_timeLine->currentFrame()/(m_maxFrame * 1.0);
    double currRowHeight = m_rowHeight * rowRate;

    auto drawStr = [painter](double posX, double posY, double width, double height, QColor color, QFont font, QString str){
        QRectF rect(posX, posY, width, height);
        painter->setPen(color);
        painter->setFont(font);
        painter->drawText(rect, Qt::AlignBottom, str);
    };

    // set the bounding Rect of title and date string
    if(index.row() == m_animatedIndex.row()){
        if(m_state == MoveIn){
            titleRectHeight = topOffsetY + fmRectTitle.height() + currRowHeight;

            dateRectPosY = titleRectHeight;
            dateRectHeight = fmRectDate.height() + spaceY;

        }else{

            if((fmRectTitle.height() + topOffsetY) >= ((1.0 - rowRate) * m_rowHeight)){
                titleRectHeight = (fmRectTitle.height() + topOffsetY) - (1.0 - rowRate) * m_rowHeight;
            }else{
                titleRectHeight = 0;

                double labelsSumHeight = fmRectTitle.height() + topOffsetY + fmRectDate.height() + spaceY;
                double bottomSpace = m_rowHeight - labelsSumHeight;

                if(currRowHeight > bottomSpace){
                    dateRectHeight = currRowHeight - bottomSpace;
                }else{
                    dateRectHeight = 0;
                }
            }

            dateRectPosY = titleRectHeight + rowPosY;
        }
    }

    // draw title & date
    title = fmTitle.elidedText(title, Qt::ElideRight, int(titleRectWidth));
    drawStr(titleRectPosX, titleRectPosY, titleRectWidth, titleRectHeight, m_titleColor, titleFont, title);
    drawStr(dateRectPosX, dateRectPosY, dateRectWidth, dateRectHeight, m_dateColor, m_dateFont, date);
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

void NoteWidgetDelegate::setTheme(NoteView::Theme theme)
{
    switch(theme){
    case NoteView::Theme::Light:
    {
        m_defaultColor = QColor(255, 255, 255);
        break;
    }
    case NoteView::Theme::Dark:
    {
        m_defaultColor = QColor(16, 16, 16);
        break;
    }
    case NoteView::Theme::Sepia:
    {
        m_defaultColor = QColor(251, 240, 217);
        break;
    }
    }
}

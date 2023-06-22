#include "taglistdelegate.h"
#include "taglistmodel.h"
#include <QPainter>
#include <QPainterPath>

TagListDelegate::TagListDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
#ifdef __APPLE__
      m_displayFont(QFont(QStringLiteral("SF Pro Text")).exactMatch()
                            ? QStringLiteral("SF Pro Text")
                            : QStringLiteral("Roboto")),
#elif _WIN32
      m_displayFont(QFont(QStringLiteral("Segoe UI")).exactMatch() ? QStringLiteral("Segoe UI")
                                                                   : QStringLiteral("Roboto")),
#else
      m_displayFont(QStringLiteral("Roboto")),
#endif
#ifdef __APPLE__
      m_titleFont(m_displayFont, 13, QFont::DemiBold),
#else
      m_titleFont(m_displayFont, 10, QFont::DemiBold),
#endif
      m_titleColor(26, 26, 26),
      m_theme(Theme::Light)
{
}

void TagListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::Antialiasing);
    auto name = index.data(TagListModel::NameRole).toString();
    auto color = index.data(TagListModel::ColorRole).toString();

    auto rect = option.rect;
    rect.setHeight(20);
    QPainterPath path;
    path.addRoundedRect(rect, 10, 10);
    if (m_theme == Theme::Dark) {
        painter->fillPath(path, QColor(76, 85, 97));
    } else {
        painter->fillPath(path, QColor(218, 235, 248));
    }
    auto iconRect = QRect(rect.x() + 5, rect.y() + (rect.height() - 12) / 2, 12, 12);
    painter->setPen(QColor(color));
#ifdef __APPLE__
    int iconPointSizeOffset = 0;
#else
    int iconPointSizeOffset = -4;
#endif
    painter->setFont(QFont("Font Awesome 6 Free Solid", 12 + iconPointSizeOffset));
    painter->drawText(iconRect, u8"\uf111"); // fa-circle
    painter->setBrush(m_titleColor);
    painter->setPen(m_titleColor);

    QRect nameRect(rect);
    nameRect.setLeft(iconRect.x() + iconRect.width() + 5);
    painter->setFont(m_titleFont);
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, name);
}

QSize TagListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    QSize size;
    size.setHeight(20);
    auto name = index.data(TagListModel::NameRole).toString();
    QFontMetrics fmName(m_titleFont);
    QRect fmRectName = fmName.boundingRect(name);
    size.setWidth(5 + 12 + 5 + fmRectName.width() + 7);
    return size;
}

void TagListDelegate::setTheme(Theme::Value theme)
{
    m_theme = theme;
    switch (m_theme) {
    case Theme::Light: {
        m_titleColor = QColor(26, 26, 26);
        break;
    }
    case Theme::Dark: {
        m_titleColor = QColor(212, 212, 212);
        break;
    }
    case Theme::Sepia: {
        m_titleColor = QColor(26, 26, 26);
        break;
    }
    }
}

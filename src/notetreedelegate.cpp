#include "notetreedelegate.h"
#include "notetreemodel.h"
#include <QPainter>

NoteTreeDelegate::NoteTreeDelegate(QObject *parent):
    QStyledItemDelegate{parent},
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
    m_defaultColor(255, 255, 255)
{

}

void NoteTreeDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::Antialiasing);
    auto itemType = static_cast<NoteItem::Type>(index.data(NoteItem::Roles::ItemType).toInt());
    switch (itemType) {
    case NoteItem::Type::RootItem: {
        break;
    }
    case NoteItem::Type::AllNoteButton:
    case NoteItem::Type::TrashButton: {
        auto iconRect = QRect(option.rect.x() + 5, option.rect.y() + (option.rect.height() - 18) / 2, 14, 18);
        auto iconPath = index.data(NoteItem::Roles::Icon).toString();
        painter->drawImage(iconRect, QImage(iconPath));
        auto displayName = index.data(NoteItem::Roles::DisplayText).toString();
        QRect nameRect(option.rect);
        nameRect.setLeft(iconRect.x() + iconRect.width() + 5);
        painter->setPen(m_titleColor);
        painter->setFont(m_titleFont);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        break;
    }
    case NoteItem::Type::FolderSeparator:
    case NoteItem::Type::TagSeparator:{
        auto textRect = option.rect;
        textRect.moveLeft(textRect.x() + 5);
        auto displayName = index.data(NoteItem::Roles::DisplayText).toString();
        painter->setPen(m_separatorColor);
        painter->setFont(m_titleFont);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        break;
    }
    case NoteItem::Type::FolderItem: {
        QRect nameRect(option.rect);
        nameRect.setLeft(nameRect.x() + 10 + 5);
        auto displayName = index.data(NoteItem::Roles::DisplayText).toString();
        painter->setPen(m_titleColor);
        painter->setFont(m_titleFont);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        break;
    }
    case NoteItem::Type::NoteItem: {
        QRect nameRect(option.rect);
        nameRect.setLeft(nameRect.x() + 10 + 5);
        auto displayName = index.data(NoteItem::Roles::DisplayText).toString();
        painter->setPen(m_titleColor);
        painter->setFont(m_titleFont);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        break;
    }
    case NoteItem::Type::TagItem: {
        auto iconRect = QRect(option.rect.x() + 10, option.rect.y() + (option.rect.height() - 14) / 2, 14, 14);
        auto tagColor = index.data(NoteItem::Roles::TagColor).toString();
        painter->setBrush(QColor(tagColor));
        painter->setPen(QColor(tagColor));
        painter->drawEllipse(iconRect);
        painter->setBrush(Qt::black);
        painter->setPen(Qt::black);
        QRect nameRect(option.rect);
        nameRect.setLeft(iconRect.x() + iconRect.width() + 5);
        auto displayName = index.data(NoteItem::Roles::DisplayText).toString();
        painter->setPen(m_titleColor);
        painter->setFont(m_titleFont);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        break;
    }
    }
}

QSize NoteTreeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize result = QStyledItemDelegate::sizeHint(option, index);
    auto itemType = static_cast<NoteItem::Type>(index.data(NoteItem::Roles::ItemType).toInt());
    if (itemType == NoteItem::Type::FolderSeparator || itemType == NoteItem::Type::TagSeparator) {
        result.setHeight(25);
    } else {
        result.setHeight(30);
    }
    return result;
}

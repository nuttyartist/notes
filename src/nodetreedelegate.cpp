#include "nodetreedelegate.h"
#include "nodetreemodel.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include "pushbuttontype.h"
#include "foldertreedelegateeditor.h"

NodeTreeDelegate::NodeTreeDelegate(QTreeView *view, QObject *parent):
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
    m_titleSelectedColor(255, 255, 255),
    m_dateColor(132, 132, 132),
    m_ActiveColor(68, 138, 201),
    m_notActiveColor(175, 212, 228),
    m_hoverColor(207, 207, 207),
    m_applicationInactiveColor(207, 207, 207),
    m_separatorColor(221, 221, 221),
    m_defaultColor(255, 255, 255),
    m_separatorTextColor(143, 143, 143),
    m_view(view)
{

}

void NodeTreeDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::Antialiasing);
    auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());

    painter->fillRect(option.rect, Qt::white);
    switch (itemType) {
    case NodeItem::Type::RootItem: {
        break;
    }
    case NodeItem::Type::AllNoteButton:
    case NodeItem::Type::TrashButton: {
        paintBackgroundSelectable(painter, option);
        auto iconRect = QRect(option.rect.x() + 5, option.rect.y() + (option.rect.height() - 20) / 2, 18, 20);
        auto iconPath = index.data(NodeItem::Roles::Icon).toString();
        painter->drawImage(iconRect, QImage(iconPath));
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString();
        QRect nameRect(option.rect);
        nameRect.setLeft(iconRect.x() + iconRect.width() + 5);
        if((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
            painter->setPen(m_titleSelectedColor);
        } else {
            painter->setPen(m_titleColor);
        }
        painter->setFont(m_titleFont);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        break;
    }
    case NodeItem::Type::FolderSeparator:
    case NodeItem::Type::TagSeparator:{
        auto textRect = option.rect;
        textRect.moveLeft(textRect.x() + 5);
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString();
        painter->setPen(m_separatorColor);
        painter->setFont(m_titleFont);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        break;
    }
    case NodeItem::Type::FolderItem: {
        paintBackgroundSelectable(painter, option);
        auto iconRect = QRect(option.rect.x() + 5, option.rect.y() + (option.rect.height() - 20) / 2, 20, 20);
        QString iconPath;
        if((option.state & QStyle::State_Open) == QStyle::State_Open) {
            iconPath = ":/images/tree-node-expanded.png";
        } else {
            iconPath = ":/images/tree-node-normal.png";
        }
        painter->drawImage(iconRect, QImage(iconPath));
        QRect nameRect(option.rect);
//        nameRect.setLeft(nameRect.x() + 10 + 5);
        nameRect.setLeft(iconRect.x() + iconRect.width() + 5);
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString();
        if((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
            painter->setPen(m_titleSelectedColor);
        } else {
            painter->setPen(m_titleColor);
        }
        painter->setFont(m_titleFont);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        break;
    }
    case NodeItem::Type::NoteItem: {
        paintBackgroundSelectable(painter, option);
        QRect nameRect(option.rect);
        nameRect.setLeft(nameRect.x() + 10 + 5);
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString();
        if((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
            painter->setPen(m_titleSelectedColor);
        } else {
            painter->setPen(m_titleColor);
        }
        painter->setFont(m_titleFont);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        break;
    }
    case NodeItem::Type::TagItem: {
        paintBackgroundSelectable(painter, option);
        auto iconRect = QRect(option.rect.x() + 10, option.rect.y() + (option.rect.height() - 14) / 2, 14, 14);
        auto tagColor = index.data(NodeItem::Roles::TagColor).toString();
        painter->setBrush(QColor(tagColor));
        painter->setPen(QColor(tagColor));
        painter->drawEllipse(iconRect);
        painter->setBrush(Qt::black);
        painter->setPen(Qt::black);
        QRect nameRect(option.rect);
        nameRect.setLeft(iconRect.x() + iconRect.width() + 5);
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString();
        if((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
            painter->setPen(m_titleSelectedColor);
        } else {
            painter->setPen(m_titleColor);
        }
        painter->setFont(m_titleFont);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        break;
    }
    }
}

void NodeTreeDelegate::paintBackgroundSelectable(QPainter *painter, const QStyleOptionViewItem &option) const
{
    if((option.state & QStyle::State_Selected) == QStyle::State_Selected){
        painter->fillRect(option.rect, QBrush(m_ActiveColor));
    } else if((option.state & QStyle::State_MouseOver) == QStyle::State_MouseOver){
        painter->fillRect(option.rect, QBrush(m_hoverColor));
    }
}

QSize NodeTreeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize result = QStyledItemDelegate::sizeHint(option, index);
    auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
    if (itemType == NodeItem::Type::FolderSeparator || itemType == NodeItem::Type::TagSeparator) {
        result.setHeight(25);
    } else {
        result.setHeight(30);
    }
    return result;
}

QWidget *NodeTreeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
    if (itemType == NodeItem::Type::FolderSeparator || itemType == NodeItem::Type::TagSeparator) {
        auto widget = new QWidget(parent);
        widget->setContentsMargins(0, 0, 0, 0);
        auto layout = new QHBoxLayout(widget);
        layout->setContentsMargins(5, 0, 0, 0);
        widget->setLayout(layout);
        auto label = new QLabel(widget);
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString();
        label->setStyleSheet(QStringLiteral("QLabel{color: rgb(%1, %2, %3);}")
                             .arg(QString::number(m_separatorTextColor.red()),
                                  QString::number(m_separatorTextColor.green()),
                                  QString::number(m_separatorTextColor.blue())));
        label->setFont(m_titleFont);
        label->setText(displayName);
        layout->addWidget(label);
        auto addButton = new PushButtonType(parent);
        addButton->setMaximumSize({33, 25});
        addButton->setMinimumSize({33, 25});
        addButton->setCursor(QCursor(Qt::PointingHandCursor));
        addButton->setFocusPolicy(Qt::TabFocus);
        addButton->setNormalIcon(QIcon(QString::fromUtf8(":/images/newNote_Regular.png")));
        addButton->setHoveredIcon(QIcon(QString::fromUtf8(":/images/newNote_Hovered.png")));
        addButton->setPressedIcon(QIcon(QString::fromUtf8(":/images/newNote_Pressed.png")));
        addButton->setIconSize(QSize(16, 16));
        addButton->setStyleSheet(QStringLiteral(R"(QPushButton { )"
                                                R"(    border: none; )"
                                                R"(    padding: 0px; )"
                                                R"(})"));
        if (itemType == NodeItem::Type::FolderSeparator) {
            connect(addButton, &QPushButton::clicked,
                    this, &NodeTreeDelegate::addFolderRequested);
        } else {
            connect(addButton, &QPushButton::clicked,
                    this, &NodeTreeDelegate::addTagRequested);
        }
        layout->addWidget(addButton, 1, Qt::AlignRight);
        return widget;
    } else if (itemType == NodeItem::Type::FolderItem) {
        auto widget = new FolderTreeDelegateEditor(m_view, option, index, parent);
        return widget;
    } else if (itemType == NodeItem::Type::NoteItem) {
        return nullptr;
    }
    return nullptr;
}

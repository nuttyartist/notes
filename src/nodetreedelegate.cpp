#include "nodetreedelegate.h"
#include "nodetreemodel.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include "nodetreeview.h"
#include "pushbuttontype.h"
#include "foldertreedelegateeditor.h"
#include "tagtreedelegateeditor.h"
#include "trashbuttondelegateeditor.h"
#include "defaultnotefolderdelegateeditor.h"
#include "allnotebuttontreedelegateeditor.h"
#include <QFontMetrics>

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
    m_defaultColor(247, 247, 247),
    m_separatorTextColor(143, 143, 143),
    m_currentBackgroundColor(255, 255, 255),
    m_view(view),
    m_theme(Theme::Light)
{

}

void NodeTreeDelegate::setTheme(Theme theme)
{
    m_theme = theme;
    switch (theme) {
    case Theme::Light:
    {
        m_titleColor = QColor(26, 26, 26);
        m_dateColor = QColor(26, 26, 26);
        m_defaultColor = QColor(247, 247, 247);
        //        m_ActiveColor = QColor(218, 233, 239);
        m_notActiveColor = QColor(175, 212, 228);
        m_hoverColor = QColor(207, 207, 207);
        m_currentBackgroundColor = QColor(247, 247, 247);
        break;
    }
    case Theme::Dark:
    {
        m_titleColor = QColor(204, 204, 204);
        m_dateColor = QColor(204, 204, 204);
        m_defaultColor = QColor(26, 26, 26);
        //        m_ActiveColor = QColor(0, 59, 148);
        m_notActiveColor = QColor(0, 59, 148);
        m_hoverColor = QColor(15, 45, 90);
        m_currentBackgroundColor = QColor(26, 26, 26);
        break;
    }
    case Theme::Sepia:
    {
        m_titleColor = QColor(26, 26, 26);
        m_dateColor = QColor(26, 26, 26);
        m_defaultColor = QColor(251, 240, 217);
        //        m_ActiveColor = QColor(218, 233, 239);
        m_notActiveColor = QColor(175, 212, 228);
        m_hoverColor = QColor(207, 207, 207);
        m_currentBackgroundColor = QColor(251, 240, 217);
        break;
    }
    }
}

void NodeTreeDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::Antialiasing);
    auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());

    painter->fillRect(option.rect, m_currentBackgroundColor);
    switch (itemType) {
    case NodeItem::Type::RootItem: {
        break;
    }
    case NodeItem::Type::AllNoteButton:
    case NodeItem::Type::TrashButton: {
        paintBackgroundSelectable(painter, option, index);
        auto iconRect = QRect(option.rect.x() + 5, option.rect.y() + (option.rect.height() - 20) / 2, 18, 20);
        if (m_theme == Theme::Dark) {
            if (itemType == NodeItem::Type::AllNoteButton) {
                painter->drawImage(iconRect, QImage(":/images/all-notes-icon-dark.png"));
            } else if (itemType == NodeItem::Type::TrashButton) {
                painter->drawImage(iconRect, QImage(":/images/trash-icon-dark.png"));
            }
        } else {
            auto iconPath = index.data(NodeItem::Roles::Icon).toString();
            painter->drawImage(iconRect, QImage(iconPath));
        }
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString();
        QRect nameRect(option.rect);
        nameRect.setLeft(iconRect.x() + iconRect.width() + 5);
        nameRect.setWidth(nameRect.width() - 5 - 40);
        if((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
            painter->setPen(m_titleSelectedColor);
        } else {
            painter->setPen(m_titleColor);
        }
        painter->setFont(m_titleFont);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        auto childCountRect = option.rect;
        childCountRect.setLeft(nameRect.right() + 5);
        childCountRect.setWidth(childCountRect.width() - 5);
        auto childCount = index.data(NodeItem::Roles::ChildCount).toInt();
        if((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
            painter->setPen(m_titleSelectedColor);
        } else {
            painter->setPen(m_titleColor);
        }
        painter->setFont(m_titleFont);
        painter->drawText(childCountRect, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(childCount));
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
        paintBackgroundSelectable(painter, option, index);
        auto iconRect = QRect(option.rect.x() + 5, option.rect.y() + (option.rect.height() - 20) / 2, 20, 20);
        QString iconPath;
        if (m_theme == Theme::Dark) {
            if((option.state & QStyle::State_Open) == QStyle::State_Open) {
                iconPath = ":/images/tree-node-expanded-dark.png";
            } else {
                iconPath = ":/images/tree-node-normal-dark.png";
            }
        } else {
            if((option.state & QStyle::State_Open) == QStyle::State_Open) {
                iconPath = ":/images/tree-node-expanded.png";
            } else {
                iconPath = ":/images/tree-node-normal.png";
            }
        }
        if (index.data(NodeItem::Roles::IsExpandable).toBool()) {
            painter->drawImage(iconRect, QImage(iconPath));
        }
        QRect nameRect(option.rect);
        nameRect.setLeft(iconRect.x() + iconRect.width() + 5);
        nameRect.setWidth(nameRect.width() - 5 - 40);
        QFontMetrics fm(m_titleFont);
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString();
        displayName = fm.elidedText(displayName, Qt::ElideRight, nameRect.width());
        if((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
            painter->setPen(m_titleSelectedColor);
        } else {
            painter->setPen(m_titleColor);
        }
        painter->setFont(m_titleFont);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        auto childCountRect = option.rect;
        childCountRect.setLeft(nameRect.right() + 5);
        childCountRect.setWidth(childCountRect.width() - 5);
        auto childCount = index.data(NodeItem::Roles::ChildCount).toInt();
        if((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
            painter->setPen(m_titleSelectedColor);
        } else {
            painter->setPen(m_titleColor);
        }
        painter->setFont(m_titleFont);
        painter->drawText(childCountRect, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(childCount));
        break;
    }
    case NodeItem::Type::NoteItem: {
        paintBackgroundSelectable(painter, option, index);
        QRect nameRect(option.rect);
        nameRect.setLeft(nameRect.x() + 10 + 5);
        nameRect.setWidth(nameRect.width() - 5);
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString();
        QFontMetrics fm(m_titleFont);
        displayName = fm.elidedText(displayName, Qt::ElideRight, nameRect.width());
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
        paintBackgroundSelectable(painter, option, index);
        auto iconRect = QRect(option.rect.x() + 10, option.rect.y() + (option.rect.height() - 14) / 2, 14, 14);
        auto tagColor = index.data(NodeItem::Roles::TagColor).toString();
        painter->setBrush(QColor(tagColor));
        painter->setPen(QColor(tagColor));
        painter->drawEllipse(iconRect);
        painter->setBrush(Qt::black);
        painter->setPen(Qt::black);
        QRect nameRect(option.rect);
        nameRect.setLeft(iconRect.x() + iconRect.width() + 5);
        nameRect.setWidth(nameRect.width() - 5 - 40);
        auto displayName = index.data(NodeItem::Roles::DisplayText).toString();
        QFontMetrics fm(m_titleFont);
        displayName = fm.elidedText(displayName, Qt::ElideRight, nameRect.width());
        if((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
            painter->setPen(m_titleSelectedColor);
        } else {
            painter->setPen(m_titleColor);
        }
        painter->setFont(m_titleFont);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
        auto childCountRect = option.rect;
        childCountRect.setLeft(nameRect.right() + 5);
        childCountRect.setWidth(childCountRect.width() - 5);
        auto childCount = index.data(NodeItem::Roles::ChildCount).toInt();
        if((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
            painter->setPen(m_titleSelectedColor);
        } else {
            painter->setPen(m_titleColor);
        }
        painter->setFont(m_titleFont);
        painter->drawText(childCountRect, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(childCount));
        break;
    }
    }
}

void NodeTreeDelegate::paintBackgroundSelectable(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    if((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
        painter->fillRect(option.rect, QBrush(m_ActiveColor));
    } else if((option.state & QStyle::State_MouseOver) == QStyle::State_MouseOver) {
        auto treeView = dynamic_cast<NodeTreeView*>(m_view);
        auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
        if (itemType == NodeItem::Type::TrashButton) {
            return;
        }
        if (!treeView->isDragging()) {
            painter->fillRect(option.rect, QBrush(m_hoverColor));
        }
    }
}

QSize NodeTreeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize result = QStyledItemDelegate::sizeHint(option, index);
    auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
    if (itemType == NodeItem::Type::FolderSeparator) {
        result.setHeight(NoteTreeConstant::folderLabelHeight);
    } else if (itemType == NodeItem::Type::TagSeparator) {
        result.setHeight(NoteTreeConstant::tagLabelHeight);
    } else if (itemType == NodeItem::Type::FolderItem ||
               itemType == NodeItem::Type::TrashButton ||
               itemType == NodeItem::Type::AllNoteButton) {
        result.setHeight(NoteTreeConstant::folderItemHeight);
    } else if (itemType == NodeItem::Type::TagItem) {
        result.setHeight(NoteTreeConstant::tagItemHeight);
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
        addButton->setMaximumSize({38, 25});
        addButton->setMinimumSize({38, 25});
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
        auto id = index.data(NodeItem::Roles::NodeId).toInt();
        if (id == SpecialNodeID::DefaultNotesFolder) {
            auto widget = new DefaultNoteFolderDelegateEditor(m_view, option, index, parent);
            return widget;
        } else {
            auto widget = new FolderTreeDelegateEditor(m_view, option, index, parent);
            return widget;
        }
    } else if (itemType == NodeItem::Type::TagItem) {
        auto widget = new TagTreeDelegateEditor(m_view, option, index, parent);
        return widget;
    } else if (itemType == NodeItem::Type::TrashButton) {
        auto widget = new TrashButtonDelegateEditor(m_view, option, index, parent);
        return widget;
    } else if (itemType == NodeItem::Type::AllNoteButton) {
        auto widget = new AllNoteButtonTreeDelegateEditor(m_view, option, index, parent);
        return widget;
    }
    return nullptr;
}

void NodeTreeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    auto itemType = static_cast<NodeItem::Type>(index.data(NodeItem::Roles::ItemType).toInt());
    if (itemType == NodeItem::Type::TrashButton || itemType == NodeItem::Type::AllNoteButton) {
        editor->setGeometry(0, editor->y(), option.rect.width(), option.rect.height());
    }
}

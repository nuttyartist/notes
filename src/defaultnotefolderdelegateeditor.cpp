#include "defaultnotefolderdelegateeditor.h"
#include <QPainter>
#include <QDebug>
#include <QTreeView>
#include "nodetreemodel.h"
#include "notelistview.h"
#include "editorsettingsoptions.h"

DefaultNoteFolderDelegateEditor::DefaultNoteFolderDelegateEditor(QTreeView *view,
                                                                 const QStyleOptionViewItem &option,
                                                                 const QModelIndex &index,
                                                                 QListView *listView,
                                                                 QWidget *parent)
    : QWidget(parent),
      m_option(option),
      m_index(index),
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
      m_numberOfNotesFont(m_displayFont, 12, QFont::DemiBold),
#else
      m_titleFont(m_displayFont, 10, QFont::DemiBold),
      m_numberOfNotesFont(m_displayFont, 9, QFont::DemiBold),
#endif
      m_titleColor(26, 26, 26),
      m_titleSelectedColor(255, 255, 255),
      m_activeColor(68, 138, 201),
      m_hoverColor(207, 207, 207),
      m_folderIconColor(68, 138, 201),
      m_numberOfNotesColor(26, 26, 26, 127),
      m_numberOfNotesSelectedColor(255, 255, 255),
      m_view(view),
      m_listView(listView)
{
    setContentsMargins(0, 0, 0, 0);
}

void DefaultNoteFolderDelegateEditor::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    auto iconRect = QRect(rect().x() + 4, rect().y() + (rect().height() - 20) / 2, 20, 20);

    QRect folderIconRect(rect());
    folderIconRect.setLeft(iconRect.x() + iconRect.width());
    folderIconRect.setTop(rect().y() + 5);
    folderIconRect.setWidth(18);
    if (m_view->selectionModel()->isSelected(m_index)) {
        painter.fillRect(rect(), QBrush(m_activeColor));
        painter.setPen(m_titleSelectedColor);
    } else {
        auto listView = dynamic_cast<NoteListView *>(m_listView);
        if (listView->isDragging()) {
            if (m_theme == Theme::Dark) {
                painter.fillRect(rect(), QBrush(QColor(35, 52, 69)));
            } else {
                painter.fillRect(rect(), QBrush(QColor(180, 208, 233)));
            }
        } else {
            painter.fillRect(rect(), QBrush(m_hoverColor));
        }
        painter.setPen(m_folderIconColor);
    }
#ifdef __APPLE__
    int iconPointSizeOffset = 0;
#else
    int iconPointSizeOffset = -4;
#endif
    painter.setFont(QFont("Material Symbols Outlined", 16 + iconPointSizeOffset));
    painter.drawText(folderIconRect, u8"\ue2c7"); // folder

    QRect nameRect(rect());
    nameRect.setLeft(folderIconRect.x() + folderIconRect.width() + 5);
    nameRect.setWidth(nameRect.width() - 5 - 40);

    auto displayName = m_index.data(NodeItem::Roles::DisplayText).toString();
    QFontMetrics fm(m_titleFont);
    displayName = fm.elidedText(displayName, Qt::ElideRight, nameRect.width());

    if (m_view->selectionModel()->isSelected(m_index)) {
        painter.setPen(m_titleSelectedColor);
    } else {
        painter.setPen(m_titleColor);
    }
    painter.setFont(m_titleFont);
    painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
    auto childCountRect = rect();
    childCountRect.setLeft(nameRect.right() + 5);
    childCountRect.setWidth(childCountRect.width() - 5);
    auto childCount = m_index.data(NodeItem::Roles::ChildCount).toInt();
    if (m_view->selectionModel()->isSelected(m_index)) {
        painter.setPen(m_numberOfNotesSelectedColor);
    } else {
        painter.setPen(m_numberOfNotesColor);
    }
    painter.setFont(m_numberOfNotesFont);
    painter.drawText(childCountRect, Qt::AlignHCenter | Qt::AlignVCenter,
                     QString::number(childCount));
    QWidget::paintEvent(event);
}

void DefaultNoteFolderDelegateEditor::setTheme(Theme::Value theme)
{
    m_theme = theme;
    switch (theme) {
    case Theme::Light: {
        m_hoverColor = QColor(247, 247, 247);
        m_titleColor = QColor(26, 26, 26);
        m_numberOfNotesColor = QColor(26, 26, 26, 127);
        break;
    }
    case Theme::Dark: {
        m_hoverColor = QColor(25, 25, 25);
        m_titleColor = QColor(212, 212, 212);
        m_numberOfNotesColor = QColor(212, 212, 212, 127);
        break;
    }
    case Theme::Sepia: {
        m_hoverColor = QColor(251, 240, 217);
        m_titleColor = QColor(26, 26, 26);
        m_numberOfNotesColor = QColor(26, 26, 26, 127);
        break;
    }
    }
}

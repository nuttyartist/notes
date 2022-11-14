#include "defaultnotefolderdelegateeditor.h"
#include <QPainter>
#include <QDebug>
#include <QTreeView>
#include "nodetreemodel.h"

DefaultNoteFolderDelegateEditor::DefaultNoteFolderDelegateEditor(QTreeView *view,
                                                                 const QStyleOptionViewItem &option,
                                                                 const QModelIndex &m_index,
                                                                 QWidget *parent)
    : QWidget(parent),
      m_option(option),
      m_index(m_index),
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
      m_titleSelectedColor(255, 255, 255),
      m_activeColor(68, 138, 201),
      m_hoverColor(207, 207, 207),
      m_view(view)
{
    setContentsMargins(0, 0, 0, 0);
}

void DefaultNoteFolderDelegateEditor::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    auto iconRect = QRect(rect().x() + 5, rect().y() + (rect().height() - 20) / 2, 20, 20);
    QRect nameRect(rect());
    nameRect.setLeft(iconRect.x() + iconRect.width());
    nameRect.setWidth(nameRect.width() - 5 - 40);

    auto displayName = m_index.data(NodeItem::Roles::DisplayText).toString();
    QFontMetrics fm(m_titleFont);
    displayName = fm.elidedText(displayName, Qt::ElideRight, nameRect.width());

    if (m_view->selectionModel()->isSelected(m_index)) {
        painter.fillRect(rect(), QBrush(m_activeColor));
        painter.setPen(m_titleSelectedColor);
    } else {
        painter.fillRect(rect(), QBrush(m_hoverColor));
        painter.setPen(m_titleColor);
    }
    painter.setFont(m_titleFont);
    painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
    auto childCountRect = rect();
    childCountRect.setLeft(nameRect.right() + 5);
    childCountRect.setWidth(childCountRect.width() - 5);
    auto childCount = m_index.data(NodeItem::Roles::ChildCount).toInt();
    if (m_view->selectionModel()->isSelected(m_index)) {
        painter.setPen(m_titleSelectedColor);
    } else {
        painter.setPen(m_titleColor);
    }
    painter.setFont(m_titleFont);
    painter.drawText(childCountRect, Qt::AlignHCenter | Qt::AlignVCenter,
                     QString::number(childCount));
    QWidget::paintEvent(event);
}

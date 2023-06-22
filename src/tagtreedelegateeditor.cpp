#include "tagtreedelegateeditor.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QDebug>
#include <QTreeView>
#include <QMouseEvent>
#include "pushbuttontype.h"
#include "nodetreemodel.h"
#include "nodetreeview.h"
#include "labeledittype.h"
#include "notelistview.h"

TagTreeDelegateEditor::TagTreeDelegateEditor(QTreeView *view, const QStyleOptionViewItem &option,
                                             const QModelIndex &index, QListView *listView,
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
#else
      m_titleFont(m_displayFont, 10, QFont::DemiBold),
#endif
      m_titleColor(26, 26, 26),
      m_titleSelectedColor(255, 255, 255),
      m_activeColor(68, 138, 201),
      m_hoverColor(207, 207, 207),
      m_view(view),
      m_listView(listView)
{
    setContentsMargins(0, 0, 0, 0);
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(22, 0, 0, 0);
    layout->setSpacing(5);
    setLayout(layout);
    layout->addSpacing(27);

    m_label = new LabelEditType(this);
    m_label->setFont(m_titleFont);
    QSizePolicy labelPolicy;
    labelPolicy.setVerticalPolicy(QSizePolicy::Expanding);
    labelPolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    m_label->setSizePolicy(labelPolicy);
    m_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    connect(m_label, &LabelEditType::editingStarted, this, [this] {
        auto tree_view = dynamic_cast<NodeTreeView *>(m_view);
        tree_view->setIsEditing(true);
    });
    connect(m_label, &LabelEditType::editingFinished, this, [this](const QString &label) {
        auto tree_view = dynamic_cast<NodeTreeView *>(m_view);
        tree_view->onRenameTagFinished(label);
        tree_view->setIsEditing(false);
    });
    connect(dynamic_cast<NodeTreeView *>(m_view), &NodeTreeView::renameTagRequested, m_label,
            &LabelEditType::openEditor);
    layout->addWidget(m_label);
    m_contextButton = new PushButtonType(parent);
    m_contextButton->setMaximumSize({ 33, 25 });
    m_contextButton->setMinimumSize({ 33, 25 });
    m_contextButton->setCursor(QCursor(Qt::PointingHandCursor));
    m_contextButton->setFocusPolicy(Qt::TabFocus);
    m_contextButton->setIconSize(QSize(16, 16));
    if (m_view->selectionModel()->isSelected(m_index)) {
        m_contextButton->setStyleSheet(QStringLiteral(R"(QPushButton { )"
                                                      R"(    border: none; )"
                                                      R"(    padding: 0px; )"
                                                      R"(    color: white; )"
                                                      R"(})"
                                                      R"(QPushButton:pressed { )"
                                                      R"(    border: none; )"
                                                      R"(    padding: 0px; )"
                                                      R"(    color: rgb(210, 210, 210); )"
                                                      R"(})"));
    } else {
        m_contextButton->setStyleSheet(QStringLiteral(R"(QPushButton { )"
                                                      R"(    border: none; )"
                                                      R"(    padding: 0px; )"
                                                      R"(    color: rgb(68, 138, 201); )"
                                                      R"(})"
                                                      R"(QPushButton:pressed { )"
                                                      R"(    border: none; )"
                                                      R"(    padding: 0px; )"
                                                      R"(    color: rgb(39, 85, 125); )"
                                                      R"(})"));
    }

#ifdef __APPLE__
    int pointSizeOffset = 0;
#else
    int pointSizeOffset = -4;
#endif
    m_contextButton->setFont(QFont("Font Awesome 6 Free Solid", 14 + pointSizeOffset));
    m_contextButton->setText(u8"\uf141"); // fa-ellipsis-h

    connect(m_contextButton, &QPushButton::clicked, m_view, [this](bool) {
        auto tree_view = dynamic_cast<NodeTreeView *>(m_view);
        if (!m_view->selectionModel()->selectedIndexes().contains(m_index)) {
            tree_view->setCurrentIndexC(m_index);
        }
        tree_view->onCustomContextMenu(tree_view->visualRect(m_index).topLeft()
                                       + m_contextButton->geometry().bottomLeft());
    });
    layout->addWidget(m_contextButton, 0, Qt::AlignRight);
    layout->addSpacing(5);
    connect(m_view, &QTreeView::expanded, this, [this](const QModelIndex &) { update(); });
}

void TagTreeDelegateEditor::updateDelegate()
{
    auto displayName = m_index.data(NodeItem::Roles::DisplayText).toString();
    QFontMetrics fm(m_titleFont);
    displayName = fm.elidedText(displayName, Qt::ElideRight, m_label->contentsRect().width());

    if (m_view->selectionModel()->selectedIndexes().contains(m_index)) {
        m_label->setStyleSheet(QStringLiteral("QLabel{color: rgb(%1, %2, %3);}")
                                       .arg(QString::number(m_titleSelectedColor.red()),
                                            QString::number(m_titleSelectedColor.green()),
                                            QString::number(m_titleSelectedColor.blue())));
    } else {
        m_label->setStyleSheet(QStringLiteral("QLabel{color: rgb(%1, %2, %3);}")
                                       .arg(QString::number(m_titleColor.red()),
                                            QString::number(m_titleColor.green()),
                                            QString::number(m_titleColor.blue())));
    }
    m_label->setText(displayName);
}

void TagTreeDelegateEditor::paintEvent(QPaintEvent *event)
{
    updateDelegate();
    QPainter painter(this);
    if (m_view->selectionModel()->selectedIndexes().contains(m_index)) {
        painter.fillRect(rect(), QBrush(m_activeColor));
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
    }

    auto iconRect = QRect(rect().x() + 22, rect().y() + (rect().height() - 14) / 2, 16, 16);
    auto tagColor = m_index.data(NodeItem::Roles::TagColor).toString();
    painter.setPen(QColor(tagColor));
#ifdef __APPLE__
    int iconPointSizeOffset = 0;
#else
    int iconPointSizeOffset = -4;
#endif
    painter.setFont(QFont("Font Awesome 6 Free Solid", 16 + iconPointSizeOffset));
    painter.drawText(iconRect, u8"\uf111"); // fa-circle
    QWidget::paintEvent(event);
}

void TagTreeDelegateEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
    auto iconRect = QRect(rect().x() + 10, rect().y() + (rect().height() - 14) / 2, 14, 14);
    if (iconRect.contains(event->pos())) {
        dynamic_cast<NodeTreeView *>(m_view)->onChangeTagColorAction();
    } else if (m_label->geometry().contains(event->pos())) {
        m_label->openEditor();
    } else {
        QWidget::mouseDoubleClickEvent(event);
    }
}

void TagTreeDelegateEditor::setTheme(Theme::Value theme)
{
    m_theme = theme;
    switch (theme) {
    case Theme::Light: {
        m_hoverColor = QColor(247, 247, 247);
        m_titleColor = QColor(26, 26, 26);
        break;
    }
    case Theme::Dark: {
        m_hoverColor = QColor(25, 25, 25);
        m_titleColor = QColor(212, 212, 212);
        break;
    }
    case Theme::Sepia: {
        m_hoverColor = QColor(251, 240, 217);
        m_titleColor = QColor(26, 26, 26);
        break;
    }
    }
}

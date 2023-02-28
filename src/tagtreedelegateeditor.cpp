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

TagTreeDelegateEditor::TagTreeDelegateEditor(QTreeView *view, const QStyleOptionViewItem &option,
                                             const QModelIndex &index, QWidget *parent)
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
      m_view(view)
{
    setContentsMargins(0, 0, 0, 0);
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 0, 0, 0);
    layout->setSpacing(5);
    setLayout(layout);
    layout->addSpacing(24);

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
    m_contextButton->setStyleSheet(QStringLiteral(R"(QPushButton { )"
                                                  R"(    border: none; )"
                                                  R"(    padding: 0px; )"
                                                  R"(})"));
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
        m_contextButton->setNormalIcon(QIcon(QString::fromUtf8(":/images/3dots_Highlighted.png")));
        m_contextButton->setHoveredIcon(QIcon(QString::fromUtf8(":/images/3dots_Highlighted.png")));
        m_contextButton->setPressedIcon(QIcon(QString::fromUtf8(":/images/3dots_Highlighted.png")));
    } else {
        m_label->setStyleSheet(QStringLiteral("QLabel{color: rgb(%1, %2, %3);}")
                                       .arg(QString::number(m_titleColor.red()),
                                            QString::number(m_titleColor.green()),
                                            QString::number(m_titleColor.blue())));
        m_contextButton->setNormalIcon(QIcon(QString::fromUtf8(":/images/3dots_Regular.png")));
        m_contextButton->setHoveredIcon(QIcon(QString::fromUtf8(":/images/3dots_Hovered.png")));
        m_contextButton->setPressedIcon(QIcon(QString::fromUtf8(":/images/3dots_Pressed.png")));
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
        painter.fillRect(rect(), QBrush(m_hoverColor));
    }

    auto iconRect = QRect(rect().x() + 10, rect().y() + (rect().height() - 14) / 2, 14, 14);
    auto tagColor = m_index.data(NodeItem::Roles::TagColor).toString();
    painter.setBrush(QColor(tagColor));
    painter.setPen(QColor(tagColor));
    painter.drawEllipse(iconRect);
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

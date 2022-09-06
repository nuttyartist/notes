#include "labeledittype.h"
#include <QLineEdit>
#include <QBoxLayout>
#include <QDebug>

LabelEditType::LabelEditType(QWidget *parent) : QLabel(parent)
{
    setContentsMargins(0, 0, 0, 0);
    m_editor = new QLineEdit(this);
    auto layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->addWidget(m_editor);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    m_editor->hide();
    connect(m_editor, &QLineEdit::editingFinished, this, &LabelEditType::onFinishedEdit);
}

void LabelEditType::openEditor()
{
    m_editor->setText(text());
    m_editor->setSelection(0, text().length());
    m_editor->setFont(font());
    m_editor->setFocus(Qt::MouseFocusReason);
    m_editor->show();
    emit editingStarted();
}

void LabelEditType::onFinishedEdit()
{
    m_editor->hide();
    if (!m_editor->text().isEmpty()) {
        setText(m_editor->text());
    }
    emit editingFinished(text());
}

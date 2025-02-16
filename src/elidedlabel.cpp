#include "elidedlabel.h"

#include <QResizeEvent>
#include <QTimer>

ElidedLabel::ElidedLabel(QWidget *parent, Qt::WindowFlags f) : ElidedLabel("", parent, f) { }

ElidedLabel::ElidedLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(text, parent, f), m_original(""), m_defaultType(Qt::ElideRight), m_eliding(false)
{
    setText(text);
}

void ElidedLabel::setType(Qt::TextElideMode type)
{
    m_defaultType = type;
    elide();
}

QString const &ElidedLabel::text() const
{
    return m_original;
}

void ElidedLabel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    QTimer::singleShot(50, this, SLOT(elide()));
}

void ElidedLabel::setText(const QString &text)
{
    m_original = text;
    QLabel::setText(text);

    elide();
}

void ElidedLabel::elide()
{
    if (!m_eliding) {
        m_eliding = true;

        QFontMetrics metrics(font());
        QLabel::setText(metrics.elidedText(m_original, m_defaultType, width()));

        m_eliding = false;
    }
}

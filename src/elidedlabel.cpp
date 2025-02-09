#include "elidedlabel.h"

#include <QResizeEvent>
#include <QTimer>

ElidedLabel::ElidedLabel(QWidget *parent, Qt::WindowFlags f) : ElidedLabel("", parent, f) { }

ElidedLabel::ElidedLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(text, parent, f), original(""), defaultType(Qt::ElideRight), eliding(false)
{
    setText(text);
}

void ElidedLabel::setType(const Qt::TextElideMode type)
{
    defaultType = type;
    elide();
}

QString const &ElidedLabel::text() const
{
    return original;
}

void ElidedLabel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    QTimer::singleShot(50, this, SLOT(elide()));
}

void ElidedLabel::setText(const QString &text)
{
    original = text;
    QLabel::setText(text);

    elide();
}

void ElidedLabel::elide()
{
    if (!eliding) {
        eliding = true;

        QFontMetrics metrics(font());
        QLabel::setText(metrics.elidedText(original, defaultType, width()));

        eliding = false;
    }
}

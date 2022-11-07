#include "elidedlabel.h"

#include <QResizeEvent>
#include <QTimer>

ElidedLabel::ElidedLabel(QWidget *parent, Qt::WindowFlags f) : QLabel(parent, f)
{
    defaultType = Qt::ElideRight;
    eliding = false;
    original = "";
}

ElidedLabel::ElidedLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(text, parent, f)
{
    defaultType = Qt::ElideRight;
    eliding = false;
    setText(text);
}

void ElidedLabel::setType(const Qt::TextElideMode type)
{
    defaultType = type;
    elide();
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
    if (eliding == false) {
        eliding = true;

        QFontMetrics metrics(font());
        QLabel::setText(metrics.elidedText(original, defaultType, width()));

        eliding = false;
    }
}

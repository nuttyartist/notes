#include "splitterstyle.h"

SplitterStyle::SplitterStyle(QObject *parent) : QProxyStyle()
{
    setParent(parent); // to ensure proper object destruction
}

void SplitterStyle::drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    if (element != QStyle::CE_Splitter) {
        QProxyStyle::drawControl(element, opt, p, w);
    }
}

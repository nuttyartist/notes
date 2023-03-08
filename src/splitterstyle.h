#ifndef SPLITTERSTYLE_H
#define SPLITTERSTYLE_H

#include <QProxyStyle>

class SplitterStyle : public QProxyStyle
{
public:
    SplitterStyle();

    // QStyle interface
public:
    virtual void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                             const QWidget *w) const override;
};

#endif // SPLITTERSTYLE_H

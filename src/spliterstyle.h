#ifndef SPLITERSTYLE_H
#define SPLITERSTYLE_H

#include <QProxyStyle>

class SpliterStyle : public QProxyStyle
{
public:
    SpliterStyle();

    // QStyle interface
public:
    virtual void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w) const override;
};

#endif // SPLITERSTYLE_H

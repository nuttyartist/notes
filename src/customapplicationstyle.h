#ifndef CUSTOMAPPLICATIONSTYLE_H
#define CUSTOMAPPLICATIONSTYLE_H

#include <QProxyStyle>
#include "styleeditorwindow.h"

class CustomApplicationStyle : public QProxyStyle
{
public:
    CustomApplicationStyle();

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                       const QWidget *widget) const;
    void setTheme(Theme newTheme);

private:
    Theme m_theme;
};

#endif // CUSTOMAPPLICATIONSTYLE_H

#ifndef CUSTOMAPPLICATIONSTYLE_H
#define CUSTOMAPPLICATIONSTYLE_H

#include <QProxyStyle>
#include "editorsettingsoptions.h"

class CustomApplicationStyle : public QProxyStyle
{
public:
    CustomApplicationStyle();

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                       const QWidget *widget) const;
    void setTheme(Theme::Value theme);

private:
    Theme::Value m_theme;
};

#endif // CUSTOMAPPLICATIONSTYLE_H

#include "customapplicationstyle.h"
#include <QPainter>
#include <QStyleOption>
#include "editorsettingsoptions.h"

void CustomApplicationStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                                           QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::PE_IndicatorItemViewItemDrop) {
        painter->setRenderHint(QPainter::Antialiasing, true);

        QColor color(207, 207, 207);
        QPen pen(color);
        pen.setWidth(2);
        painter->setPen(pen);

        if (option->rect.height() == 0) {
            color.setAlpha(50);
            painter->setBrush(color);
            painter->drawLine(option->rect.topLeft(), option->rect.topRight());
        } else {
            color.setAlpha(200);
            painter->fillRect(option->rect, color);
        }
    } else {
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}

void CustomApplicationStyle::setTheme(Theme::Value theme)
{
    m_theme = theme;
}

CustomApplicationStyle::CustomApplicationStyle() : m_theme(Theme::Light) { }

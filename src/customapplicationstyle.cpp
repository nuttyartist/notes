#include "customapplicationstyle.h"
#include <QPainter>
#include <QStyleOption>

void CustomApplicationStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                                           QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::PE_IndicatorItemViewItemDrop) {
        painter->setRenderHint(QPainter::Antialiasing, true);

        QColor c;
        //        if (m_theme == Theme::Dark) {
        //            c = QColor(15, 45, 90);
        //        } else {
        c = QColor(207, 207, 207);
        //        }
        QPen pen(c);
        pen.setWidth(2);
        c.setAlpha(50);
        QBrush brush(c);

        painter->setPen(pen);
        painter->setBrush(brush);
        if (option->rect.height() == 0) {
            painter->drawLine(option->rect.topLeft(), option->rect.topRight());
        } else {
            c.setAlpha(200);
            QBrush brush(c);
            painter->fillRect(option->rect, brush);
        }
    } else {
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}

void CustomApplicationStyle::setTheme(Theme newTheme)
{
    m_theme = newTheme;
}

CustomApplicationStyle::CustomApplicationStyle() : m_theme(Theme::Light) { }

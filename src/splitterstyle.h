#ifndef SPLITTERSTYLE_H
#define SPLITTERSTYLE_H

#include <QProxyStyle>

class SplitterStyle : public QProxyStyle
{
    Q_OBJECT
public:
    SplitterStyle(QObject *parent);

public:
    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                     const QWidget *w) const override;
};

#endif // SPLITTERSTYLE_H

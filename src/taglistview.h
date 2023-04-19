#pragma once

#include <QListView>
#include "theme.h"

class TagListView : public QListView
{
    Q_OBJECT
public:
    explicit TagListView(QWidget *parent = nullptr);
    void setTheme(Theme theme);
    void setBackground(const QColor color);
signals:
    // QAbstractItemView interface
public slots:
    virtual void reset() override;

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private:
    QColor m_backgroundColor;
};

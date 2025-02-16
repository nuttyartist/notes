#pragma once

#include <QListView>
#include "editorsettingsoptions.h"

class TagListView : public QListView
{
    Q_OBJECT
public:
    explicit TagListView(QWidget *parent = nullptr);
    void setTheme(Theme::Value theme);
    void setBackground(QColor color);
signals:
    // QAbstractItemView interface
public slots:
    void reset() override;

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QColor m_backgroundColor;
};

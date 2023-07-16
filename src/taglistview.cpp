#include "taglistview.h"
#include <QFile>
#include <QDebug>
#include <QMouseEvent>

TagListView::TagListView(QWidget *parent) : QListView(parent)
{
    setFlow(QListView::LeftToRight);
    setSpacing(3);
    setWrapping(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QFile file(":/styles/taglistview.css");
    file.open(QFile::ReadOnly);
    setStyleSheet(file.readAll());
    setTheme(Theme::Light);
}

void TagListView::setTheme(Theme::Value theme)
{
    setCSSThemeAndUpdate(this, theme);
}

void TagListView::setBackground(const QColor color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        QString ss = QStringLiteral(
                R"(QListView { background: %1; } )"
                R"(QScrollBar::handle:vertical:hover { background: rgb(170, 170, 171); } )"
                R"(QScrollBar::handle:vertical:pressed { background: rgb(149, 149, 149); } )"
                R"(QScrollBar::handle:vertical { border-radius: 4px; background: rgb(188, 188, 188); min-height: 20px; }  )"
                R"(QScrollBar::vertical {border-radius: 4px; width: 8px; color: rgba(255, 255, 255,0);} )"
                R"(QScrollBar {margin: 0; background: transparent;} )"
                R"(QScrollBar:hover { background-color: rgb(217, 217, 217);})"
                R"(QScrollBar::add-line:vertical { width:0px; height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  )"
                R"(QScrollBar::sub-line:vertical { width:0px; height: 0px; subcontrol-position: top; subcontrol-origin: margin; })");
        setStyleSheet(ss.arg(m_backgroundColor.name()));
    }
}

void TagListView::reset()
{
    QListView::reset();
    auto sz = sizeHint();
    if (!model() || model()->rowCount() == 0) {
        sz.setHeight(0);
    } else {
        auto firstIndex = model()->index(0, 0);
        auto lastIndex = model()->index(model()->rowCount() - 1, 0);
        auto fr = visualRect(firstIndex);
        fr.setBottom(visualRect(lastIndex).bottom());
        if (fr.height() < 80) {
            sz.setHeight(fr.height() + 10);
        } else {
            sz.setHeight(80);
        }
    }
    setFixedHeight(sz.height());
}

void TagListView::resizeEvent(QResizeEvent *event)
{
    QListView::resizeEvent(event);
    setWrapping(true);
}

void TagListView::mousePressEvent(QMouseEvent *event)
{
    event->ignore();
}

void TagListView::mouseReleaseEvent(QMouseEvent *event)
{
    event->ignore();
}

void TagListView::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->ignore();
}

void TagListView::mouseMoveEvent(QMouseEvent *event)
{
    event->ignore();
}

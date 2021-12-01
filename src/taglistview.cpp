#include "taglistview.h"
#include <QDebug>

TagListView::TagListView(QWidget *parent) : QListView(parent)
{
    setFlow(QListView::LeftToRight);
    setSpacing(3);
    setWrapping(true);
}

QSize TagListView::sizeHint() const
{
    if (!model() || model()->rowCount() == 0) {
        return QSize(width(), 0);
    }
    qDebug() << sizeHintForColumn(0);
    return QListView::sizeHint();
//    int nToShow = _nItemsToShow < model()->rowCount() ? _nItemsToShow : model()->rowCount();
    //    return QSize(width(), nToShow*sizeHintForRow(0));
}

void TagListView::reset()
{
    qDebug() <<     contentsSize() << contentsRect() << viewportSizeHint() << childrenRect();
    QListView::reset();
}

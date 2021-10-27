#include "taglistview.h"

TagListView::TagListView(QWidget *parent) : QListView(parent)
{
    setFlow(QListView::LeftToRight);
    setSpacing(3);
}

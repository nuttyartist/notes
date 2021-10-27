#ifndef TAGLISTVIEW_H
#define TAGLISTVIEW_H

#include <QListView>

class TagListView : public QListView
{
    Q_OBJECT
public:
    explicit TagListView(QWidget *parent = nullptr);

signals:

};

#endif // TAGLISTVIEW_H

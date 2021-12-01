#ifndef TAGLISTVIEW_H
#define TAGLISTVIEW_H

#include <QListView>

class TagListView : public QListView
{
    Q_OBJECT
public:
    explicit TagListView(QWidget *parent = nullptr);

signals:


    // QWidget interface
public:
    virtual QSize sizeHint() const override;

    // QAbstractItemView interface
public slots:
    virtual void reset() override;
};

#endif // TAGLISTVIEW_H

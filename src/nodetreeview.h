#ifndef NODETREEVIEW_H
#define NODETREEVIEW_H

#include <QTreeView>

class NodeTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit NodeTreeView(QWidget* parent = Q_NULLPTR);

signals:

private slots:
    void onClicked(const QModelIndex& index);
};

#endif // NODETREEVIEW_H

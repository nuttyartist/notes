#ifndef NODETREEVIEW_H
#define NODETREEVIEW_H

#include <QTreeView>

class QMenu;

class NodeTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit NodeTreeView(QWidget* parent = Q_NULLPTR);

signals:
    void addFolderRequested();

private slots:
    void onClicked(const QModelIndex& index);
    void onCustomContextMenu(const QPoint& point);

private:
    QMenu* contextMenu;
};

#endif // NODETREEVIEW_H

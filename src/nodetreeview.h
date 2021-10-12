#ifndef NODETREEVIEW_H
#define NODETREEVIEW_H

#include <QTreeView>

class QMenu;
class QAction;

class NodeTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit NodeTreeView(QWidget* parent = Q_NULLPTR);

    void setTreeSeparator(const QVector<QModelIndex> &newTreeSeparator);

public slots:
    void onCustomContextMenu(const QPoint& point);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

signals:
    void addFolderRequested();
    void renameFolderRequested();
    void deleteFolderRequested();
    void loadNotesRequested(int folderID, bool isRecursive);

private slots:
    void onClicked(const QModelIndex& index);

private:
    QMenu* contextMenu;
    QAction* renameFolderAction;
    QAction* deleteFolderAction;
    QAction* addSubfolderAction;

    QVector<QModelIndex> m_treeSeparator;
    QModelIndex m_currentHoveringIndex;
    bool m_isContextMenuOpened;
};

#endif // NODETREEVIEW_H

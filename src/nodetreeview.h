#ifndef NODETREEVIEW_H
#define NODETREEVIEW_H

#include <QTreeView>
#include <QTimer>

class QMenu;
class QAction;

class NodeTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit NodeTreeView(QWidget* parent = Q_NULLPTR);

    void setTreeSeparator(const QVector<QModelIndex> &newTreeSeparator);
    void setIsEditing(bool newIsEditing);
    void onRenameFolderFinished(const QString& newName);

public slots:
    void onCustomContextMenu(const QPoint& point);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

signals:
    void addFolderRequested();
    void renameFolderRequested();
    void renameFolderInDatabase(const QModelIndex& index, const QString& newName);
    void deleteNodeRequested(const QModelIndex& index);
    void loadNotesRequested(int folderID, bool isRecursive);

private slots:
    void onClicked(const QModelIndex& index);
    void onDeleteNodeAction();

private:
    QMenu* contextMenu;
    QAction* renameFolderAction;
    QAction* deleteFolderAction;
    QAction* addSubfolderAction;
    QTimer contextMenuTimer;

    QVector<QModelIndex> m_treeSeparator;
    QModelIndex m_currentEditingIndex;
    bool m_isContextMenuOpened;
    bool m_isEditing;

    void updateEditingIndex(QMouseEvent* event);
    void closeCurrentEditor();
};

#endif // NODETREEVIEW_H

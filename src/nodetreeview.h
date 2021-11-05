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
    void onRenameTagFinished(const QString& newName);
    void onCurrentChanged(const QModelIndex& current);

public slots:
    void onCustomContextMenu(const QPoint& point);
    void onChangeTagColorAction();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

signals:
    void addFolderRequested();
    void renameFolderRequested();
    void renameFolderInDatabase(const QModelIndex& index, const QString& newName);
    void renameTagInDatabase(const QModelIndex& index, const QString& newName);
    void deleteNodeRequested(const QModelIndex& index);
    void loadNotesInFolderRequested(int folderID, bool isRecursive);
    void loadNotesInTagRequested(int tagId);

    void renameTagRequested();
    void changeTagColorRequested(const QModelIndex& index);
    void deleteTagRequested(const QModelIndex& index);
private slots:
    void onClicked(const QModelIndex& index);
    void onDeleteNodeAction();
private:
    QMenu* contextMenu;
    QAction* renameFolderAction;
    QAction* deleteFolderAction;
    QAction* addSubfolderAction;
    QAction* renameTagAction;
    QAction* changeTagColorAction;
    QAction* deleteTagAction;
    QAction* clearSelectionAction;

    QTimer contextMenuTimer;

    QVector<QModelIndex> m_treeSeparator;
    QModelIndex m_currentEditingIndex;
    bool m_isContextMenuOpened;
    bool m_isEditing;

    void updateEditingIndex(QMouseEvent* event);
    void closeCurrentEditor();
    // QAbstractItemView interface
protected slots:
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
};

#endif // NODETREEVIEW_H

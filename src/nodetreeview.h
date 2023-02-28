#ifndef NODETREEVIEW_H
#define NODETREEVIEW_H

#include <QTreeView>
#include <QTimer>
#include "styleeditorwindow.h"
#include "nodedata.h"

class QMenu;
class QAction;
class NodeTreeViewPrivate;
struct NoteTreeConstant
{
    static constexpr int folderItemHeight = 30;
    static constexpr int tagItemHeight = 30;
    static constexpr int folderLabelHeight = 25;
    static constexpr int tagLabelHeight = 25;
};

class NodeTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit NodeTreeView(QWidget *parent = nullptr);

    void setTreeSeparator(const QVector<QModelIndex> &newTreeSeparator,
                          const QModelIndex &defaultNotesIndex);
    void setIsEditing(bool newIsEditing);
    void onRenameFolderFinished(const QString &newName);
    void onRenameTagFinished(const QString &newName);
    void setCurrentIndexC(const QModelIndex &index);
    void setCurrentIndexNC(const QModelIndex &index);
    void setTheme(Theme theme);
    Theme theme() const;
    bool isDragging() const;
    void reExpandC();
    void reExpandC(const QStringList &expanded);

    void setIgnoreThisCurrentLoad(bool newIgnoreThisCurrentLoad);
    const QModelIndex &currentEditingIndex() const;

public slots:
    void onCustomContextMenu(QPoint point);
    void onChangeTagColorAction();
    void onRequestExpand(const QString &folderPath);
    void onUpdateAbsPath(const QString &oldPath, const QString &newPath);
    void onFolderDropSuccessfull(const QString &path);
    void onTagsDropSuccessfull(const QSet<int> &ids);

signals:
    void addFolderRequested();
    void renameFolderRequested();
    void renameFolderInDatabase(const QModelIndex &index, const QString &newName);
    void renameTagInDatabase(const QModelIndex &index, const QString &newName);
    void deleteNodeRequested(const QModelIndex &index);
    void loadNotesInFolderRequested(int folderID, bool isRecursive, bool notInterested = false,
                                    int scrollToId = SpecialNodeID::InvalidNodeId);
    void loadNotesInTagsRequested(const QSet<int> &tagIds, bool notInterested = false,
                                  int scrollToId = SpecialNodeID::InvalidNodeId);
    void moveNodeRequested(int node, int target);
    void renameTagRequested();
    void changeTagColorRequested(const QModelIndex &index);
    void deleteTagRequested(const QModelIndex &index);
    void addNoteToTag(int noteId, int tagId);
    void saveExpand(const QStringList &ex);
    void saveSelected(bool isSelectingFolder, const QString &folder, const QSet<int> &tags);
    void saveLastSelectedNote();
    void requestLoadLastSelectedNote();

private slots:
    void onDeleteNodeAction();
    void onExpanded(const QModelIndex &index);
    void onCollapsed(const QModelIndex &index);

private:
    QMenu *contextMenu;
    QAction *renameFolderAction;
    QAction *deleteFolderAction;
    QAction *addSubfolderAction;
    QAction *renameTagAction;
    QAction *changeTagColorAction;
    QAction *deleteTagAction;
    QAction *clearSelectionAction;
    QTimer contextMenuTimer;
    QVector<QModelIndex> m_treeSeparator;
    QModelIndex m_defaultNotesIndex;
    QModelIndex m_currentEditingIndex;
    bool m_isContextMenuOpened;
    bool m_isEditing;
    Theme m_theme;
    QVector<QString> m_expanded;
    QModelIndex m_needReleaseIndex;
    bool m_ignoreThisCurrentLoad;
    QString m_lastSelectFolder;
    bool m_isLastSelectedFolder;
    void updateEditingIndex(QPoint pos);
    void closeCurrentEditor();

    // QAbstractItemView interface
protected slots:
    virtual void selectionChanged(const QItemSelection &selected,
                                  const QItemSelection &deselected) override;
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

    // QWidget interface
protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;

    // QAbstractItemView interface
public slots:
    virtual void reset() override;

private:
    Q_DECLARE_PRIVATE(NodeTreeView)
};

#endif // NODETREEVIEW_H

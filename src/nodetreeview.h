#ifndef NODETREEVIEW_H
#define NODETREEVIEW_H

#include <QTreeView>
#include <QTimer>
#include "nodedata.h"
#include "editorsettingsoptions.h"

class QMenu;
class QAction;
class NodeTreeViewPrivate;
struct NoteTreeConstant
{
    static constexpr int FOLDER_ITEM_HEIGHT = 30;
    static constexpr int TAG_ITEM_HEIGHT = 30;
    static constexpr int FOLDER_LABEL_HEIGHT = 35;
    static constexpr int TAG_LABEL_HEIGHT = 35;
};

class NodeTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit NodeTreeView(QWidget *parent = nullptr);

    void setTreeSeparator(const QVector<QModelIndex> &newTreeSeparator, const QModelIndex &defaultNotesIndex);
    void setIsEditing(bool newIsEditing);
    void onRenameFolderFinished(const QString &newName);
    void onRenameTagFinished(const QString &newName);
    void setCurrentIndexC(const QModelIndex &index);
    void setCurrentIndexNC(const QModelIndex &index);
    void setTheme(Theme::Value theme);
    Theme::Value theme() const;
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
    void onFolderDropSuccessful(const QString &path);
    void onTagsDropSuccessful(const QSet<int> &ids);

signals:
    void addFolderRequested();
    void renameFolderRequested();
    void renameFolderInDatabase(const QModelIndex &index, const QString &newName);
    void renameTagInDatabase(const QModelIndex &index, const QString &newName);
    void deleteNodeRequested(const QModelIndex &index);
    void loadNotesInFolderRequested(int folderID, bool isRecursive, bool notInterested = false, int scrollToId = INVALID_NODE_ID);
    void loadNotesInTagsRequested(const QSet<int> &tagIds, bool notInterested = false, int scrollToId = INVALID_NODE_ID);
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
    QMenu *m_contextMenu;
    QAction *m_renameFolderAction;
    QAction *m_deleteFolderAction;
    QAction *m_addSubfolderAction;
    QAction *m_renameTagAction;
    QAction *m_changeTagColorAction;
    QAction *m_deleteTagAction;
    QAction *m_clearSelectionAction;
    QTimer m_contextMenuTimer;
    QVector<QModelIndex> m_treeSeparator;
    QModelIndex m_defaultNotesIndex;
    QModelIndex m_currentEditingIndex;
    bool m_isContextMenuOpened;
    bool m_isEditing;
    Theme::Value m_theme;
    QVector<QString> m_expanded;
    QModelIndex m_needReleaseIndex;
    bool m_ignoreThisCurrentLoad;
    QString m_lastSelectFolder;
    bool m_isLastSelectedFolder;
    void updateEditingIndex(QPoint pos);
    void closeCurrentEditor();

    // QAbstractItemView interface
protected slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

    // QWidget interface
protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

    // QAbstractItemView interface
public slots:
    void reset() override;

private:
    Q_DECLARE_PRIVATE(NodeTreeView)
};

#endif // NODETREEVIEW_H

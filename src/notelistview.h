#ifndef NOTELISTVIEW_H
#define NOTELISTVIEW_H

#include <QListView>
#include <QScrollArea>
#include "styleeditorwindow.h"

class TagPool;
class DBManager;

class NoteListView : public QListView
{
    Q_OBJECT

public:
    explicit NoteListView(QWidget* parent = Q_NULLPTR);
    ~NoteListView();

    void animateAddedRow(const QModelIndex &parent, int start, int end);
    void setAnimationEnabled(bool isEnabled);
    void setCurrentRowActive(bool isActive);
    void setTheme(Theme theme);    
    void setTagPool(TagPool *newTagPool);
    void setIsInTrash(bool newIsInTrash);
    void setDbManager(DBManager *newDbManager);

    void setCurrentFolderId(int newCurrentFolderId);
    void openPersistentEditorC(const QModelIndex& index);
    void closePersistentEditorC(const QModelIndex& index);
    void closeAllEditor();

public slots:
    void onCustomContextMenu(const QPoint& point);
    void onTagsMenu(const QPoint& point);

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* e) Q_DECL_OVERRIDE;
    bool viewportEvent(QEvent* e) Q_DECL_OVERRIDE;
    // QAbstractScrollArea interface
protected:
    virtual void scrollContentsBy(int dx, int dy) override;

public slots:
    void rowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                            const QModelIndex &destinationParent, int destinationRow);

    void rowsMoved(const QModelIndex &parent, int start, int end,
                   const QModelIndex &destination, int row);

private slots:
    void init();

protected slots:
    void rowsInserted(const QModelIndex &parent, int start, int end) Q_DECL_OVERRIDE;
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) Q_DECL_OVERRIDE;

signals:
    void viewportPressed();
    void addTagRequested(const QModelIndex& index, int tadId);
    void removeTagRequested(const QModelIndex& index, int tadId);
    void deleteNoteRequested(const QModelIndex& index);
    void restoreNoteRequested(const QModelIndex& index);
    void newNoteRequested();
    void moveNoteRequested(int noteId, int folderId);
private:
    bool m_isScrollBarHidden;
    bool m_animationEnabled;
    bool m_isMousePressed;
    int m_rowHeight;
    QColor m_currentBackgroundColor;
    QMenu* contextMenu;
    QAction* addToTagAction;
    QAction* deleteNoteAction;
    QAction* restoreNoteAction;
    QAction* pinNoteAction;
    QAction* newNoteAction;
    QMenu* tagsMenu;
    TagPool* m_tagPool;
    DBManager* m_dbManager;
    int m_currentFolderId;
    QVector<QAction*> m_noteTagActions;
    QVector<QAction*> m_folderActions;
    bool m_isInTrash;
    QPoint m_dragStartPosition;
    QPixmap m_dragPixmap;
    QSet<QModelIndex> m_openedEditor;

    void setupSignalsSlots();
    void setupStyleSheet();

    void addCurrentNoteToTag(int tagId);
    void removeCurrentNoteFromTag(int tagId);

};

#endif // NOTELISTVIEW_H

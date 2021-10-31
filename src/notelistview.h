#ifndef NOTELISTVIEW_H
#define NOTELISTVIEW_H

#include <QListView>
#include <QScrollArea>

class TagPool;

class NoteListView : public QListView
{
    Q_OBJECT

public:
    enum class Theme {
        Light,
        Dark,
        Sepia
    };

    explicit NoteListView(QWidget* parent = Q_NULLPTR);
    ~NoteListView();

    void animateAddedRow(const QModelIndex &parent, int start, int end);
    void setAnimationEnabled(bool isEnabled);
    void setCurrentRowActive(bool isActive);
    void setTheme(Theme theme);    
    void setTagPool(TagPool *newTagPool);

public slots:
    void onCustomContextMenu(const QPoint& point);
    void onTagsMenu(const QPoint& point);

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* e) Q_DECL_OVERRIDE;
    bool viewportEvent(QEvent* e) Q_DECL_OVERRIDE;

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
private:
    bool m_isScrollBarHidden;
    bool m_animationEnabled;
    bool m_isMousePressed;
    int m_rowHeight;
    QColor m_currentBackgroundColor;
    QMenu* contextMenu;
    QAction* addToTagAction;
    QAction* deleteNoteAction;
    QMenu* tagsMenu;
    TagPool* m_tagPool;
    QVector<QAction*> m_addTagActions;

    void setupSignalsSlots();
    void setupStyleSheet();

    void addCurrentNoteToTag(int tagId);
};

#endif // NOTELISTVIEW_H
#ifndef NOTELISTDELEGATEEDITOR_H
#define NOTELISTDELEGATEEDITOR_H

#include <QWidget>
#include "notelistview.h"
#include <QTimeLine>

class NoteListDelegate;
class TagListModel;
class TagListView;
class TagListDelegate;
class NoteListModel;
struct NoteListConstant {
    static constexpr int leftOffsetX = 10;
    static constexpr int topOffsetY = 5;   // space on top of title
    static constexpr int titleDateSpace = 1;  // space between title and date
    static constexpr int dateDescSpace = 4;  // space between date and description
    static constexpr int descFolderSpace = 9;  // space between description and folder name
    static constexpr int lastElSepSpace = 10; // space between the last element and the seperator
    static constexpr int nextNoteOffset = 0; // space between the seperator and the next note underneath it
    static constexpr int pinnedHeaderToNoteSpace = 0; // space between Pinned label to the pinned list
    static constexpr int unpinnedHeaderToNoteSpace = 0; // space between Notes label and the normal notes list
    static constexpr int lastPinnedToUnpinnedHeader = 0; // space between the last pinned note to Notes label
};

class NoteListDelegateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit NoteListDelegateEditor(const NoteListDelegate* delegate,
                                    QListView *view,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index,
                                    TagPool *tagPool,
                                    QWidget *parent = nullptr);
    ~NoteListDelegateEditor();

    void setRowRightOffset(int rowRightOffset);
    void setActive(bool isActive);
    void recalculateSize();
    void setScrollBarPos(int pos);
    int getScrollBarPos();
    bool underMouseC() const;
    QPixmap renderToPixmap();

public slots:
    void setTheme(Theme theme);
signals:
    void updateSizeHint(int id, const QSize& sz, const QModelIndex& index);
    void nearDestroyed(int id, const QModelIndex& index);

private:
    void paintBackground(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index)const;
    void paintLabels(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintSeparator(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QString parseDateTime(const QDateTime& dateTime) const;

    const NoteListDelegate* m_delegate;
    QStyleOptionViewItem m_option;
    int m_id;
    QListView *m_view;

    TagPool* m_tagPool;
    QString m_displayFont;
    QFont m_titleFont;
    QFont m_titleSelectedFont;
    QFont m_dateFont;
    QFont m_headerFont;
    QColor m_titleColor;
    QColor m_dateColor;
    QColor m_contentColor;
    QColor m_ActiveColor;
    QColor m_notActiveColor;
    QColor m_hoverColor;
    QColor m_applicationInactiveColor;
    QColor m_separatorColor;
    QColor m_defaultColor;
    int m_rowHeight;
    int m_rowRightOffset;
    bool m_isActive;
    QImage m_folderIcon;
    QImage m_pinnedExpandIcon;
    QImage m_pinnedCollapseIcon;
    Theme m_theme;
    bool m_containsMouse;
    QModelIndex m_animatedIndex;

    TagListView* m_tagListView;
    TagListModel* m_tagListModel;
    TagListDelegate* m_tagListDelegate;
    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
};

#endif // NOTELISTDELEGATEEDITOR_H

#ifndef NOTELISTDELEGATEEDITOR_H
#define NOTELISTDELEGATEEDITOR_H

#include <QWidget>
#include "notelistview.h"
#include <QTimeLine>

class NoteListDelegate;
class TagListModel;
class TagListView;
class TagListDelegate;
struct NoteListConstant {
    static constexpr int leftOffsetX = 10;
    static constexpr int topOffsetY = 5;   // space on top of title
    static constexpr int titleDateSpace = 1;  // space between title and date
    static constexpr int dateDescSpace = 4;  // space between date and description
    static constexpr int descFolderSpace = 9;  // space between description and folder name
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

    void setRowRightOffset(int rowRightOffset);
    void setActive(bool isActive);
    void recalculateSize();
    void setScrollBarPos(int pos);
    int getScrollBarPos();

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
    QImage m_pinnedIcon;
    Theme m_theme;
    QModelIndex m_animatedIndex;

    TagListView* m_tagListView;
    TagListModel* m_tagListModel;
    TagListDelegate* m_tagListDelegate;
    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // NOTELISTDELEGATEEDITOR_H

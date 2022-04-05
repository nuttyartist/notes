#ifndef NOTELISTDELEGATE_H
#define NOTELISTDELEGATE_H

#include "notelistview.h"
#include <QStyledItemDelegate>
#include <QTimeLine>

class TagPool;

enum class NoteListState{
    Normal,
    Insert,
    Remove,
    MoveOut,
    MoveIn
};

class NoteListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    NoteListDelegate(NoteListView* view, TagPool *tagPool, QObject *parent = Q_NULLPTR);

    void setState(NoteListState NewState , QModelIndex index);
    void setAnimationDuration(const int duration);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize bufferSizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;

    QTimeLine::State animationState();

    void setCurrentSelectedIndex(const QModelIndex &currentSelectedIndex);
    void setHoveredIndex(const QModelIndex &hoveredIndex);
    void setRowRightOffset(int rowRightOffset);
    void setActive(bool isActive);
    void setTheme(Theme theme);
    Theme theme() const;
    void setIsInAllNotes(bool newIsInAllNotes);
    bool isInAllNotes() const;
    void clearSizeMap();

public slots:
    void updateSizeMap(int id, const QSize& sz, const QModelIndex& index);
    void editorDestroyed(int id, const QModelIndex& index);

    // QAbstractItemDelegate interface
public:
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    const QModelIndex &currentSelectedIndex() const;
    const QModelIndex &hoveredIndex() const;

signals:
    void themeChanged(Theme theme);
    void animationFinished(NoteListState animationState);

private:
    void paintBackground(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index)const;
    void paintLabels(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintSeparator(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
    void paintTagList(int top, QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QString parseDateTime(const QDateTime& dateTime) const;

    NoteListView* m_view;
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
    int m_maxFrame;
    int m_rowRightOffset;
    NoteListState m_state;
    bool m_isActive;
    bool m_isInAllNotes;
    QImage m_folderIcon;
    QImage m_pinnedExpandIcon;
    QImage m_pinnedCollapseIcon;
    Theme m_theme;
    QTimeLine *m_timeLine;
    QModelIndex m_animatedIndex;
    QModelIndex m_currentSelectedIndex;
    QModelIndex m_hoveredIndex;    
    QMap<int, QSize> szMap;
};

#endif // NOTELISTDELEGATE_H

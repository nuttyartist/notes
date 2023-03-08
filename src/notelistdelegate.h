#ifndef NOTELISTDELEGATE_H
#define NOTELISTDELEGATE_H

#include "notelistview.h"
#include <QStyledItemDelegate>
#include <QTimeLine>
#include <QQueue>

class TagPool;
class NoteListModel;
enum class NoteListState { Normal, Insert, Remove, MoveOut, MoveIn };

class NoteListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    NoteListDelegate(NoteListView *view, TagPool *tagPool, QObject *parent = nullptr);

    void setState(NoteListState NewState, QModelIndexList indexes);
    void setAnimationDuration(const int duration);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize bufferSizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QTimeLine::State animationState();

    void setHoveredIndex(const QModelIndex &hoveredIndex);
    void setRowRightOffset(int rowRightOffset);
    void setActive(bool isActive);
    void setTheme(Theme theme);
    Theme theme() const;
    void setIsInAllNotes(bool newIsInAllNotes);
    bool isInAllNotes() const;
    void clearSizeMap();

public slots:
    void updateSizeMap(int id, QSize sz, const QModelIndex &index);
    void editorDestroyed(int id, const QModelIndex &index);

    // QAbstractItemDelegate interface
public:
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const override;
    const QModelIndex &hoveredIndex() const;
    bool shouldPaintSeparator(const QModelIndex &index, const NoteListModel &model) const;

signals:
    void themeChanged(Theme theme);
    void animationFinished(NoteListState animationState);

private:
    void paintBackground(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const;
    void paintLabels(QPainter *painter, const QStyleOptionViewItem &option,
                     const QModelIndex &index) const;
    void paintSeparator(QPainter *painter, QRect rect, const QModelIndex &index) const;
    void paintTagList(int top, QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const;
    QString parseDateTime(const QDateTime &dateTime) const;
    void setStateI(NoteListState NewState, const QModelIndexList &indexes);

    NoteListView *m_view;
    TagPool *m_tagPool;
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
    QModelIndexList m_animatedIndexes;
    QModelIndex m_hoveredIndex;
    QMap<int, QSize> szMap;
    QQueue<QPair<QSet<int>, NoteListState>> animationQueue;
};

#endif // NOTELISTDELEGATE_H

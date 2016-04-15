#ifndef NOTEWIDGETDELEGATE_H
#define NOTEWIDGETDELEGATE_H

#include <QStyledItemDelegate>
#include <QTimeLine>

class NoteWidgetDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    NoteWidgetDelegate(QObject *parent = Q_NULLPTR);

    enum States{
        Normal,
        Insert,
        Remove,
        MoveOut,
        MoveIn
    };

    void setState(States NewState , QModelIndex index);
    void setAnimationDuration(const int duration);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

    QTimeLine::State animationState();

    void setCurrentSelectedIndex(const QModelIndex &currentSelectedIndex);
    void setHoveredIndex(const QModelIndex &hoveredIndex);
    void setRowRightOffset(int rowRightOffset);

private:
    void paintBackground(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index)const;
    void paintTitle(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintDateTime(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintSeparator(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QString parseDateTime(const QDateTime& dateTime) const;

    QFont m_titleFont;
    QFont m_dateFont;
    QColor m_titleColor;
    QColor m_dateColor;
    QColor m_focusColor;
    QColor m_noFocusColor;
    QColor m_hoverColor;
    QColor m_separatorColor;
    QColor m_defaultColor;
    int m_rowHeight;
    int m_maxFrame;
    int m_rowRightOffset;
    States m_state;

    QTimeLine *m_timeLine;
    QModelIndex m_animatedIndex;
    QModelIndex m_currentSelectedIndex;
    QModelIndex m_hoveredIndex;

signals:
    void update(const QModelIndex &index);
};

#endif // NOTEWIDGETDELEGATE_H

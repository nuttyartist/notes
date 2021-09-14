#ifndef NOTEWIDGETDELEGATE_H
#define NOTEWIDGETDELEGATE_H

#include "noteview.h"
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
    void setActive(bool isActive);
    void setTheme(NoteView::Theme theme);

private:
    void paintBackground(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index)const;
    void paintLabels(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintSeparator(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QString parseDateTime(const QDateTime& dateTime) const;

    QString m_displayFont;
    QFont m_titleFont;
    QFont m_titleSelectedFont;
    QFont m_dateFont;
    QColor m_titleColor;
    QColor m_dateColor;
    QColor m_ActiveColor;
    QColor m_notActiveColor;
    QColor m_hoverColor;
    QColor m_applicationInactiveColor;
    QColor m_separatorColor;
    QColor m_defaultColor;
    int m_rowHeight;
    int m_maxFrame;
    int m_rowRightOffset;
    States m_state;
    bool m_isActive;

    QTimeLine *m_timeLine;
    QModelIndex m_animatedIndex;
    QModelIndex m_currentSelectedIndex;
    QModelIndex m_hoveredIndex;

signals:
    void update(const QModelIndex &index);
};

#endif // NOTEWIDGETDELEGATE_H

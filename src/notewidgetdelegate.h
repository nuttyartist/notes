#ifndef NOTEWIDGETDELEGATE_H
#define NOTEWIDGETDELEGATE_H

#include <QStyledItemDelegate>

class NoteWidgetDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    NoteWidgetDelegate(QWidget *parent = Q_NULLPTR);

protected:

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    void paintTitle(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintDateTime(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QString parseDateTime(const QDateTime& dateTime) const;

    QFont m_titleFont;
    QFont m_dateFont;
    QColor m_titleColor;
    QColor m_dateColor;
};

#endif // NOTEWIDGETDELEGATE_H

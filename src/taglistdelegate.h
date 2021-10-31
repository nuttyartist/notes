#ifndef TAGLISTDELEGATE_H
#define TAGLISTDELEGATE_H

#include <QStyledItemDelegate>

class TagListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TagListDelegate(QObject *parent = Q_NULLPTR);

    // QAbstractItemDelegate interface
public:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    QString m_displayFont;
    QFont m_titleFont;
    QFont m_titleSelectedFont;
    QFont m_dateFont;
    QColor m_titleColor;
};



#endif // TAGLISTDELEGATE_H
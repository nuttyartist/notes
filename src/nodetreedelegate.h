#ifndef NODETREEDELEGATE_H
#define NODETREEDELEGATE_H

#include <QStyledItemDelegate>

class NodeTreeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit NodeTreeDelegate(QObject *parent = Q_NULLPTR);

signals:


    // QAbstractItemDelegate interface
public:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void paintBackgroundSelectable(QPainter *painter, const QStyleOptionViewItem &option) const;

private:
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
};

#endif // NODETREEDELEGATE_H

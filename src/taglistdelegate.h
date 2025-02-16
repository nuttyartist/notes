#ifndef TAGLISTDELEGATE_H
#define TAGLISTDELEGATE_H

#include <QStyledItemDelegate>
#include "editorsettingsoptions.h"

class TagListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TagListDelegate(QObject *parent = nullptr);

    // QAbstractItemDelegate interface
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setTheme(Theme::Value theme);

private:
    QString m_displayFont;
    QFont m_titleFont;
    QFont m_titleSelectedFont;
    QFont m_dateFont;
    QColor m_titleColor;
    Theme::Value m_theme;
};

#endif // TAGLISTDELEGATE_H

#ifndef NOTELISTVIEW_P_H
#define NOTELISTVIEW_P_H
#include <QtWidgets/private/qabstractitemview_p.h>
#include "notelistview.h"

class NoteListViewPrivate : public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(NoteListView)

public:
    NoteListViewPrivate() : QAbstractItemViewPrivate(){};
    virtual ~NoteListViewPrivate() { }
    QPixmap renderToPixmap(const QModelIndexList &indexes, QRect *r) const;
    QStyleOptionViewItem viewOptionsV1() const;
};

#endif // NOTELISTVIEW_P_H

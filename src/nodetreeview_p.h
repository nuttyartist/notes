#ifndef NODETREEVIEW_P_H
#define NODETREEVIEW_P_H
#include <QtWidgets/private/qabstractitemview_p.h>
#include "nodetreeview.h"

class NodeTreeViewPrivate : public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(NodeTreeView)

public:
    NodeTreeViewPrivate() : QAbstractItemViewPrivate(){};
    virtual ~NodeTreeViewPrivate() { }
};

#endif // NODETREEVIEW_P_H

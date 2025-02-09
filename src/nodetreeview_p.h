#pragma once
#include "nodetreeview.h"

#include <QtWidgets/private/qabstractitemview_p.h>

class NodeTreeViewPrivate : public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(NodeTreeView)

public:
    NodeTreeViewPrivate() : QAbstractItemViewPrivate(){};
    virtual ~NodeTreeViewPrivate() { }
};

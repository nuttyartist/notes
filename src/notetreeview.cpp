#include "notetreeview.h"

NoteTreeView::NoteTreeView(QWidget *parent) : QTreeView(parent)
{
    setHeaderHidden(true);
    setStyleSheet("QTreeView {\n"
                  "   border-style: none;\n"
                  "   background-color: rgb(255, 255, 255);\n"
                  "}\n"
                  "\n"
                  "QTreeView::branch{\n"
                  "   border-image: url(none.png);\n"
                  "}");
    setRootIsDecorated(false);
}

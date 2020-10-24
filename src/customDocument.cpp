#include "customDocument.h"
#include <QDebug>

CustomDocument::CustomDocument(QWidget *parent)
    : QTextEdit(parent)
{
    setViewportMargins(27,0,27,2);
}

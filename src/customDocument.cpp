#include "customDocument.h"
#include <QDebug>

CustomDocument::CustomDocument(QWidget *parent)
    : QTextEdit(parent)
{

}

void CustomDocument::setDocumentPadding(int left, int top, int right, int bottom)
{
    this->setViewportMargins(left, top, right, bottom);
}

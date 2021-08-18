#include "customDocument.h"
#include <QDebug>

CustomDocument::CustomDocument(QWidget *parent)
    : QTextEdit(parent)
{

}

/*!
 * \brief CustomDocument::setDocumentPadding
 * We use a custom document for MainWindow::m_textEdit
 * so we can set the document padding without the (upstream Qt) issue
 * where the vertical scrollbar gets padded with the text as well.
 * This way, only the text gets padded, and the vertical scroll bar stays where it is.
 * \param left
 * \param top
 * \param right
 * \param bottom
 */
void CustomDocument::setDocumentPadding(int left, int top, int right, int bottom)
{
    this->setViewportMargins(left, top, right, bottom);
}

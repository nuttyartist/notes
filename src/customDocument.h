#ifndef CUSTOMDOCUMENT_H
#define CUSTOMDOCUMENT_H

#include <QtGui>
#include <QTextEdit>

class CustomDocument : public QTextEdit
{
    Q_OBJECT
public:
    CustomDocument(QWidget *parent = Q_NULLPTR);
    void setDocumentPadding(int left, int top, int right, int bottom);
};

#endif // CUSTOMDOCUMENT_H

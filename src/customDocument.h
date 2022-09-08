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
signals:
    void resized();

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);
};

#endif // CUSTOMDOCUMENT_H

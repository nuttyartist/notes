#ifndef CUSTOMDOCUMENT_H
#define CUSTOMDOCUMENT_H

#include <QtGui>
#include <QTextEdit>

class CustomDocument : public QTextEdit
{
    Q_OBJECT
public:
    CustomDocument(QWidget *parent = Q_NULLPTR);
};

#endif // CUSTOMDOCUMENT_H

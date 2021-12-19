#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include <QLabel>

class ElidedLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ElidedLabel(QWidget *parent=0, Qt::WindowFlags f=0);
    explicit ElidedLabel(const QString &text, QWidget *parent=0, Qt::WindowFlags f=0);
    void setType(const Qt::TextElideMode type);

public slots:
    void setText(const QString &text);
    void elide();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    QString original;
    Qt::TextElideMode defaultType;
    bool eliding;
};

#endif // ELIDEDLABEL_H

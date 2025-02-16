#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include <QLabel>

class ElidedLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ElidedLabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    explicit ElidedLabel(const QString &text, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    void setType(Qt::TextElideMode type);
    QString const &text() const;

public slots:
    void setText(const QString &text);
    void elide();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QString m_original;
    Qt::TextElideMode m_defaultType;
    bool m_eliding;
};

#endif // ELIDEDLABEL_H

#ifndef LABELEDITTYPE_H
#define LABELEDITTYPE_H

#include "elidedlabel.h"

class QLineEdit;
class LabelEditType : public ElidedLabel
{
    Q_OBJECT
public:
    explicit LabelEditType(QWidget *parent = nullptr);

public slots:
    void openEditor();

private slots:
    void onFinishedEdit();

signals:
    void editingStarted();
    void editingFinished(const QString &text);

private:
    QLineEdit *m_editor;
};

#endif // LABELEDITTYPE_H

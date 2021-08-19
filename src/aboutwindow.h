#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QWidget>

namespace Ui {
class AboutWindow;
}

class AboutWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AboutWindow (QWidget *parent = 0);
    ~AboutWindow();
    void setTheme(QColor backgroundColor, QColor textColor);

public slots:


signals:

private slots:

protected:

private:
    Ui::AboutWindow *m_ui;
};

#endif // ABOUTWINDOW_H

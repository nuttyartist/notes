#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QDialog>

namespace Ui {
class AboutWindow;
}

class AboutWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AboutWindow(QWidget *parent = 0);
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

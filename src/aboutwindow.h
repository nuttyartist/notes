#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QDialog>
#include "editorsettingsoptions.h"

namespace Ui {
class AboutWindow;
}

class AboutWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AboutWindow(QWidget *parent = 0);
    ~AboutWindow();
    void setTheme(Theme::Value theme);
    void setProVersion(bool isProVersion);

public slots:

signals:

private slots:

protected:
private:
    Ui::AboutWindow *m_ui;
    bool m_isProVersion;
    void setAboutText();
};

#endif // ABOUTWINDOW_H

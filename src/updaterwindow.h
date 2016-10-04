/*********************************************************************************************
* Mozila License
* Just a meantime project to see the ability of qt, the framework that my OS might be based on
* And for those linux users that beleive in the power of notes
*********************************************************************************************/

#ifndef UPDATERWINDOW_H
#define UPDATERWINDOW_H

#include <QWidget>

namespace Ui {
class UpdaterWindow;
}

class UpdaterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit UpdaterWindow (QWidget *parent = 0);
    ~UpdaterWindow();

public slots:
    void checkForUpdates();

private:
    Ui::UpdaterWindow *ui;
};

#endif

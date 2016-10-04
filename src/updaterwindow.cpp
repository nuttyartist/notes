/*********************************************************************************************
* Mozila License
* Just a meantime project to see the ability of qt, the framework that my OS might be based on
* And for those linux users that beleive in the power of notes
*********************************************************************************************/

#include "updaterwindow.h"
#include "ui_updaterwindow.h"

const QString UPDATES_URL = "https://raw.githubusercontent.com/nuttyartist/"
                            "notes/dev/UPDATES.json";

UpdaterWindow::UpdaterWindow (QWidget *parent) : QWidget (parent)
{
    /* Initialize the UI */
    ui = new Ui::UpdaterWindow;
    ui->setupUi (this);

    /* Setup the color palettes */
    QPalette pal (palette());
    pal.setColor (QPalette::Background, QColor (248, 248, 248));
    setAutoFillBackground (true);
    setPalette (pal);
}

UpdaterWindow::~UpdaterWindow()
{
    delete ui;
}

void UpdaterWindow::checkForUpdates()
{
    show();
}

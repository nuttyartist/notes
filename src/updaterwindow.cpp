/*********************************************************************************************
* Mozila License
* Just a meantime project to see the ability of qt, the framework that my OS might be based on
* And for those linux users that beleive in the power of notes
*********************************************************************************************/

#include "updaterwindow.h"
#include "ui_updaterwindow.h"

#include <qdebug.h>
#include <QDesktopServices>
#include <QSimpleUpdater.h>

const QString UPDATES_URL = "https://raw.githubusercontent.com/nuttyartist/notes/dev/UPDATES.json";

UpdaterWindow::UpdaterWindow (QWidget *parent) : QWidget (parent)
{
    /* Initialize the UI */
    ui = new Ui::UpdaterWindow;
    ui->setupUi (this);

    /* Set window title to app name */
    setWindowTitle (qApp->applicationName());

    /* Setup the color palettes */
    QPalette pal (palette());
    pal.setColor (QPalette::Background, QColor (248, 248, 248));
    setAutoFillBackground (true);
    setPalette (pal);

    /* Connect UI signals/slots */
    connect (ui->closeButton,  SIGNAL (clicked()), this, SLOT (close()));
    connect (ui->updateButton, SIGNAL (clicked()), this, SLOT (download()));

    /* Connect signals/slots of the QSU */
    m_updater = QSimpleUpdater::getInstance();
    connect (m_updater, SIGNAL (checkingFinished (QString)),
             this,      SLOT (onCheckFinished  (QString)));
}

UpdaterWindow::~UpdaterWindow()
{
    delete ui;
}

void UpdaterWindow::checkForUpdates()
{
    /* Set module properties */
    m_updater->setNotifyOnFinish (UPDATES_URL, false);
    m_updater->setNotifyOnUpdate (UPDATES_URL, false);
    m_updater->setDownloaderEnabled (UPDATES_URL, false);
    m_updater->setUseCustomInstallProcedures (UPDATES_URL, true);
    m_updater->setModuleVersion (UPDATES_URL, qApp->applicationVersion());

    /* Check for updates */
    m_updater->checkForUpdates (UPDATES_URL);
}

void UpdaterWindow::download()
{
    if (m_updater->getUpdateAvailable (UPDATES_URL))
        QDesktopServices::openUrl (QUrl (m_updater->getDownloadUrl (UPDATES_URL)));
}

void UpdaterWindow::showWindow (const QString& url)
{
    /* Set version labels */
    ui->installedVersion->setText (qApp->applicationVersion());
    ui->availableVersion->setText (m_updater->getLatestVersion (url));

    /* Set changelog text */
    ui->changelog->setText (m_updater->getChangelog (url));
    if (ui->changelog->toPlainText().isEmpty())
        ui->changelog->setText ("<p>No Changelog found...</p>");

    /* Enable/disable update button */
    ui->updateButton->setEnabled (m_updater->getUpdateAvailable (url));

    /* Show the window if update button is enabled */
    if (ui->updateButton->isEnabled())
        show();
}

void UpdaterWindow::onCheckFinished (const QString &url)
{
    /* There is an update available, show the window */
    if (m_updater->getUpdateAvailable (url) && (UPDATES_URL == url))
        showWindow (url);
}

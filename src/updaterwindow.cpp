/*********************************************************************************************
* Mozila License
* Just a meantime project to see the ability of qt, the framework that my OS might be based on
* And for those linux users that beleive in the power of notes
*********************************************************************************************/

#include "updaterwindow.h"
#include "ui_updaterwindow.h"

#include <QMessageBox>
#include <QDesktopServices>
#include <QSimpleUpdater.h>

/**
 * Indicates from where we should download the update definitions file
 */
const QString UPDATES_URL = "https://raw.githubusercontent.com/nuttyartist/notes/dev/UPDATES.json";

/**
 * Initializes the window components and configures the QSimpleUpdater
 */
UpdaterWindow::UpdaterWindow (QWidget *parent) : QWidget (parent)
{
    /* Initialize the UI */
    ui = new Ui::UpdaterWindow;
    ui->setupUi (this);

    /* Set window title to app name */
    setWindowTitle (qApp->applicationName() + " " + tr ("Updater"));

    /* Setup the color palettes */
#if defined Q_OS_WIN
    QPalette pal (palette());
    pal.setColor (QPalette::Background, QColor (248, 248, 248));
    setAutoFillBackground (true);
    setPalette (pal);
#endif

    /* Connect UI signals/slots */
    connect (ui->closeButton,  SIGNAL (clicked()), this, SLOT (close()));
    connect (ui->updateButton, SIGNAL (clicked()), this, SLOT (download()));

    /* Connect signals/slots of the QSU */
    m_updater = QSimpleUpdater::getInstance();
    connect (m_updater, SIGNAL (checkingFinished (QString)),
             this,      SLOT   (onCheckFinished  (QString)));
}

/**
 * Deletes the user interface
 */
UpdaterWindow::~UpdaterWindow()
{
    delete ui;
}

/**
 * Instructs the QSimpleUpdater to download and interpret the updater
 * definitions file
 */
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

/**
 * Initializes the download of the binary installer of the latest verion
 * of Notes
 */
void UpdaterWindow::download()
{
    if (m_updater->getUpdateAvailable (UPDATES_URL)) {
        ui->updateButton->setEnabled (false);
    }
}

/**
 * Resizes the dialog to the smallest possible size, without hiding any
 * user controls
 */
void UpdaterWindow::resizeToFit()
{
    /* Resize window */
    resize (minimumSize());
    setFixedSize (size());

    /* Set window flags */
    setWindowModality (Qt::ApplicationModal);
    setWindowFlags (Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
}

/**
 * Resets the state, text and information displayed the the UI controls
 * to indicate what is happening right now in the QSimpleUpdater
 */
void UpdaterWindow::resetControls()
{
    /* Hide the progress controls */
    ui->progressControls->hide();

    /* Reset the button states */
    ui->updateButton->setEnabled (false);

    /* Set installed version label */
    ui->installedVersion->setText (qApp->applicationVersion());

    /* Set available version label */
    if (m_updater->getUpdateAvailable (UPDATES_URL))
        ui->availableVersion->setText (m_updater->getLatestVersion (UPDATES_URL));
    else
        ui->availableVersion->setText ("--.--");

    /* Set title label */
    if (m_updater->getUpdateAvailable (UPDATES_URL)) {
        ui->title->setText (tr ("An newer version of %1 is available!")
                            .arg (qApp->applicationName()));
    } else {
        ui->title->setText (tr ("You are running the latest version of %1!")
                            .arg (qApp->applicationName()));
    }

    /* Set changelog text */
    ui->changelog->setText (m_updater->getChangelog (UPDATES_URL));
    if (ui->changelog->toPlainText().isEmpty())
        ui->changelog->setText ("<p>No changelog found...</p>");

    /* Enable/disable update button */
    ui->updateButton->setEnabled (m_updater->getUpdateAvailable (UPDATES_URL));

    /* Resize the dialog to its minimum size */
    resizeToFit();
}

/**
 * Updates the text displayed the the UI controls to reflect the information
 * obtained by the QSimpleUpdater and shows the dialog
 */
void UpdaterWindow::onUpdateAvailable()
{
    resetControls();
    showNormal();
}

/**
 * Displays a message box that informs the user that he/she is running the
 * latest version of the application
 */
void UpdaterWindow::onNoUpdateAvailable()
{
    /* Construct the message to display to the user */
    QString msg = tr ("Congratulations! You are running the latest "
                      "version of %1 (%2)!");

    /* Show a lovely message box */
    QMessageBox::information (this, windowTitle(),
                              msg.arg (qApp->applicationName(),
                                       qApp->applicationVersion()));
}

/**
 * Called when the \a QSimpleUpdater notifies us that it downloaded and
 * interpreted the update definitions file.
 *
 * This function decides whenever to show the dialog or just notify the user
 * that he/she is running the latest version of notes
 */
void UpdaterWindow::onCheckFinished (const QString &url)
{
    /* Ensure that the controls indicate what is actually happening */
    resetControls();

    /* There is an update available, show the window */
    if (m_updater->getUpdateAvailable (url) && (UPDATES_URL == url))
        onUpdateAvailable();

    /* There are no updates available */
    else
        onNoUpdateAvailable();


    show();
}

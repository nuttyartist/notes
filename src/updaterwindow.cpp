/*********************************************************************************************
* Mozila License
* Just a meantime project to see the ability of qt, the framework that my OS might be based on
* And for those linux users that beleive in the power of notes
*********************************************************************************************/

#include "updaterwindow.h"
#include "ui_updaterwindow.h"

#include <math.h>

#include <QDir>
#include <QMessageBox>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QSimpleUpdater.h>
#include <QNetworkConfiguration>
#include <QNetworkAccessManager>

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
    m_ui = new Ui::UpdaterWindow;
    m_ui->setupUi (this);

    /* Set window title to app name */
    setWindowTitle (qApp->applicationName() + " " + tr ("Updater"));

    /* Setup the color palettes */
#if defined Q_OS_WIN
    QPalette pal (palette());
    pal.setColor (QPalette::Background, QColor (248, 248, 248));
    setAutoFillBackground (true);
    setPalette (pal);
#endif

    /* Configure the network manager */
    m_manager = new QNetworkAccessManager (this);

    /* Connect UI signals/slots */
    connect (m_ui->closeButton,  SIGNAL (clicked()),
             this,               SLOT (close()));
    connect (m_ui->updateButton, SIGNAL (clicked()),
             this,               SLOT (onDownloadButtonClicked()));

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
    delete m_ui;
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
 * Resizes the dialog to the smallest possible size, without hiding any
 * user controls
 */
void UpdaterWindow::resizeToFit()
{
    /* Resize window */
    resize (minimumSizeHint());
    setFixedSize (minimumSizeHint());

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
    /* Reset the button states */
    m_ui->updateButton->setEnabled (false);

    /* Set installed version label */
    m_ui->installedVersion->setText (qApp->applicationVersion());

    /* Set available version label */
    if (m_updater->getUpdateAvailable (UPDATES_URL))
        m_ui->availableVersion->setText (m_updater->getLatestVersion (UPDATES_URL));
    else
        m_ui->availableVersion->setText (tr ("N/A"));

    /* Set title label */
    if (m_updater->getUpdateAvailable (UPDATES_URL)) {
        m_ui->title->setText (tr ("An newer version of %1 is available!")
                              .arg (qApp->applicationName()));
    } else {
        m_ui->title->setText (tr ("You are running the latest version of %1!")
                              .arg (qApp->applicationName()));
    }

    /* Reset the progress controls */
    m_ui->progressControls->hide();
    m_ui->progressBar->setValue (0);
    m_ui->downloadLabel->setText (tr ("Downloading Updates") + "...");
    m_ui->timeLabel->setText (tr ("Time remaining") + ": " + tr ("unknown"));

    /* Set changelog text */
    m_ui->changelog->setText (m_updater->getChangelog (UPDATES_URL));
    if (m_ui->changelog->toPlainText().isEmpty())
        m_ui->changelog->setText ("<p>No changelog found...</p>");

    /* Enable/disable update button */
    bool validURL = !m_updater->getOpenUrl (UPDATES_URL).isEmpty() ||
            !m_updater->getDownloadUrl (UPDATES_URL).isEmpty();
    bool available= m_updater->getUpdateAvailable (UPDATES_URL);
    m_ui->updateButton->setEnabled (available && validURL);

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

void UpdaterWindow::onDownloadFinished()
{
    /* Read the downloaded data */
    QByteArray data = m_reply->readAll();

    /* Data is invalid, abort */
    if (data.isEmpty()) {
        QMessageBox::critical (this,
                               tr ("Download Error"),
                               tr ("Received an empty file!"));
        return;
    }

    /* Check if we need to redirect */
    QUrl url = m_reply->attribute (QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!url.isEmpty()) {
        startDownload (url);
        return;
    }

    /* Prepare downloads directory for writing */
    QDir dir (QDir::homePath() + "/Downloads/");
    if (!dir.exists())
        dir.mkpath (".");

    /* Save downloaded data to disk */
    QString name = m_reply->url().toString().split ("/").last();
    QFile file (dir.path() + name);
    if (file.open (QIODevice::WriteOnly)) {
        file.write (data);
        file.close();
        openDownload (file.fileName());
    }
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
 * Initializes the download of the update and disables the 'update' button
 */
void UpdaterWindow::onDownloadButtonClicked()
{
    if (m_updater->getUpdateAvailable (UPDATES_URL)) {
        m_ui->updateButton->setEnabled (false);
        startDownload (m_updater->getDownloadUrl (UPDATES_URL));
    }
}

/**
 * Initializes the download of the update and configures the connections
 * to automatically update the user interface when we receive new data
 * from the download/update server
 */
void UpdaterWindow::startDownload (const QUrl& url)
{
    /* URL is invalid, try opening web browser */
    if (url.isEmpty()) {
        QDesktopServices::openUrl (QUrl (m_updater->getOpenUrl (UPDATES_URL)));
        return;
    }

    /* Start download */
    m_startTime = QDateTime::currentDateTime().toTime_t();
    m_reply = m_manager->get (QNetworkRequest (url));

    /* Show UI controls */
    m_ui->progressControls->show();
    resizeToFit();
    showNormal();

    /* Update UI when download progress changes or download finishes */
    connect (m_reply, SIGNAL (downloadProgress (qint64, qint64)),
             this,      SLOT (updateProgress   (qint64, qint64)));
    connect (m_reply, SIGNAL (redirected       (QUrl)),
             this,      SLOT (startDownload    (QUrl)));
    connect (m_reply, SIGNAL (finished()),
             this,      SLOT (onDownloadFinished()));
}

/**
 * Opens the downloaded file
 */
void UpdaterWindow::openDownload (const QString& path)
{
    if (!path.isEmpty()) {
        QDesktopServices::openUrl (QUrl ("file:///" + path));
        m_ui->downloadLabel->setText (tr ("Download finished!"));
        m_ui->timeLabel->setText (tr ("Opening downloaded file") + "...");
    }
}

/**
 * Called when the \a QSimpleUpdater notifies us that it downloaded and
 * interpreted the update definitions file.
 *
 * This function decides whenever to show the dialog or just notify the user
 * that he/she is running the latest version of notes
 */
void UpdaterWindow::onCheckFinished (const QString &url) {
    /* There is an update available, show the window */
    if (m_updater->getUpdateAvailable (url) && (UPDATES_URL == url))
        onUpdateAvailable();

    /* There are no updates available */
    else
        onNoUpdateAvailable();
}

/**
 * Calculates the appropiate size units (bytes, KB or MB) for the received
 * data and the total download size. Then, this function proceeds to update the
 * dialog controls/UI.
 */
void UpdaterWindow::calculateSizes (qint64 received, qint64 total)
{
    QString totalSize;
    QString receivedSize;

    if (total < 1024)
        totalSize = tr ("%1 bytes").arg (total);

    else if (total < 1048576)
        totalSize = tr ("%1 KB").arg (round (total / 1024));

    else
        totalSize = tr ("%1 MB").arg (round (total / 1048576));

    if (received < 1024)
        receivedSize = tr ("%1 bytes").arg (received);

    else if (received < 1048576)
        receivedSize = tr ("%1 KB").arg (received / 1024);

    else
        receivedSize = tr ("%1 MB").arg (received / 1048576);

    m_ui->downloadLabel->setText (tr ("Downloading updates")
                                  + " (" + receivedSize + " " + tr ("of")
                                  + " " + totalSize + ")");
}

/**
 * Uses the \a received and \a total parameters to get the download progress
 * and update the progressbar value on the dialog.
 */
void UpdaterWindow::updateProgress (qint64 received, qint64 total)
{
    if (total > 0) {
        m_ui->progressBar->setMinimum (0);
        m_ui->progressBar->setMaximum (100);
        m_ui->progressBar->setValue ((received * 100) / total);

        calculateSizes (received, total);
        calculateTimeRemaining (received, total);
    }

    else {
        m_ui->progressBar->setMinimum (0);
        m_ui->progressBar->setMaximum (0);
        m_ui->progressBar->setValue (-1);
        m_ui->downloadLabel->setText (tr ("Downloading Updates") + "...");
        m_ui->timeLabel->setText (QString ("%1: %2")
                                  .arg (tr ("Time Remaining"))
                                  .arg (tr ("Unknown")));
    }
}

/**
 * Uses two time samples (from the current time and a previous sample) to
 * calculate how many bytes have been downloaded.
 *
 * Then, this function proceeds to calculate the appropiate units of time
 * (hours, minutes or seconds) and constructs a user-friendly string, which
 * is displayed in the dialog.
 */
void UpdaterWindow::calculateTimeRemaining (qint64 received, qint64 total)
{
    uint difference = QDateTime::currentDateTime().toTime_t() - m_startTime;

    if (difference > 0) {
        QString timeString;
        qreal timeRemaining = total / (received / difference);

        if (timeRemaining > 7200) {
            timeRemaining /= 3600;
            timeString = tr ("About %1 hours").arg (int (timeRemaining + 0.5));
        }

        else if (timeRemaining > 60) {
            timeRemaining /= 60;
            timeString = tr ("About %1 minutes").arg (int (timeRemaining + 0.5));
        }

        else if (timeRemaining <= 60)
            timeString = tr ("%1 seconds").arg (int (timeRemaining + 0.5));

        m_ui->timeLabel->setText (tr ("Time remaining") + ": " + timeString);
    }
}

/**
 * Rounds the given \a input to two decimal places
 */
qreal UpdaterWindow::round (const qreal& input) {
    return roundf (input * 100) / 100;
}

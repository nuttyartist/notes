/*********************************************************************************************
* Mozila License
* Just a meantime project to see the ability of qt, the framework that my OS might be based on
* And for those linux users that beleive in the power of notes
*********************************************************************************************/

#include "updaterwindow.h"
#include "ui_updaterwindow.h"

#include <math.h>

#include <QDir>
#include <QTimer>
#include <QMessageBox>
#include <QMouseEvent>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QSimpleUpdater.h>
#include <QNetworkConfiguration>
#include <QNetworkAccessManager>

/**
 * Indicates from where we should download the update definitions file
 */
static const QString UPDATES_URL ("https://raw.githubusercontent.com/"
                                  "nuttyartist/notes/automatic-updates-windows"
                                  "/UPDATES.json");

/**
 * Extension appended to partial downloads
 */
static const QString PARTIAL_DOWN (".part");

/**
 * Download dir
 */
static const QDir DOWNLOAD_DIR (QDir::homePath() + "/Downloads/");

/**
 * Initializes the window components and configures the QSimpleUpdater
 */
UpdaterWindow::UpdaterWindow (QWidget *parent) : QWidget (parent),
    m_fileName (""),
    m_ui (new Ui::UpdaterWindow),
    m_silent (false),
    m_canMoveWindow (false),
    m_checkingForUpdates (false),
    m_reply (Q_NULLPTR),
    m_updater (QSimpleUpdater::getInstance()),
    m_manager (new QNetworkAccessManager (this))
{
    /* Load the stylesheet */
    QFile file (":/styles/style.css");
    if (file.open (QIODevice::ReadOnly)) {
        setStyleSheet (QString::fromUtf8 (file.readAll()).toLatin1());
        file.close();
    }

    /* Initialize the UI */
    m_ui->setupUi (this);
    setWindowTitle (qApp->applicationName() + " " + tr ("Updater"));

    /* Change fonts */
    setFont (QFont ("Roboto"));
    m_ui->changelog->setFont (QFont ("Arimo"));

    /* Connect UI signals/slots */
    connect (m_ui->closeButton,  SIGNAL (clicked()),
             this,                 SLOT (close()));
    connect (m_ui->updateButton, SIGNAL (clicked()),
             this,                 SLOT (onDownloadButtonClicked()));
    connect (m_updater,          SIGNAL (checkingFinished (QString)),
             this,                 SLOT (onCheckFinished  (QString)));

    /* Start the UI loops */
    updateTitleLabel();

    /* Remove window border */
#if defined Q_OS_WIN
    setWindowFlags (Qt::CustomizeWindowHint);
#elif defined Q_OS_MAC || defined Q_OS_LINUX
    setWindowFlags (Qt::Window | Qt::FramelessWindowHint);
#else
#error "We don't support your OS yet..."
#endif

    /* Set window flags */
    setWindowModality (Qt::ApplicationModal);
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
void UpdaterWindow::checkForUpdates (bool silent)
{
    /* Change the silent flag */
    m_silent = silent;
    m_checkingForUpdates = true;

    /* Set module properties */
    m_updater->setNotifyOnFinish (UPDATES_URL, false);
    m_updater->setNotifyOnUpdate (UPDATES_URL, false);
    m_updater->setDownloaderEnabled (UPDATES_URL, false);
    m_updater->setUseCustomInstallProcedures (UPDATES_URL, true);
    m_updater->setModuleVersion (UPDATES_URL, qApp->applicationVersion());

    /* Check for updates */
    m_updater->checkForUpdates (UPDATES_URL);

    /* Show window if silent flag is not set */
    if (!m_silent) {
        m_ui->updateButton->setEnabled (false);
        m_ui->title->setText (tr ("Checking for updates...."));

        resetControls();
        showNormal();
    }
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
    m_ui->availableVersion->setText (m_updater->getLatestVersion (UPDATES_URL));

    /* Set title label */
    if (m_updater->getUpdateAvailable (UPDATES_URL))
        m_ui->title->setText (tr ("A Newer Version is Available!"));
    else
        m_ui->title->setText (tr ("You're up-to-date!"));

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
    bool available = m_updater->getUpdateAvailable (UPDATES_URL);
    bool validOpenUrl = !m_updater->getOpenUrl (UPDATES_URL).isEmpty();
    bool validDownUrl = !m_updater->getDownloadUrl (UPDATES_URL).isEmpty();
    m_ui->updateButton->setEnabled (available && (validOpenUrl || validDownUrl));

    /* Resize window */
    bool showAgain = isVisible(); {
        int height = minimumSizeHint().height();
        int width = qMax (minimumSizeHint().width(), (int) (height * 1.2));

        resize (QSize (width, height));
    }

    /* Re-show the window (if required) */
    if (showAgain)
        showNormal();
}

/**
 * Changes the number of dots of the title label while the QSimpleUpdater
 * is downloading and interpreting the update definitions file
 */
void UpdaterWindow::updateTitleLabel()
{
    if (m_checkingForUpdates) {
        QString base = tr ("Checking for updates");

        if (m_ui->title->text().endsWith ("...."))
            m_ui->title->setText (base + "." );

        else if (m_ui->title->text().endsWith ("..."))
            m_ui->title->setText (base + "....");

        else if (m_ui->title->text().endsWith (".."))
            m_ui->title->setText (base + "...");

        else if (m_ui->title->text().endsWith ("."))
            m_ui->title->setText (base + "..");

        else
            m_ui->title->setText (base + ".");
    }

    QTimer::singleShot (500, this, SLOT (updateTitleLabel()));
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

    /* Cancel previous download (if any) */
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
    }

    /* Start download */
    m_startTime = QDateTime::currentDateTime().toTime_t();
    m_reply = m_manager->get (QNetworkRequest (url));

    /* Set file name */
    m_fileName = m_updater->getDownloadUrl (UPDATES_URL).split ("/").last();
    if (m_fileName.isEmpty()) {
        m_fileName = QString ("%1 Update %2.bin")
                .arg (qApp->applicationName())
                .arg (m_updater->getLatestVersion (UPDATES_URL));
    }

    /* Prepare download directory */
    if (!DOWNLOAD_DIR.exists())
        DOWNLOAD_DIR.mkpath (".");

    /* Remove previous downloads (if any) */
    QFile::remove (DOWNLOAD_DIR.filePath (m_fileName));
    QFile::remove (DOWNLOAD_DIR.filePath (m_fileName + PARTIAL_DOWN));

    /* Show UI controls */
    m_ui->progressControls->show();
    showNormal();

    /* Update UI when download progress changes or download finishes */
    connect (m_reply, SIGNAL (downloadProgress (qint64, qint64)),
             this,      SLOT (updateProgress   (qint64, qint64)));
    connect (m_reply, SIGNAL (redirected       (QUrl)),
             this,      SLOT (startDownload    (QUrl)));
}

/**
 * Opens the downloaded file
 */
void UpdaterWindow::openDownload (const QString& file)
{
    /* File is empty, abort */
    if (file.isEmpty())
        return;

    /* This a redirection request, delete temp file */
    QUrl url = m_reply->attribute (QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!url.isEmpty()) {
        startDownload (url);
        QFile::remove (file);
        return;
    }

    /* Change labels */
    m_ui->downloadLabel->setText (tr ("Download finished!"));
    m_ui->timeLabel->setText (tr ("Opening downloaded file") + "...");

    /* Remove the .part extension */
    QString new_file = file;
    new_file = new_file.remove (PARTIAL_DOWN, Qt::CaseInsensitive);
    QFile::rename (file, new_file);

    /* Open the downloaded file */
    QDesktopServices::openUrl (QUrl::fromLocalFile (new_file));
}

/**
 * Called when the \a QSimpleUpdater notifies us that it downloaded and
 * interpreted the update definitions file.
 *
 * This function decides whenever to show the dialog or just notify the user
 * that he/she is running the latest version of notes
 */
void UpdaterWindow::onCheckFinished (const QString &url) {
    /* Do not allow the title label to change automatically */
    m_checkingForUpdates = false;

    /* There is an update available, show the window */
    if (m_updater->getUpdateAvailable (url) && (UPDATES_URL == url))
        onUpdateAvailable();

    /* There are no updates available */
    else if (!m_silent) {
        resetControls();
        showNormal();
    }
}

/**
 * Saves the downloaded file to the disk
 */
void UpdaterWindow::saveFile (qint64 received, qint64 total) {
    /* Save downloaded data to disk */
    QFile file (DOWNLOAD_DIR.filePath (m_fileName + PARTIAL_DOWN));
    if (file.open (QIODevice::WriteOnly | QIODevice::Append)) {
        file.write (m_reply->readAll());
        file.close();
    }

    /* Open the download */
    if (received >= total && total > 0)
        openDownload (file.fileName());
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
        saveFile (received, total);
    }

    else {
        m_ui->progressBar->setMinimum (0);
        m_ui->progressBar->setMaximum (0);
        m_ui->progressBar->setValue (-1);
        m_ui->downloadLabel->setText (tr ("Downloading Updates") + "...");
        m_ui->timeLabel->setText (QString ("%1: %2")
                                  .arg (tr ("Time Remaining"))
                                  .arg (tr ("unknown")));
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
            int hours = int (timeRemaining + 0.5);

            if (hours > 1)
                timeString = tr ("about %1 hours").arg (hours);
            else
                timeString = tr ("about one hour");
        }

        else if (timeRemaining > 60) {
            timeRemaining /= 60;
            int minutes = int (timeRemaining + 0.5);

            if (minutes > 1)
                timeString = tr ("%1 minutes").arg (minutes);
            else
                timeString = tr ("1 minute");
        }

        else if (timeRemaining <= 60) {
            int seconds = int (timeRemaining + 0.5);

            if (seconds > 1)
                timeString = tr ("%1 seconds").arg (seconds);
            else
                timeString = tr ("1 second");
        }

        m_ui->timeLabel->setText (tr ("Time remaining") + ": " + timeString);
    }
}

/**
 * Allows the user to move the window and registers the position in which
 * the user originally clicked to move the window
 */
void UpdaterWindow::mousePressEvent (QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (event->pos().x() < width() - 5
                && event->pos().x() >5
                && event->pos().y() < height() - 5
                && event->pos().y() > 5) {
            m_canMoveWindow = true;
            m_mousePressX = event->x();
            m_mousePressY = event->y();
        }
    }

    event->accept();
}

/**
 * Changes the cursor icon to a hand (to hint the user that he/she is dragging
 * the window) and moves the window to the desired position of the given
 * \a event
 */
void UpdaterWindow::mouseMoveEvent (QMouseEvent* event)
{
    if (m_canMoveWindow) {
        setCursor (Qt::ClosedHandCursor);
        int dx = event->globalX() - m_mousePressX;
        int dy = event->globalY() - m_mousePressY;
        move (dx, dy);
    }
}

/**
 * Disallows the user to move the window and resets the window cursor
 */
void UpdaterWindow::mouseReleaseEvent (QMouseEvent *event) {
    m_canMoveWindow = false;
    unsetCursor();
    event->accept();
}

/**
 * Rounds the given \a input to two decimal places
 */
qreal UpdaterWindow::round (const qreal& input) {
    return roundf (input * 100) / 100;
}

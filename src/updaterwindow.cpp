/*********************************************************************************************
 * Mozila License
 * Just a meantime project to see the ability of qt, the framework that my OS might be based on
 * And for those linux users that beleive in the power of notes
 *********************************************************************************************/

#include "updaterwindow.h"
#include "ui_updaterwindow.h"

#include <math.h>

#include <QTimer>
#include <QMessageBox>
#include <QMouseEvent>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QWindow>

#include <QSimpleUpdater.h>

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
#  define UseXdgOpen
#else
#  ifdef UseXdgOpen
#    undef UseXdgOpen
#  endif
#endif

#ifdef UseXdgOpen
#  include <QProcess>
static QProcess XDGOPEN_PROCESS;
#endif

/**
 * Indicates from where we should download the update definitions file
 */
static const QString
        UPDATES_URL("https://raw.githubusercontent.com/nuttyartist/notes/master/UPDATES.json");

/**
 * Initializes the window components and configures the QSimpleUpdater
 */
UpdaterWindow::UpdaterWindow(QWidget *parent)
    : QDialog(parent),
      m_downloadDir(QDir::homePath() + "/Downloads/"),
      m_ui(new Ui::UpdaterWindow),
      m_canMoveWindow(false),
      m_checkingForUpdates(false),
      m_dontShowUpdateWindow(false),
      m_forced(false),
      m_reply(nullptr),
      m_updater(QSimpleUpdater::getInstance()),
      m_manager(new QNetworkAccessManager(this))
{
    /* Load the stylesheet */
    QFile file(":/styles/style.css");
    if (file.open(QIODevice::ReadOnly)) {
        setStyleSheet(QString::fromUtf8(file.readAll()).toLatin1());
        file.close();
    }

    /* Initialize the UI */
    m_ui->setupUi(this);
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
#endif
    setWindowTitle(qApp->applicationName() + " " + tr("Updater"));

    /* Change fonts */
#ifdef __APPLE__
    QFont fontToUse = QFont(QStringLiteral("SF Pro Text")).exactMatch()
            ? QStringLiteral("SF Pro Text")
            : QStringLiteral("Roboto");
#elif _WIN32
    QFont fontToUse = QFont(QStringLiteral("Segoe UI")).exactMatch() ? QStringLiteral("Segoe UI")
                                                                     : QStringLiteral("Roboto");
#else
    QFont fontToUse = QFont(QStringLiteral("Roboto"));
#endif

    setFont(fontToUse);
    m_ui->changelog->setFont(fontToUse);
    m_ui->changelog->setTextColor(QColor(26, 26, 26));
    foreach (QWidget *widgetChild, findChildren<QWidget *>()) {
        widgetChild->setFont(fontToUse);
    }

    /* Connect UI signals/slots */
    connect(m_ui->closeButton, &QPushButton::clicked, this, &UpdaterWindow::close);
    connect(m_ui->updateButton, &QPushButton::clicked, this,
            &UpdaterWindow::onDownloadButtonClicked);
    connect(m_updater, &QSimpleUpdater::checkingFinished, this, &UpdaterWindow::onCheckFinished);
    connect(m_ui->checkBox, &QCheckBox::toggled, this, &UpdaterWindow::dontShowUpdateWindowChanged);

    /* Start the UI loops */
    updateTitleLabel();

    /* Remove native window decorations */
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    setWindowFlags(windowFlags() | Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
#else
    setWindowFlags(windowFlags() | Qt::CustomizeWindowHint);
#endif

    /* React when xdg-open finishes (Linux only) */
#ifdef UseXdgOpen
    connect(&XDGOPEN_PROCESS, SIGNAL(finished(int)), this, SLOT(onXdgOpenFinished(int)));
#endif

    /* Set window flags */
    setWindowModality(Qt::ApplicationModal);
}

/**
 * Deletes the user interface
 */
UpdaterWindow::~UpdaterWindow()
{
    /* Ensure that xdg-open process is closed */
#ifdef UseXdgOpen
    if (XDGOPEN_PROCESS.isOpen())
        XDGOPEN_PROCESS.close();
#endif

    /* Delete UI controls */
    delete m_ui;
}

void UpdaterWindow::setShowWindowDisable(const bool dontShowWindow)
{
    m_dontShowUpdateWindow = dontShowWindow;
    m_ui->checkBox->setChecked(m_dontShowUpdateWindow);
}

/**
 * Instructs the QSimpleUpdater to download and interpret the updater
 * definitions file
 */
void UpdaterWindow::checkForUpdates(bool force)
{
    /* Change the silent flag */
    if (!m_updater->getUpdateAvailable(UPDATES_URL)) {
        m_checkingForUpdates = true;

        m_forced = force;

        /* Set module properties */
        m_updater->setNotifyOnFinish(UPDATES_URL, false);
        m_updater->setNotifyOnUpdate(UPDATES_URL, false);
        m_updater->setDownloaderEnabled(UPDATES_URL, false);
        m_updater->setUseCustomInstallProcedures(UPDATES_URL, true);
        m_updater->setModuleVersion(UPDATES_URL, qApp->applicationVersion());

        /* Check for updates */
        m_updater->checkForUpdates(UPDATES_URL);
    }

    /* Show window if force flag is set */
    if (force) {
        if (!m_updater->getUpdateAvailable(UPDATES_URL)) {
            m_ui->updateButton->setEnabled(false);
            m_ui->title->setText(tr("Checking for updates...."));
        }

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
    m_ui->updateButton->setEnabled(false);

    /* Set installed version label */
    m_ui->installedVersion->setText(qApp->applicationVersion());

    /* Set available version label */
    m_ui->availableVersion->setText(m_updater->getLatestVersion(UPDATES_URL));

    /* Set title label */
    if (m_updater->getUpdateAvailable(UPDATES_URL)) {
        m_ui->title->setText(tr("A Newer Version is Available!"));
    } else {
        m_ui->title->setText(tr("You're up-to-date!"));
    }

    /* Reset the progress controls */
    m_ui->progressControls->hide();
    m_ui->progressBar->setValue(0);
    m_ui->downloadLabel->setText(tr("Downloading Updates") + "...");
    m_ui->timeLabel->setText(tr("Time remaining") + ": " + tr("unknown"));

    /* Set changelog text */
    QString changelogText = m_updater->getChangelog(UPDATES_URL);
    m_ui->changelog->setText(changelogText);
    if (m_ui->changelog->toPlainText().isEmpty()) {
        m_ui->changelog->setText("<p>No changelog found...</p>");
    } else {
        m_ui->changelog->setText(changelogText.append(
                "\n")); // Don't know why currently changelog box is dissapearing at the bottom, so
                        // I add a new line to see the text.
    }

    /* Enable/disable update button */
    bool available = m_updater->getUpdateAvailable(UPDATES_URL);
    bool validOpenUrl = !m_updater->getOpenUrl(UPDATES_URL).isEmpty();
    bool validDownUrl = !m_updater->getDownloadUrl(UPDATES_URL).isEmpty();
    m_ui->updateButton->setEnabled(available && (validOpenUrl || validDownUrl));

    /* Resize window */
    bool showAgain = isVisible();
    int height = minimumSizeHint().height();
    int width = qMax(minimumSizeHint().width(), int(height * 1.2));
    resize(QSize(width, height));

    /* Re-show the window(if required)*/
    if (showAgain) {
        showNormal();
    }
}

/**
 * Changes the number of dots of the title label while the QSimpleUpdater
 * is downloading and interpreting the update definitions file
 */
void UpdaterWindow::updateTitleLabel()
{
    if (m_checkingForUpdates) {
        static int num = 0;
        QString base = tr("Checking for updates");
        QString dot = "";
        num++;
        dot.fill('.', num);
        m_ui->title->setText(base + dot);
        if (num == 4)
            num = 0;
    }
    QTimer::singleShot(500, this, SLOT(updateTitleLabel()));
}

/**
 * Updates the text displayed the the UI controls to reflect the information
 * obtained by the QSimpleUpdater and shows the dialog
 */
void UpdaterWindow::onUpdateAvailable()
{
    if (!m_dontShowUpdateWindow) {
        resetControls();
        showNormal();
    }
}

/**
 * Initializes the download of the update and disables the 'update' button
 */
void UpdaterWindow::onDownloadButtonClicked()
{
    if (m_updater->getUpdateAvailable(UPDATES_URL)) {
        m_ui->updateButton->setEnabled(false);
        startDownload(m_updater->getDownloadUrl(UPDATES_URL));
    }
}

/**
 * Initializes the download of the update and configures the connections
 * to automatically update the user interface when we receive new data
 * from the download/update server
 */
void UpdaterWindow::startDownload(const QUrl &url)
{
    /* URL is invalid, try opening web browser */
    if (url.isEmpty()) {
        QDesktopServices::openUrl(QUrl(m_updater->getOpenUrl(UPDATES_URL)));
        return;
    }

    /* Cancel previous download (if any)*/
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
    }

    /* Start download */
    m_startTime = QDateTime::currentDateTime().toSecsSinceEpoch();
    QNetworkRequest netReq(url);

    netReq.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                        QNetworkRequest::NoLessSafeRedirectPolicy);
    m_reply = m_manager->get(netReq);

    /* Set file name */
    m_fileName = m_updater->getDownloadUrl(UPDATES_URL).split("/").last();
    if (m_fileName.isEmpty()) {
        m_fileName = QString("%1_Update_%2.bin")
                             .arg(QCoreApplication::applicationName(),
                                  m_updater->getLatestVersion(UPDATES_URL));
    }

    /* Prepare download directory */
    if (!m_downloadDir.exists())
        m_downloadDir.mkpath(".");

    /* Remove previous downloads(if any)*/
    QFile::remove(m_downloadDir.filePath(m_fileName));

    /* Show UI controls */
    m_ui->progressControls->show();
    showNormal();

    /* Update UI when download progress changes or download finishes */
    connect(m_reply, &QNetworkReply::downloadProgress, this, &UpdaterWindow::updateProgress);
    connect(m_reply, &QNetworkReply::finished, this, &UpdaterWindow::onDownloadFinished);
}

/**
 * Opens the downloaded file
 */
void UpdaterWindow::openDownload(const QString &file)
{
    /* File is empty, abort */
    if (file.isEmpty()) {
        return;
    }

    /* Change labels */
    m_ui->downloadLabel->setText(tr("Download finished!"));
    m_ui->timeLabel->setText(tr("Opening downloaded file") + "...");

    /* Try to open the downloaded file (Windows & Mac) */
#ifndef UseXdgOpen
    bool openUrl = QDesktopServices::openUrl(QUrl::fromLocalFile(file));
    if (!openUrl) {
        openDownloadFolder(file);
        m_ui->progressControls->hide();
        qApp->processEvents();
        showNormal();
        close();
    } else {
        qApp->quit();
    }
#endif

    /* On Linux, use xdg-open to know if the file was handled correctly */
#ifdef UseXdgOpen
    XDGOPEN_PROCESS.start("xdg-open", QStringList() << file);
#endif
}

/**
 * Called when the \a QSimpleUpdater notifies us that it downloaded and
 * interpreted the update definitions file.
 *
 * This function decides whenever to show the dialog or just notify the user
 * that he/she is running the latest version of notes
 */
void UpdaterWindow::onCheckFinished(const QString &url)
{
    Q_UNUSED(url)

    /* Do not allow the title label to change automatically */
    m_checkingForUpdates = false;

    /* There is an update available, show the window */
    if (m_updater->getUpdateAvailable(url) && (UPDATES_URL == url)) {
        onUpdateAvailable();
    } else if (m_forced) {
        m_forced = false;
        resetControls();
        showNormal();
    }
}

/**
 * Called when \c xdg-open finishes with the given \a exitCode.
 * If \a exitCode is not 0, then we shall try to open the folder in which
 * the update file was saved
 */
void UpdaterWindow::onXdgOpenFinished(const int exitCode)
{
#ifdef UseXdgOpen
    const auto arguments = XDGOPEN_PROCESS.arguments();
    if (exitCode != 0 && !arguments.isEmpty()) {
        openDownloadFolder(arguments.first());
    } else {
        QCoreApplication::quit();
    }
#else
    Q_UNUSED(exitCode);
#endif
}

/**
 * Notifies the user that there's been an error opening the downloaded
 * file directly and instructs the operating system to open the folder
 * in which the \a file is located
 */
void UpdaterWindow::openDownloadFolder(const QString &file)
{
    /* Notify the user of the problem */
    QString extension = file.split(".").last();
    QMessageBox::information(this, tr("Open Error"),
                             tr("It seems that your OS does not have an "
                                "application that can handle *.%1 files. "
                                "We'll open the downloads folder for you.")
                                     .arg(extension),
                             QMessageBox::Ok);

    /* Get the full path list of the downloaded file */
    QString native_path = QDir::toNativeSeparators(QDir(file).absolutePath());
    QStringList directories = native_path.split(QDir::separator());

    /* Remove file name from list to get the folder of the update file */
    directories.removeLast();
    QString path = directories.join(QDir::separator());

    /* Get valid URL and open it */
    QUrl url = QUrl::fromLocalFile(QDir(path).absolutePath());
    QDesktopServices::openUrl(url);
}

/**
 * Calculates the appropiate size units(bytes, KB or MB)for the received
 * data and the total download size. Then, this function proceeds to update the
 * dialog controls/UI.
 */
void UpdaterWindow::calculateSizes(qint64 received, qint64 total)
{
    QString totalSize;
    QString receivedSize;

    /* Get total size string */
    if (total < 1024) {
        totalSize = tr("%1 bytes").arg(total);
    } else if (total < (1024 * 1024)) {
        totalSize = tr("%1 KB").arg(round(total / 1024));
    } else {
        totalSize = tr("%1 MB").arg(round(total / (1024 * 1024)));
    }

    /* Get received size string */
    if (received < 1024) {
        receivedSize = tr("%1 bytes").arg(received);
    } else if (received < (1024 * 1024)) {
        receivedSize = tr("%1 KB").arg(received / 1024);
    } else {
        receivedSize = tr("%1 MB").arg(received / (1024 * 1024));
    }

    /* Update the label text */
    m_ui->downloadLabel->setText(tr("Downloading updates") + " (" + receivedSize + " " + tr("of")
                                 + " " + totalSize + ")");
}

/**
 * Uses the \a received and \a total parameters to get the download progress
 * and update the progressbar value on the dialog.
 */
void UpdaterWindow::updateProgress(qint64 received, qint64 total)
{
    if (total > 0) {
        m_ui->progressBar->setMinimum(0);
        m_ui->progressBar->setMaximum(100);
        m_ui->progressBar->setValue(int((received * 100) / total));

        calculateSizes(received, total);
        calculateTimeRemaining(received, total);
    } else {
        m_ui->progressBar->setMinimum(0);
        m_ui->progressBar->setMaximum(0);
        m_ui->progressBar->setValue(-1);
        m_ui->downloadLabel->setText(tr("Downloading Updates") + "...");
        m_ui->timeLabel->setText(QStringLiteral("%1: %2").arg(tr("Time Remaining"), tr("unknown")));
    }
}

/**
 * Uses two time samples(from the current time and a previous sample)to
 * calculate how many bytes have been downloaded.
 *
 * Then, this function proceeds to calculate the appropiate units of time
 *(hours, minutes or seconds)and constructs a user-friendly string, which
 * is displayed in the dialog.
 */
void UpdaterWindow::calculateTimeRemaining(qint64 received, qint64 total)
{
    uint difference = QDateTime::currentDateTime().toSecsSinceEpoch() - m_startTime;

    if (difference > 0) {
        QString timeString;
        qreal timeRemaining = total / (received / difference);

        if (timeRemaining > 7200) {
            timeRemaining /= 3600;
            int hours = int(timeRemaining + 0.5);

            if (hours > 1) {
                timeString = tr("about %1 hours").arg(hours);
            } else {
                timeString = tr("about one hour");
            }
        } else if (timeRemaining > 60) {
            timeRemaining /= 60;
            int minutes = int(timeRemaining + 0.5);

            if (minutes > 1) {
                timeString = tr("%1 minutes").arg(minutes);
            } else {
                timeString = tr("1 minute");
            }
        } else if (timeRemaining <= 60) {
            int seconds = int(timeRemaining + 0.5);

            if (seconds > 1) {
                timeString = tr("%1 seconds").arg(seconds);
            } else {
                timeString = tr("1 second");
            }
        }

        m_ui->timeLabel->setText(tr("Time remaining") + ": " + timeString);
    }
}

void UpdaterWindow::onDownloadFinished()
{
    QString redirectedUrl =
            m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (redirectedUrl.isEmpty()) {
        const QString filePath = m_downloadDir.filePath(m_fileName);
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
            file.write(m_reply->readAll());
            file.close();
            qApp->processEvents();
        }

        openDownload(filePath);
    } else {
        startDownload(redirectedUrl);
    }
}

/**
 * Allows the user to move the window and registers the position in which
 * the user originally clicked to move the window
 */
void UpdaterWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (event->x() < width() - 5 && event->x() > 5 && event->pos().y() < height() - 5
            && event->pos().y() > 5) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            m_canMoveWindow = !window()->windowHandle()->startSystemMove();
#else
            m_canMoveWindow = true;
#endif
            m_mousePressX = event->pos().x();
            m_mousePressY = event->pos().y();
        }
    }
    event->accept();
}

/**
 * Changes the cursor icon to a hand(to hint the user that he/she is dragging
 * the window)and moves the window to the desired position of the given
 * \a event
 */
void UpdaterWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_canMoveWindow) {
        int dx = event->globalX() - m_mousePressX;
        int dy = event->globalY() - m_mousePressY;
        move(dx, dy);
    }
}

/**
 * Disallows the user to move the window and resets the window cursor
 */
void UpdaterWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_canMoveWindow = false;
    unsetCursor();
    event->accept();
}

/**
 * Rounds the given \a input to two decimal places
 */
qreal UpdaterWindow::round(qreal input)
{
    return qreal(roundf(float(input * 100)) / 100);
}

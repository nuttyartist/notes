/*
 * Copyright (c) 2014-2016 Alex Spataru <alex_spataru@outlook.com>
 *
 * This file is part of the QSimpleUpdater library, which is released under
 * the DBAD license, you can read a copy of it below:
 *
 * DON'T BE A DICK PUBLIC LICENSE TERMS AND CONDITIONS FOR COPYING,
 * DISTRIBUTION AND MODIFICATION:
 *
 * Do whatever you like with the original work, just don't be a dick.
 * Being a dick includes - but is not limited to - the following instances:
 *
 * 1a. Outright copyright infringement - Don't just copy this and change the
 *     name.
 * 1b. Selling the unmodified original with no work done what-so-ever, that's
 *     REALLY being a dick.
 * 1c. Modifying the original work to contain hidden harmful content.
 *     That would make you a PROPER dick.
 *
 * If you become rich through modifications, related works/services, or
 * supporting the original work, share the love.
 * Only a dick would make loads off this work and not buy the original works
 * creator(s) a pint.
 *
 * Code is provided with no warranty. Using somebody else's code and bitching
 * when it goes wrong makes you a DONKEY dick.
 * Fix the problem yourself. A non-dick would submit the fix back.
 */

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QNetworkAccessManager>

#include <math.h>

#include "Downloader.h"

Downloader::Downloader (QWidget* parent) : QWidget (parent) {
    m_ui = new Ui::Downloader;
    m_ui->setupUi (this);

    /* Initialize private members */
    m_manager = new QNetworkAccessManager();

    /* Initialize internal values */
    m_filePath = "";
    m_startTime = 0;
    m_useCustomProcedures = false;

    /* Make the window look like a modal dialog */
    setWindowIcon (QIcon ());
    setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    /* Configure the appearance and behavior of the buttons */
    m_ui->openButton->setEnabled (false);
    m_ui->openButton->setVisible (false);
    connect (m_ui->stopButton, SIGNAL (clicked()),
             this,               SLOT (cancelDownload()));
    connect (m_ui->openButton, SIGNAL (clicked()),
             this,               SLOT (installUpdate()));

    /* Resize to fit */
    setFixedSize (minimumSizeHint());
}

Downloader::~Downloader() {
    delete m_ui;
    delete m_reply;
    delete m_manager;
}

/**
 * Returns \c true if the updater shall not intervene when the download has
 * finished (you can use the \c QSimpleUpdater signals to know when the
 * download is completed).
 */
bool Downloader::useCustomInstallProcedures() const {
    return m_useCustomProcedures;
}

/**
 * Begins downloading the file at the given \a url
 */
void Downloader::startDownload (const QUrl& url) {
    /* Reset UI */
    m_ui->progressBar->setValue (0);
    m_ui->stopButton->setText (tr ("Stop"));
    m_ui->downloadLabel->setText (tr ("Downloading updates"));
    m_ui->timeLabel->setText (tr ("Time remaining") + ": " + tr ("unknown"));

    /* Start download */
    m_startTime = QDateTime::currentDateTime().toTime_t();
    m_reply = m_manager->get (QNetworkRequest (url));

    /* Update UI when download progress changes or download finishes */
    connect (m_reply, SIGNAL (downloadProgress (qint64, qint64)),
             this,      SLOT (updateProgress   (qint64, qint64)));
    connect (m_reply, SIGNAL (redirected       (QUrl)),
             this,      SLOT (startDownload    (QUrl)));
    connect (m_reply, SIGNAL (finished()),
             this,      SLOT (onDownloadFinished()));

    showNormal();
}

/**
 * Opens the downloaded file.
 * \note If the downloaded file is not found, then the function will alert the
 *       user about the error.
 */
void Downloader::openDownload() {
    if (!m_filePath.isEmpty())
        QDesktopServices::openUrl (QUrl::fromLocalFile (m_filePath));

    else {
        QMessageBox::critical (this,
                               tr ("Error"),
                               tr ("Cannot find downloaded update!"),
                               QMessageBox::Close);
    }
}

/**
 * Instructs the OS to open the downloaded file.
 *
 * \note If \c useCustomInstallProcedures() returns \c true, the function will
 *       not instruct the OS to open the downloaded file. You can use the
 *       signals fired by the \c QSimpleUpdater to install the update with your
 *       own implementations/code.
 */
void Downloader::installUpdate() {
    if (useCustomInstallProcedures())
        return;

    QMessageBox box;
    box.setIcon (QMessageBox::Question);
    box.setDefaultButton   (QMessageBox::Ok);
    box.setStandardButtons (QMessageBox::Ok | QMessageBox::Cancel);
    box.setInformativeText (tr ("Click \"OK\" to begin installing the update"));
    box.setText ("<h3>" +
                 tr ("In order to install the update, you may need to "
                     "quit the application.")
                 + "</h3>");

    if (box.exec() == QMessageBox::Ok) {
        if (!useCustomInstallProcedures())
            openDownload();
    }

    else {
        m_ui->openButton->setEnabled (true);
        m_ui->openButton->setVisible (true);
        m_ui->timeLabel->setText (tr ("Click the \"Open\" button to "
                                      "apply the update"));
    }
}

/**
 * Prompts the user if he/she wants to cancel the download and cancels the
 * download if the user agrees to do that.
 */
void Downloader::cancelDownload() {
    if (!m_reply->isFinished()) {
        QMessageBox box;
        box.setWindowTitle (tr ("Updater"));
        box.setIcon (QMessageBox::Question);
        box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
        box.setText (tr ("Are you sure you want to cancel the download?"));

        if (box.exec() == QMessageBox::Yes) {
            hide();
            m_reply->abort();
        }
    }

    else
        hide();
}

/**
 * Writes the downloaded data to a temp. directory and updates the UI controls.
 * \note If the function detects that the downloaded data is an HTML file
 *       (e.g. a redirection notice from the server), the function will add the
 *       *.html extension to the downloaded file. This ensures that the download
 *       will be resumed when the OS opens a web-browser with the redirection
 *       notice.
 */
void Downloader::onDownloadFinished() {
    m_ui->stopButton->setText    (tr ("Close"));
    m_ui->downloadLabel->setText (tr ("Download complete!"));
    m_ui->timeLabel->setText     (tr ("The installer will open separately")
                                  + "...");

    QByteArray data = m_reply->readAll();

    if (!data.isEmpty()) {
        QString name = m_reply->url().toString().split ("/").last();

        /* Check if we need to redirect */
        QUrl url = m_reply->attribute (QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (!url.isEmpty()) {
            startDownload (url);
            return;
        }

        /* Save downloaded data to disk */
        QFile file (QDir::tempPath() + "/" + name);
        if (file.open (QIODevice::WriteOnly)) {
            file.write (data);
            file.close();

            m_filePath = file.fileName();
            emit downloadFinished (m_reply->url().toString(), m_filePath);
        }

        /* Open downloaded update */
        m_reply->close();
        installUpdate();
    }
}

/**
 * Calculates the appropiate size units (bytes, KB or MB) for the received
 * data and the total download size. Then, this function proceeds to update the
 * dialog controls/UI.
 */
void Downloader::calculateSizes (qint64 received, qint64 total) {
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
void Downloader::updateProgress (qint64 received, qint64 total) {
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
void Downloader::calculateTimeRemaining (qint64 received, qint64 total) {
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
qreal Downloader::round (const qreal& input) {
    return roundf (input * 100) / 100;
}

/**
 * If the \a custom parameter is set to \c true, then the \c Downloader will not
 * attempt to open the downloaded file.
 *
 * Use the signals fired by the \c QSimpleUpdater to implement your own install
 * procedures.
 */
void Downloader::setUseCustomInstallProcedures (const bool& custom) {
    m_useCustomProcedures = custom;
}

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

//==============================================================================
// Class includes
//==============================================================================

#include "Updater.h"
#include "Downloader.h"

//==============================================================================
// System includes
//==============================================================================

#include <QJsonValue>
#include <QJsonObject>
#include <QMessageBox>
#include <QApplication>
#include <QJsonDocument>
#include <QDesktopServices>

//==============================================================================
// Updater::Updater
//==============================================================================

Updater::Updater() {
    m_url = "";
    m_openUrl = "";
    m_changelog = "";
    m_downloadUrl = "";
    m_latestVersion = "";
    m_notifyOnUpdate = true;
    m_notifyOnFinish = false;
    m_updateAvailable = false;
    m_downloaderEnabled = true;
    m_moduleName = qApp->applicationName();
    m_moduleVersion = qApp->applicationVersion();

    m_downloader = new Downloader();
    m_manager = new QNetworkAccessManager();

#if defined Q_OS_WIN
    m_platform = "windows";
#elif defined Q_OS_MAC
    m_platform = "osx";
#elif defined Q_OS_LINUX
    m_platform = "linux";
#elif defined Q_OS_ANDROID
    m_platform = "android";
#elif defined Q_OS_IOS
    m_platform = "ios";
#endif

    connect (m_downloader, SIGNAL (downloadFinished (QString, QString)),
             this,         SIGNAL (downloadFinished (QString, QString)));
    connect (m_manager,    SIGNAL (finished (QNetworkReply*)),
             this,           SLOT (onReply  (QNetworkReply*)));
}

//==============================================================================
// Updater::~Updater
//==============================================================================

Updater::~Updater() {
    delete m_downloader;
}

//==============================================================================
// Updater::url
//==============================================================================

QString Updater::url() const {
    return m_url;
}

//==============================================================================
// Updater::platformKey
//==============================================================================

QString Updater::platformKey() const {
    return m_platform;
}

//==============================================================================
// Updater::notifyOnUpdate
//==============================================================================

bool Updater::notifyOnUpdate() const {
    return m_notifyOnUpdate;
}

//==============================================================================
// Updater::notifyOnFinish
//==============================================================================

bool Updater::notifyOnFinish() const {
    return m_notifyOnFinish;
}

//==============================================================================
// Updater::updateAvailable
//==============================================================================

bool Updater::updateAvailable() const {
    return m_updateAvailable;
}

//==============================================================================
// Updater::downloaderEnabled
//==============================================================================

bool Updater::downloaderEnabled() const {
    return m_downloaderEnabled;
}

//==============================================================================
// Updater::m_changelog
//==============================================================================

QString Updater::changelog() const {
    return m_changelog;
}

//==============================================================================
// Updater::downloadUrl
//==============================================================================

QString Updater::downloadUrl() const {
    return m_downloadUrl;
}

//==============================================================================
// Updater::latestVersion
//==============================================================================

QString Updater::latestVersion() const {
    return m_latestVersion;
}

//==============================================================================
// Updater::moduleName
//==============================================================================

QString Updater::moduleName() const {
    return m_moduleName;
}

//==============================================================================
// Updater::moduleVersion
//==============================================================================

QString Updater::moduleVersion() const {
    return m_moduleVersion;
}

//==============================================================================
// Updater::useCustomInstallProcedures
//==============================================================================

bool Updater::useCustomInstallProcedures() const {
    return m_downloader->useCustomInstallProcedures();
}

//==============================================================================
// Updater::checkForUpdates
//==============================================================================

void Updater::checkForUpdates() {
    m_manager->get (QNetworkRequest (url()));
}

//==============================================================================
// Updater::setUrl
//==============================================================================

void Updater::setUrl (const QString& url) {
    m_url = url;
}

//==============================================================================
// Updater::setNotifyOnUpdate
//==============================================================================

void Updater::setNotifyOnUpdate (bool notify) {
    m_notifyOnUpdate = notify;
}

//==============================================================================
// Updater::setNotifyOnFinish
//==============================================================================

void Updater::setNotifyOnFinish (bool notify) {
    m_notifyOnFinish = notify;
}

//==============================================================================
// Updater::setPlatformKey
//==============================================================================

void Updater::setPlatformKey (const QString& platformKey) {
    m_platform = platformKey;
}

//==============================================================================
// Updater::setModuleName
//==============================================================================

void Updater::setModuleName (const QString& name) {
    m_moduleName = name;
}

//==============================================================================
// Updater::setDownloaderEnabled
//==============================================================================

void Updater::setDownloaderEnabled (bool enabled) {
    m_downloaderEnabled = enabled;
}

//==============================================================================
// Updater::setUseCustomInstallProcedures
//==============================================================================

void Updater::setUseCustomInstallProcedures (bool custom) {
    m_downloader->setUseCustomInstallProcedures (custom);
}

//==============================================================================
// Updater::setModuleVersion
//==============================================================================

void Updater::setModuleVersion (const QString& version) {
    m_moduleVersion = version;
}

//==============================================================================
// Updater::onReply
//==============================================================================

void Updater::onReply (QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument document = QJsonDocument::fromJson (reply->readAll());

        if (document.isNull())
            return;

        QJsonObject updates = document.object().value ("updates").toObject();
        QJsonObject platform = updates.value (platformKey()).toObject();

        m_openUrl = platform.value ("open-url").toString();
        m_changelog = platform.value ("changelog").toString();
        m_downloadUrl = platform.value ("download-url").toString();
        m_latestVersion = platform.value ("latest-version").toString();

        setUpdateAvailable (compare (latestVersion(), moduleVersion()));
    }

    emit checkingFinished (url());
}

//==============================================================================
// Updater::setUpdateAvailable
//==============================================================================

void Updater::setUpdateAvailable (bool available) {
    m_updateAvailable = available;

    QMessageBox box;
    box.setTextFormat (Qt::RichText);
    box.setIcon (QMessageBox::Information);

    if (updateAvailable() && (notifyOnUpdate() || notifyOnFinish())) {
        QString text = tr ("Would you like to download the update now?");
        QString title = "<h3>"
                        + tr ("Version %1 of %2 has been released!")
                        .arg (latestVersion()).arg (moduleName())
                        + "</h3>";

        box.setText (title);
        box.setInformativeText (text);
        box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
        box.setDefaultButton   (QMessageBox::Yes);

        if (box.exec() == QMessageBox::Yes) {
            if (!m_openUrl.isEmpty())
                QDesktopServices::openUrl (QUrl (m_openUrl));

            else if (downloaderEnabled())
                m_downloader->startDownload (downloadUrl());

            else
                QDesktopServices::openUrl (QUrl (downloadUrl()));
        }
    }

    else if (notifyOnFinish()) {
        box.setStandardButtons (QMessageBox::Close);
        box.setInformativeText (tr ("No updates are available for the moment"));
        box.setText ("<h3>"
                     + tr ("Congratulations! You are running the "
                           "latest version of %1").arg (moduleName())
                     + "</h3>");

        box.exec();
    }
}

//==============================================================================
// Updater::compare
//==============================================================================

bool Updater::compare (const QString& x, const QString& y) {
    QStringList versionsX = x.split (".");
    QStringList versionsY = y.split (".");

    int count = qMin (versionsX.count(), versionsY.count());

    for (int i = 0; i < count; ++i) {
        int a = QString (versionsX.at (i)).toInt();
        int b = QString (versionsY.at (i)).toInt();

        if (a > b)
            return true;

        else if (b > a)
            return false;
    }

    return versionsY.count() < versionsX.count();
}

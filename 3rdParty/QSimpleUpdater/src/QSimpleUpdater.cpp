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
// Class Includes
//==============================================================================

#include "Updater.h"
#include "QSimpleUpdater.h"

//==============================================================================
// Implementation hacks
//==============================================================================

static QList<QString>  URLS;
static QList<Updater*> UPDATERS;

//==============================================================================
// QSimpleUpdater::~QSimpleUpdater
//==============================================================================

QSimpleUpdater::~QSimpleUpdater() {
    URLS.clear();
    UPDATERS.clear();
}

//==============================================================================
// QSimpleUpdater::getInstance
//==============================================================================

QSimpleUpdater* QSimpleUpdater::getInstance() {
    static QSimpleUpdater updater;
    return &updater;
}

//==============================================================================
// QSimpleUpdater::getNotifyOnUpdate
//==============================================================================

bool QSimpleUpdater::getNotifyOnUpdate (const QString& url) const {
    return getUpdater (url)->notifyOnUpdate();
}

//==============================================================================
// QSimpleUpdater::getNotifyOnFinish
//==============================================================================

bool QSimpleUpdater::getNotifyOnFinish (const QString& url) const {
    return getUpdater (url)->notifyOnFinish();
}

//==============================================================================
// QSimpleUpdater::getUpdateAvailable
//==============================================================================

bool QSimpleUpdater::getUpdateAvailable (const QString& url) const {
    return getUpdater (url)->updateAvailable();
}

//==============================================================================
// QSimpleUpdater::getDownloaderEnabled
//==============================================================================

bool QSimpleUpdater::getDownloaderEnabled (const QString& url) const {
    return getUpdater (url)->downloaderEnabled();
}

//==============================================================================
// QSimpleUpdater::getChangelog
//==============================================================================

QString QSimpleUpdater::getChangelog (const QString& url) const {
    return getUpdater (url)->changelog();
}

//==============================================================================
// QSimpleUpdater::getDownloadUrl
//==============================================================================

QString QSimpleUpdater::getDownloadUrl (const QString& url) const {
    return getUpdater (url)->downloadUrl();
}

//==============================================================================
// QSimpleUpdater::getLatestVersion
//==============================================================================

QString QSimpleUpdater::getLatestVersion (const QString& url) const {
    return getUpdater (url)->latestVersion();
}

//==============================================================================
// QSimpleUpdater::getPlatformKey
//==============================================================================

QString QSimpleUpdater::getPlatformKey (const QString& url) const {
    return getUpdater (url)->platformKey();
}

//==============================================================================
// QSimpleUpdater::getModuleName
//==============================================================================

QString QSimpleUpdater::getModuleName (const QString& url) const {
    return getUpdater (url)->moduleName();
}

//==============================================================================
// QSimpleUpdater::getModuleVersion
//==============================================================================

QString QSimpleUpdater::getModuleVersion (const QString& url) const {
    return getUpdater (url)->moduleVersion();
}

//==============================================================================
// QSimpleUpdater::usesCustomInstallProcedures
//==============================================================================

bool QSimpleUpdater::usesCustomInstallProcedures (const QString& url) const {
    return getUpdater (url)->useCustomInstallProcedures();
}

//==============================================================================
// QSimpleUpdater::checkForUpdates
//==============================================================================

void QSimpleUpdater::checkForUpdates (const QString& url) {
    getUpdater (url)->checkForUpdates();
}

//==============================================================================
// QSimpleUpdater::setPlatformKey
//==============================================================================

void QSimpleUpdater::setPlatformKey (const QString& url,
                                     const QString& platform) {
    getUpdater (url)->setPlatformKey (platform);
}

//==============================================================================
// QSimpleUpdater::setModuleName
//==============================================================================

void QSimpleUpdater::setModuleName (const QString& url, const QString& name) {
    getUpdater (url)->setModuleName (name);
}

//==============================================================================
// QSimpleUpdater::setModuleVersion
//==============================================================================

void QSimpleUpdater::setModuleVersion (const QString& url,
                                       const QString& version) {
    getUpdater (url)->setModuleVersion (version);
}

//==============================================================================
// QSimpleUpdater::setNotifyOnUpdate
//==============================================================================

void QSimpleUpdater::setNotifyOnUpdate (const QString& url,
                                        bool notify) {
    getUpdater (url)->setNotifyOnUpdate (notify);
}

//==============================================================================
// QSimpleUpdater::setNotifyOnFinish
//==============================================================================

void QSimpleUpdater::setNotifyOnFinish (const QString& url,
                                        bool notify) {
    getUpdater (url)->setNotifyOnFinish (notify);
}

//==============================================================================
// QSimpleUpdater::setDownloaderEnabled
//==============================================================================

void QSimpleUpdater::setDownloaderEnabled (const QString& url,
                                           bool enabled) {
    getUpdater (url)->setDownloaderEnabled (enabled);
}

//==============================================================================
// QSimpleUpdater::setUseCustomInstallProcedures
//==============================================================================

void QSimpleUpdater::setUseCustomInstallProcedures (const QString& url,
        bool custom) {
    getUpdater (url)->setUseCustomInstallProcedures (custom);
}

//==============================================================================
// QSimpleUpdater::getUpdater
//==============================================================================

Updater* QSimpleUpdater::getUpdater (const QString& url) const {
    if (!URLS.contains (url)) {
        Updater* updater = new Updater;
        updater->setUrl (url);

        URLS.append (url);
        UPDATERS.append (updater);

        connect (updater, SIGNAL (checkingFinished (QString)),
                 this,    SIGNAL (checkingFinished (QString)));
        connect (updater, SIGNAL (downloadFinished (QString, QString)),
                 this,    SIGNAL (downloadFinished (QString, QString)));
    }

    return UPDATERS.at (URLS.indexOf (url));
}

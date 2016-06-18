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

#ifndef _QSIMPLEUPDATER_MAIN_H
#define _QSIMPLEUPDATER_MAIN_H

#include <QUrl>
#include <QList>
#include <QObject>

#if defined (QSU_SHARED)
#  define QSU_DECL Q_DECL_EXPORT
#elif defined (QSU_IMPORT)
#  define QSU_DECL Q_DECL_IMPORT
#else
#  define QSU_DECL
#endif

class Updater;

///
/// Project homepage: http://qsimpleupdater.sf.net/
/// Code repository: http://github.com/alex-spataru/qsimpleupdater
///
/// The \c QSimpleUpdater class manages the updater system and allows for
/// parallel application modules to check for updates and download them.
///
/// The behavior of each updater can be regulated by specifying the update
/// definitions URL (from where we download the individual update definitions)
/// and defining the desired options by calling the individual "setter"
/// functions (e.g. \c setNotifyOnUpdate()).
///
/// The \c QSimpleUpdater also implements an integrated downloader.
/// If you need to use a custom install procedure/code, just create a function
/// that is called when the \c downloadFinished() signal is emitted to
/// implement your own install procedures.
///
/// By default, the downloader will try to open the file as if you opened it
/// from a file manager or a web browser (with the "file:///" url).
///
class QSU_DECL QSimpleUpdater : public QObject {
    Q_OBJECT

  public:
    ///
    /// Returns the only instance of the class
    ///
    static QSimpleUpdater* getInstance();

    ///
    /// Returns \c true if the updater registered with the given \a url is set
    /// to notify the user when it finds an available update.
    ///
    bool getNotifyOnUpdate (const QString& url) const;

    ///
    /// Returns \c true if the updater registered with the given \a url is set
    /// to notify the user when it finishes checking for updates
    ///
    bool getNotifyOnFinish (const QString& url) const;

    ///
    /// Returns \c true if the updater registered with the given \a url has an
    /// update available.
    ///
    /// \note you should call \c checkForUpdates() for this URL first in order
    ///       for this function to regurn a valid value
    ///
    bool getUpdateAvailable (const QString& url) const;

    ///
    /// Returns \c true if the downloader is enabled for the updater registered
    /// with the given \a c url
    ///
    bool getDownloaderEnabled (const QString& url) const;

    ///
    /// Returns the changelog of the updater instance with the given \c url.
    ///
    QString getChangelog (const QString& url) const;

    ///
    /// Returns the URL from where we can download the update
    ///
    QString getDownloadUrl (const QString& url) const;

    ///
    /// Returns the latest version online
    ///
    QString getLatestVersion (const QString& url) const;

    ///
    /// Returns the platform of the updater with the given \c url.
    ///
    QString getPlatformKey (const QString& url) const;

    ///
    /// Returns the application name registered for the given \c url.
    ///
    QString getModuleName (const QString& url) const;

    ///
    /// Returns the application version registered for the given \c url
    ///
    QString getModuleVersion (const QString& url) const;

    ///
    /// Returns \c true if the downloader will not attempt to install the
    /// downloaded file.
    ///
    /// This can be useful if you want to use the \c downloadFinished() signal
    /// to implement your own install procedures.
    ///
    bool usesCustomInstallProcedures (const QString& url) const;

  public slots:
    ///
    /// Checks for updates by downloading the update definitions file at the
    /// given \a url.
    ///
    /// You can have more than one updater running at the same time while the
    /// \a url is different. Every updater instance will have its own set of
    /// settings.
    ///
    /// This can be used - for example - when having multiple shared modules
    /// (e.g. plugins) that can be updated separately.
    ///
    void checkForUpdates (const QString& url);

    ///
    /// Changes the platform key which we use to get version data and download
    /// link in the appcast in the given \c url.
    ///
    /// \note By default, the updater will use the name of the current operating
    ///       system as its platform key.
    ///
    void setPlatformKey (const QString& url, const QString& platform);

    ///
    /// Changes the application name to display in the notification messages
    /// for the given appcast \a url.
    ///
    /// This can be used - for example - when having multiple shared modules
    /// (e.g. plugins) that can be updated separately.
    ///
    /// \note By default, the updater will use the name given to the
    ///       \c QApplication during initialization of your application.
    ///
    void setModuleName (const QString& url, const QString& name);

    ///
    /// Changes the application version to use when comparing the local and
    /// remote application versions.
    ///
    /// This can be used - for example - when having multiple shared modules
    /// (e.g. plugins) that can be updated separately.
    ///
    /// \note By default, the updater will use the version given to the
    ///       \c QApplication during initialization of your application.
    ///
    void setModuleVersion (const QString& url, const QString& version);

    ///
    /// If \c notify is set to true, the \c QSimpleUpdater will notify the user
    /// when an update is available.
    ///
    /// If \c notify is set to false, the \c QSimpleUpdater will not notify the
    /// user when an update is available.
    ///
    /// \note this feature is enabled by default
    /// \note you should disable this feature if you are implementing your own
    ///       notification methods or update procedures in your application.
    /// \note this function only changes the behavior for the updater registered
    ///       with the given \a url.
    ///
    void setNotifyOnUpdate (const QString& url, bool notify);

    ///
    /// If set to \c true, the updater will notify the user when it finishes
    /// checking for updates (even where there are no updates available).
    ///
    /// If set to \c false (default), the updater will only notify the user
    /// when there is an update available (if setNotifyOnUpdate() is \c true).
    ///
    /// You can enable this feature when the user triggers manually the updater
    /// (e.g. by clicking on the "Check for Updates..." action on the menu).
    ///
    /// \note this feature is disabled by default
    /// \note you should disable this feature if you are implementing your own
    ///       notification methods or update procedures in your application.
    /// \note this function only changes the behavior for the updater registered
    ///       with the given \a url.
    ///
    void setNotifyOnFinish (const QString& url, bool notify);

    ///
    /// If set to true, the updater will allow the user to choose whenever to
    /// download the update directly from the application (instead of opening
    /// the given download link through a browser).
    ///
    /// \note this feature is enabled by default
    /// \note you should disable this if you are implementing your own update
    ///       procedures in your application.
    /// \note this function only changes the behavior for the updater registered
    ///       with the given \a url.
    ///
    void setDownloaderEnabled (const QString& url, bool enabled);

    ///
    /// If \c custom is set to true, then the Downloader will not attempt to
    /// open or install the downloaded updates. This can be useful if you want
    /// to implement your own install procedures using the \c downloadFinished()
    /// signal.
    ///
    void setUseCustomInstallProcedures (const QString& url, bool custom);

  signals:
    ///
    /// Emitted when the check for updates process finishes.
    /// You can use this function if you are implementing your own notification
    /// methods or download procedures.
    ///
    /// \note use of this signal is not obligatory if you don't want
    ///       to show a custom notification or create your own downloader.
    ///
    void checkingFinished (const QString& url);

    ///
    /// Emitted when the download has finished.
    /// You can use this to implement your own procedures to install the
    /// downloaded updates.
    ///
    void downloadFinished (const QString& url, const QString& filepath);

  protected:
    ~QSimpleUpdater();

  private:
    ///
    /// Returns the updater object registered with the given \a url.
    /// If an updater object with the given \a url is not found, then this
    /// function will create it and configure it.
    ///
    Updater* getUpdater (const QString& url) const;
};

#endif

/*
 * (C) Copyright 2014 Alex Spataru
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 */

#ifndef DOWNLOAD_DIALOG_H
#define DOWNLOAD_DIALOG_H

#include <QDialog>
#include <ui_Downloader.h>

namespace Ui {
class Downloader;
}

class QNetworkReply;
class QNetworkAccessManager;

class Downloader : public QWidget {
    Q_OBJECT

  public:
    explicit Downloader (QWidget* parent = 0);
    ~Downloader();

    ///
    /// Returns \c true if the downloader will not attempt to install the
    /// downloaded file.
    ///
    /// This can be useful if you want to use the \c downloadFinished() signal
    /// to implement your own install procedures.
    ///
    bool useCustomInstallProcedures() const;

  public slots:
    ///
    /// Begins downloading the update
    ///
    void startDownload (const QUrl& url);

    ///
    /// If \c custom is set to true, then the Downloader will not attempt to
    /// open or install the downloaded updates. This can be useful if you want
    /// to implement your own install procedures using the \c downloadFinished()
    /// signal.
    ///
    void setUseCustomInstallProcedures (bool custom);

  private slots:
    void openDownload();
    void installUpdate();
    void cancelDownload();
    void onDownloadFinished();
    void calculateSizes (qint64 received, qint64 total);
    void updateProgress (qint64 received, qint64 total);
    void calculateTimeRemaining (qint64 received, qint64 total);

  private:
    ///
    /// Rounds the \a input to the nearest integer
    ///
    float round (float input);

  signals:
    ///
    /// Emitted when the download has finished.
    /// You can use this to implement your own procedures to install the
    /// downloaded updates.
    ///
    void downloadFinished (const QString& url, const QString& filepath);

  private:
    uint m_startTime;
    QString m_filePath;
    Ui::Downloader* m_ui;
    QNetworkReply* m_reply;
    bool m_useCustomProcedures;
    QNetworkAccessManager* m_manager;
};

#endif

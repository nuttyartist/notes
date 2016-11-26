/*********************************************************************************************
* Mozila License
* Just a meantime project to see the ability of qt, the framework that my OS might be based on
* And for those linux users that beleive in the power of notes
*********************************************************************************************/

#ifndef UPDATERWINDOW_H
#define UPDATERWINDOW_H

#include <QWidget>

namespace Ui {
class UpdaterWindow;
}

class QNetworkReply;
class QSimpleUpdater;
class QNetworkAccessManager;

class UpdaterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit UpdaterWindow (QWidget *parent = 0);
    ~UpdaterWindow();

public slots:
    void checkForUpdates (bool silent);

private slots:
    void resizeToFit();
    void resetControls();
    void updateTitleLabel();
    void onUpdateAvailable();
    void onDownloadFinished();
    void onNoUpdateAvailable();
    void onDownloadButtonClicked();
    void startDownload (const QUrl& url);
    void openDownload(const QString &path);
    void onCheckFinished (const QString& url);
    void calculateSizes (qint64 received, qint64 total);
    void updateProgress (qint64 received, qint64 total);
    void calculateTimeRemaining (qint64 received, qint64 total);

private:
    qreal round (const qreal& input);

private:
    Ui::UpdaterWindow *m_ui;
    QSimpleUpdater* m_updater;

    bool m_silent;
    uint m_startTime;
    QNetworkReply* m_reply;
    bool m_checkingForUpdates;
    QNetworkAccessManager* m_manager;
};

#endif

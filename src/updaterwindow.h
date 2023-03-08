/*********************************************************************************************
 * Mozila License
 * Just a meantime project to see the ability of qt, the framework that my OS might be based on
 * And for those linux users that beleive in the power of notes
 *********************************************************************************************/

#ifndef UPDATERWINDOW_H
#define UPDATERWINDOW_H

#include <QDialog>
#include <QDir>

namespace Ui {
class UpdaterWindow;
}

class QMouseEvent;
class QNetworkReply;
class QSimpleUpdater;
class QNetworkAccessManager;

class UpdaterWindow : public QDialog
{
    Q_OBJECT

public:
    explicit UpdaterWindow(QWidget *parent = 0);
    ~UpdaterWindow();

    void setShowWindowDisable(const bool dontShowWindow);

public slots:
    void checkForUpdates(bool force);

signals:
    void dontShowUpdateWindowChanged(bool state);

private slots:
    void resetControls();
    void updateTitleLabel();
    void onUpdateAvailable();
    void onDownloadButtonClicked();
    void startDownload(const QUrl &url);
    void openDownload(const QString &file);
    void onCheckFinished(const QString &url);
    void onXdgOpenFinished(const int exitCode);
    void openDownloadFolder(const QString &file);
    void calculateSizes(qint64 received, qint64 total);
    void updateProgress(qint64 received, qint64 total);
    void calculateTimeRemaining(qint64 received, qint64 total);
    void onDownloadFinished();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    qreal round(qreal input);

private:
    QString m_fileName;
    const QDir m_downloadDir;

    Ui::UpdaterWindow *m_ui;

    QPoint m_dragPosition;

    int m_mousePressX;
    int m_mousePressY;
    bool m_canMoveWindow;
    bool m_checkingForUpdates;
    bool m_dontShowUpdateWindow;
    bool m_forced;

    uint m_startTime;
    QNetworkReply *m_reply;
    QSimpleUpdater *m_updater;
    QNetworkAccessManager *m_manager;
};

#endif

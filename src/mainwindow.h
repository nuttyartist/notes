/*********************************************************************************************
* Mozila License
* Just a meantime project to see the ability of qt, the framework that my OS might be based on
* And for those linux users that beleive in the power of notes
*********************************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtCore>
#include <QGroupBox>
#include <QPushButton>
#include <vector>
#include <QToolButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QSettings>
#include <QSplitter>
#include <QSystemTrayIcon>
#include <QMenu>
#include <qaction.h>
#include "notedata.h"
#include "notemodel.h"
#include "noteview.h"
#include "updaterwindow.h"
#include "dbmanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class tst_MainWindow;

public:

    enum class StretchSide{
        None = 0,
        Left,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
        };

    Q_ENUM(StretchSide)

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setMainWindowVisibility(bool state);

protected:
    void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent*) Q_DECL_OVERRIDE;
    bool eventFilter(QObject* object, QEvent* event) Q_DECL_OVERRIDE;

private:

    Ui::MainWindow* ui;

    QMenu* m_mainMenu;

    QAction* m_rightToLeftAction;
    QAction* m_checkForUpdatesAction;

    QTimer* m_autoSaveTimer;
    QSettings* m_settingsDatabase;
    QVBoxLayout* m_noteWidgetsContainer;
    QToolButton* m_clearButton;
    QPushButton* m_greenMaximizeButton;
    QPushButton* m_redCloseButton;
    QPushButton* m_yellowMinimizeButton;
    QPushButton* m_newNoteButton;
    QPushButton* m_trashButton;
    QPushButton* m_dotsButton;
    QTextEdit* m_textEdit;
    QLineEdit* m_lineEdit;
    QLabel* m_editorDateLabel;
    QSplitter *m_splitter;
    QSystemTrayIcon* m_trayIcon;
    QAction* m_restoreAction;
    QAction* m_quitAction;
    QMenu* m_trayIconMenu;

    NoteView* m_noteView;
    NoteModel* m_noteModel;
    NoteModel* m_deletedNotesModel;
    QSortFilterProxyModel* m_proxyModel;
    QModelIndex m_currentSelectedNoteProxy;
    QModelIndex m_selectedNoteBeforeSearchingInSource;
    QQueue<QString> m_searchQueue;
    DBManager* m_dbManager;

    UpdaterWindow m_updater;
    StretchSide m_stretchSide;

    int m_currentVerticalScrollAreaRange;
    int m_mousePressX;
    int m_mousePressY;
    int m_textEditLeftPadding;
    int m_noteCounter;
    int m_trashCounter;
    int m_layoutMargin;
    bool m_canMoveWindow;
    bool m_canStretchWindow;
    bool m_isTemp;
    bool m_isListViewScrollBarHidden;
    bool m_isContentModified;
    bool m_isOperationRunning;

    void setupMainWindow();
    void createActions();
    void createMenu();
    void setupFonts();
    void setupTrayIcon();
    void setupKeyboardShortcuts();
    void setupNewNoteButtonAndTrahButton();
    void setupSplitter();
    void setupLine();
    void setupRightFrame();
    void setupTitleBarButtons();
    void setupSignalsSlots();
    void autoCheckForUpdates();
    void setupLineEdit();
    void setupTextEdit();
    void setupDatabases();
    void setupModelView();
    void initializeSettingsDatabase();
    void createNewNoteIfEmpty();
    void setLayoutForScrollArea();
    void setButtonsAndFieldsEnabled(bool doEnable);
    void restoreStates();
    QString getFirstLine(const QString& str);
    QString getNoteDateEditor (QString dateEdited);
    NoteData *generateNote(QString noteName);
    QDateTime getQDateTime(QString date);
    void showNoteInEditor(const QModelIndex& noteIndex);
    void sortNotesList(QStringList &stringNotesList);
    void loadNotes();
    void saveNoteToDB(const QModelIndex& noteIndex);
    void removeNoteFromDB(const QModelIndex& noteIndex);
    void selectFirstNote();
    void moveNoteToTop();
    void clearSearch();
    void findNotesContain(const QString &keyword);
    void selectNote(const QModelIndex& noteIndex);
    void checkMigration();
    void migrateNote(QString notePath);
    void migrateTrash(QString trashPath);

private slots:
    void InitData();
    void onNewNoteButtonPressed();
    void onNewNoteButtonClicked();
    void onTrashButtonPressed();
    void onTrashButtonClicked();
    void onDotsButtonPressed();
    void onDotsButtonClicked();
    void onNotePressed(const QModelIndex &index);
    void onTextEditTextChanged();
    void onLineEditTextChanged(const QString& keyword);
    void onClearButtonClicked();
    void onGreenMaximizeButtonPressed ();
    void onYellowMinimizeButtonPressed ();
    void onRedCloseButtonPressed ();
    void onGreenMaximizeButtonClicked();
    void onYellowMinimizeButtonClicked();
    void onRedCloseButtonClicked();
    void createNewNote();
    void deleteNote(const QModelIndex& noteIndex, bool isFromUser=true);
    void deleteSelectedNote();
    void setFocusOnCurrentNote();
    void selectNoteDown();
    void selectNoteUp();
    void setFocusOnText();
    void fullscreenWindow();
    void maximizeWindow();
    void minimizeWindow();
    void QuitApplication();
    void checkForUpdates (const bool clicked);
};

#endif // MAINWINDOW_H

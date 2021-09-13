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
#include <QProgressDialog>
#include <QAction>

#include <QAutostart>

#include "notedata.h"
#include "notemodel.h"
#include "noteview.h"
#include "updaterwindow.h"
#include "styleeditorwindow.h"
#include "dbmanager.h"
#include "markdownhighlighter.h"
#include "customDocument.h"
#include "aboutwindow.h"
#include "framelesswindow.h"

namespace Ui {
class MainWindow;
}

#if defined(Q_OS_LINUX)
class MainWindow : public QMainWindow
#else
class MainWindow : public CFramelessWindow
#endif
{
    Q_OBJECT

    friend class tst_MainWindow;

public:

    enum class ShadowType{
        Linear = 0,
        Radial
        };

    enum class ShadowSide{
        Left = 0,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
        };

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

#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    Q_ENUM(ShadowType)
    Q_ENUM(ShadowSide)
    Q_ENUM(StretchSide)
#endif

    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow() Q_DECL_OVERRIDE;

    void setMainWindowVisibility(bool state);

protected:
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent*) Q_DECL_OVERRIDE;
    bool eventFilter(QObject* object, QEvent* event) Q_DECL_OVERRIDE;

private:

    Ui::MainWindow* ui;

    QTimer* m_autoSaveTimer;
    QSettings* m_settingsDatabase;
    QToolButton* m_clearButton;
    QPushButton* m_greenMaximizeButton;
    QPushButton* m_redCloseButton;
    QPushButton* m_yellowMinimizeButton;
    QHBoxLayout m_trafficLightLayout;
    QPushButton* m_newNoteButton;
    QPushButton* m_trashButton;
    QPushButton* m_dotsButton;
    QPushButton* m_styleEditorButton;
    CustomDocument* m_textEdit;
    QLineEdit* m_searchEdit;
    QLabel* m_editorDateLabel;
    QSplitter *m_splitter;
    bool is_markdown_enabled=true;
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
    QThread* m_dbThread;
    MarkdownHighlighter *m_highlighter;

    UpdaterWindow m_updater;
    StyleEditorWindow m_styleEditorWindow;
    AboutWindow m_aboutWindow;
    StretchSide m_stretchSide;
    Autostart m_autostart;
    int m_mousePressX;
    int m_mousePressY;
    int m_noteCounter;
    int m_trashCounter;
    int m_layoutMargin;
    int m_shadowWidth;
    int m_noteListWidth;
    int m_smallEditorWidth;
    int m_largeEditorWidth;
    int m_currentMinimumEditorPadding;
    int m_currentAdaptableEditorPadding;
    bool m_canMoveWindow;
    bool m_canStretchWindow;
    bool m_isTemp;
    bool m_isListViewScrollBarHidden;
    bool m_isContentModified;
    bool m_isOperationRunning;
    bool m_dontShowUpdateWindow;
    bool m_alwaysStayOnTop;
    bool m_useNativeWindowFrame;

    QStringList m_listOfSerifFonts;
    QStringList m_listOfSansSerifFonts;
    QStringList m_listOfMonoFonts;
    int m_chosenSerifFontIndex;
    int m_chosenSansSerifFontIndex;
    int m_chosenMonoFontIndex;
    int m_editorMediumFontSize;
    int m_currentFontPointSize;
    struct m_charsLimitPerFont {
        int mono;
        int serif;
        int sansSerif;
    } m_currentCharsLimitPerFont;
    FontTypeface m_currentFontTypeface;
    QString m_currentFontFamily;
    QFont m_currentSelectedFont;
    QString m_displayFont;
    QColor m_currentEditorBackgroundColor;
    QColor m_currentRightFrameColor;
    Theme m_currentTheme;
    QColor m_currentEditorTextColor;
    QColor m_currentThemeBackgroundColor;
    bool m_areNonEditorWidgetsVisible;
    bool m_isFrameRightTopWidgetsVisible;

    void setupMainWindow();
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
    void setupSearchEdit();
    void resetEditorSettings();
    void setupTextEditStyleSheet(int paddingLeft, int paddingRight);
    void alignTextEditText();
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
    NoteData* generateNote(const int noteID);
    QDateTime getQDateTime(QString date);
    void showNoteInEditor(const QModelIndex& noteIndex);
    void sortNotesList(QStringList &stringNotesList);
    void saveNoteToDB(const QModelIndex& noteIndex);
    void removeNoteFromDB(const QModelIndex& noteIndex);
    void selectFirstNote();
    void moveNoteToTop();
    void clearSearch();
    void highlightSearch() const;
    void findNotesContain(const QString &keyword);
    void selectNote(const QModelIndex& noteIndex);
    void checkMigration();
    void executeImport(const bool replace);
    void migrateNote(QString notePath);
    void migrateTrash(QString trashPath);
    void setCurrentFontBasedOnTypeface(FontTypeface selectedFontTypeFace);
    void setVisibilityOfFrameRightNonEditor(bool isVisible);
    void adjustUpperWidgets(bool shouldPushUp);
    void setSearchEditStyleSheet(bool isFocused);

    void dropShadow(QPainter& painter, ShadowType type, ShadowSide side);
    void fillRectWithGradient(QPainter& painter, const QRect& rect, QGradient& gradient);
    double gaussianDist(double x, const double center, double sigma) const;

    void setMargins(QMargins margins);

private slots:
    void InitData();
    void loadNotes(QList<NoteData *> noteList, int noteCounter);
    void onNewNoteButtonPressed();
    void onNewNoteButtonClicked();
    void onTrashButtonPressed();
    void onTrashButtonClicked();
    void onDotsButtonPressed();
    void onDotsButtonClicked();
    void onStyleEditorButtonPressed();
    void onStyleEditorButtonClicked();
    void onNotePressed(const QModelIndex &index);
    void onTextEditTextChanged();
    void onSearchEditTextChanged(const QString& keyword);
    void onClearButtonClicked();
    void onGreenMaximizeButtonPressed();
    void onYellowMinimizeButtonPressed();
    void onRedCloseButtonPressed();
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
    void collapseNoteList();
    void expandNoteList();
    void enableMarkdownHighlighter();
    void disableMarkdownHighlighter();
    void toggleNoteList();
    void importNotesFile(const bool clicked);
    void exportNotesFile(const bool clicked);
    void restoreNotesFile (const bool clicked);
    void stayOnTop(bool checked);
    void askBeforeSettingNativeWindowFrame();
    void setUseNativeWindowFrame(bool useNativeWindowFrame);
    void toggleStayOnTop();
    void onSearchEditReturnPressed();
    void changeEditorFontTypeFromStyleButtons(FontTypeface fontType);
    void changeEditorFontSizeFromStyleButtons(FontSizeAction fontSizeAction);
    void changeEditorTextWidthFromStyleButtons(EditorTextWidth editorTextWidth);
    void resetEditorToDefaultSettings();
    void setTheme(Theme theme);
    void createOrSelectFirstNote();

signals:
    void requestNotesList();
    void requestOpenDBManager(QString path, bool doCreate);
    void requestCreateUpdateNote(NoteData* note);
    void requestDeleteNote(NoteData* note);
    void requestRestoreNotes(QList<NoteData *> noteList);
    void requestImportNotes(QList<NoteData *> noteList);
    void requestExportNotes(QString fileName);
    void requestMigrateNotes(QList<NoteData *> noteList);
    void requestMigrateTrash(QList<NoteData *> noteList);
    void requestForceLastRowIndexValue(int index);
};

#endif // MAINWINDOW_H

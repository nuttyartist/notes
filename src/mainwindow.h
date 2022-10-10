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
#include <QtGlobal>

#include "nodedata.h"
#include "notelistmodel.h"
#include "notelistview.h"
#include "nodetreemodel.h"
#include "updaterwindow.h"
#include "styleeditorwindow.h"
#include "dbmanager.h"
#include "customDocument.h"
#include "aboutwindow.h"
#include "framelesswindow.h"
#include "nodetreeview.h"

namespace Ui {
class MainWindow;
}
class TreeViewLogic;
class ListViewLogic;
class NoteEditorLogic;
class TagPool;
class SpliterStyle;

#if defined(Q_OS_WINDOWS) || defined(Q_OS_WIN)
//#if defined(__MINGW32__) || defined(__GNUC__)
using MainWindowBase = QMainWindow;
//#else
//using MainWindowBase = CFramelessWindow;
//#endif
#elif defined(Q_OS_MACOS)
using MainWindowBase = CFramelessWindow;
#else
using MainWindowBase = QMainWindow;
#endif
class MainWindow : public MainWindowBase
{
    Q_OBJECT

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

public slots:
    void saveLastSelectedFolderTags(bool isFolder, const QString &folderPath, const QSet<int>& tagId);
    void saveExpandedFolder(const QStringList& folderPaths);
    void saveLastSelectedNote(const QSet<int> &notesId);

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
    NoteEditorLogic* m_noteEditorLogic;
    QLineEdit* m_searchEdit;
    QLabel* m_editorDateLabel;
    QSplitter *m_splitter;
    QSystemTrayIcon* m_trayIcon;
    QAction* m_restoreAction;
    QAction* m_quitAction;
    QMenu* m_trayIconMenu;

    NoteListView* m_listView;
    NoteListModel* m_listModel;
    ListViewLogic* m_listViewLogic;
    NodeTreeView* m_treeView;
    NodeTreeModel* m_treeModel;
    TreeViewLogic* m_treeViewLogic;
    TagPool* m_tagPool;
    DBManager* m_dbManager;
    QThread* m_dbThread;
    SpliterStyle* m_spliterStyle;
    UpdaterWindow m_updater;
    StyleEditorWindow m_styleEditorWindow;
    AboutWindow m_aboutWindow;
    StretchSide m_stretchSide;
    Autostart m_autostart;
    int m_mousePressX;
    int m_mousePressY;
    int m_trashCounter;
    int m_layoutMargin;
    int m_shadowWidth;
    int m_noteListWidth;
    int m_nodeTreeWidth;
    int m_smallEditorWidth;
    int m_largeEditorWidth;
    bool m_canMoveWindow;
    bool m_canStretchWindow;
    bool m_isTemp;
    bool m_isListViewScrollBarHidden;
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
    bool m_isNoteListCollapsed;
    bool m_isTreeCollapsed;
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

    bool alreadyAppliedFormat(const QString &formatChars);
    void applyFormat(const QString &formatChars);
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
    void setLayoutForScrollArea();
    void setButtonsAndFieldsEnabled(bool doEnable);
    void resetFormat(const QString &formatChars);
    void restoreStates();
    void migrateFromV0_9_0();
    void executeImport(const bool replace);
    void migrateNoteFromV0_9_0(QString notePath);
    void migrateTrashFromV0_9_0(QString trashPath);
    void setCurrentFontBasedOnTypeface(FontTypeface selectedFontTypeFace);
    void setVisibilityOfFrameRightNonEditor(bool isVisible);
    void setWindowButtonsVisible(bool isVisible);
    void adjustUpperWidgets(bool shouldPushUp);
    void setSearchEditStyleSheet(bool isFocused);

    void dropShadow(QPainter& painter, ShadowType type, ShadowSide side);
    void fillRectWithGradient(QPainter& painter, const QRect& rect, QGradient& gradient);
    double gaussianDist(double x, const double center, double sigma) const;

    void setMargins(QMargins margins);

private slots:
    void InitData();

    void onNewNoteButtonPressed();
    void onNewNoteButtonClicked();
    void onTrashButtonPressed();
    void onTrashButtonClicked();
    void onDotsButtonPressed();
    void onDotsButtonClicked();
    void onStyleEditorButtonPressed();
    void onStyleEditorButtonClicked();
    void onClearButtonClicked();
    void onGreenMaximizeButtonPressed();
    void onYellowMinimizeButtonPressed();
    void onRedCloseButtonPressed();
    void onGreenMaximizeButtonClicked();
    void onYellowMinimizeButtonClicked();
    void onRedCloseButtonClicked();
    void resetBlockFormat();
    void createNewNote();
    void selectNoteDown();
    void selectNoteUp();
    void setFocusOnText();
    void fullscreenWindow();
    void makeCode();
    void makeBold();
    void makeItalic();
    void makeStrikethrough();
    void maximizeWindow();
    void minimizeWindow();
    void QuitApplication();
    void checkForUpdates();
    void collapseNoteList();
    void expandNoteList();
    void toggleNoteList();
    void collapseNodeTree();
    void expandNodeTree();
    void toggleNodeTree();
    void importNotesFile();
    void exportNotesFile();
    void restoreNotesFile();
    void stayOnTop(bool checked);
    void askBeforeSettingNativeWindowFrame();
    void increaseHeading();
    void decreaseHeading();
    void setHeading(int level);
    void setUseNativeWindowFrame(bool useNativeWindowFrame);
    void toggleStayOnTop();
    void onSearchEditReturnPressed();
    void changeEditorFontTypeFromStyleButtons(FontTypeface fontType);
    void changeEditorFontSizeFromStyleButtons(FontSizeAction fontSizeAction);
    void changeEditorTextWidthFromStyleButtons(EditorTextWidth editorTextWidth);
    void resetEditorToDefaultSettings();
    void setTheme(Theme theme);
    void deleteSelectedNote();
    void clearSearch();
    void showErrorMessage(const QString& title, const QString& content);
    void setNoteListLoading();
    void selectAllNotesInList();
    void updateFrame();

signals:
    void requestNodesTree();
    void requestOpenDBManager(const QString& path, bool doCreate);
    void requestRestoreNotes(const QString& filePath);
    void requestImportNotes(const QString& filePath);
    void requestExportNotes(QString fileName);
    void requestMigrateNotesFromV0_9_0(QVector<NodeData>& noteList);
    void requestMigrateTrashFromV0_9_0(QVector<NodeData>& noteList);
    void requestMigrateNotesFromV1_5_0(const QString& path);
    void requestChangeDatabasePath(const QString& newPath);
};

#endif // MAINWINDOW_H

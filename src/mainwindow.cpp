/**************************************************************************************
 * We believe in the power of notes to help us record ideas and thoughts.
 * We want people to have an easy, beautiful and simple way of doing that.
 * And so we have Notes.
 ***************************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qxtglobalshortcut.h"
#include "treeviewlogic.h"
#include "listviewlogic.h"
#include "noteeditorlogic.h"
#include "tagpool.h"
#include "splitterstyle.h"

#include <QScrollBar>
#include <QShortcut>
#include <QTextStream>
#include <QScrollArea>
#include <QtConcurrent>
#include <QProgressDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QList>
#include <QWidgetAction>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlQuery>

#define DEFAULT_DATABASE_NAME "default_database"

/*!
 * \brief MainWindow::MainWindow
 * \param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : MainWindowBase(parent),
      ui(new Ui::MainWindow),
      m_settingsDatabase(nullptr),
      m_clearButton(nullptr),
      m_greenMaximizeButton(nullptr),
      m_redCloseButton(nullptr),
      m_yellowMinimizeButton(nullptr),
      m_trafficLightLayout(nullptr),
      m_newNoteButton(nullptr),
      m_trashButton(nullptr),
      m_dotsButton(nullptr),
      m_styleEditorButton(nullptr),
      m_textEdit(nullptr),
      m_searchEdit(nullptr),
      m_editorDateLabel(nullptr),
      m_splitter(nullptr),
      m_trayIcon(new QSystemTrayIcon(this)),
#if !defined(Q_OS_MAC)
      m_restoreAction(new QAction(tr("&Hide Notes"), this)),
      m_quitAction(new QAction(tr("&Quit"), this)),
#endif
      m_listView(nullptr),
      m_listModel(nullptr),
      m_listViewLogic(nullptr),
      m_treeView(nullptr),
      m_treeModel(new NodeTreeModel(this)),
      m_treeViewLogic(nullptr),
      m_tagPool(nullptr),
      m_dbManager(nullptr),
      m_dbThread(nullptr),
      m_styleEditorWindow(this),
      m_aboutWindow(this),
      m_trashCounter(0),
      m_layoutMargin(10),
      m_shadowWidth(10),
      m_smallEditorWidth(420),
      m_largeEditorWidth(1250),
      m_canMoveWindow(false),
      m_canStretchWindow(false),
      m_isTemp(false),
      m_isListViewScrollBarHidden(true),
      m_isOperationRunning(false),
#if defined(UPDATE_CHECKER)
      m_dontShowUpdateWindow(false),
#endif
      m_alwaysStayOnTop(false),
      m_useNativeWindowFrame(false),
      m_hideToTray(false),
      m_listOfSerifFonts(
              { QStringLiteral("Trykker"), QStringLiteral("PT Serif"), QStringLiteral("Mate") }),
      m_listOfSansSerifFonts({ QStringLiteral("Source Sans Pro"), QStringLiteral("Roboto") }),
      m_listOfMonoFonts({ QStringLiteral("iA Writer Mono S"), QStringLiteral("iA Writer Duo S"),
                          QStringLiteral("iA Writer Quattro S") }),
      m_chosenSerifFontIndex(0),
      m_chosenSansSerifFontIndex(0),
      m_chosenMonoFontIndex(0),
      m_currentCharsLimitPerFont({ 64, // Mono    TODO: is this the proper way to initialize?
                                   80, // Serif
                                   80 }), // SansSerif
      m_currentFontTypeface(FontTypeface::SansSerif),
#ifdef __APPLE__
      m_displayFont(QFont(QStringLiteral("SF Pro Text")).exactMatch()
                            ? QStringLiteral("SF Pro Text")
                            : QStringLiteral("Roboto")),
#elif _WIN32
      m_displayFont(QFont(QStringLiteral("Segoe UI")).exactMatch() ? QStringLiteral("Segoe UI")
                                                                   : QStringLiteral("Roboto")),
#else
      m_displayFont(QStringLiteral("Roboto")),
#endif
      m_currentEditorBackgroundColor(247, 247, 247),
      m_currentRightFrameColor(247, 247, 247),
      m_currentTheme(Theme::Light),
      m_currentEditorTextColor(26, 26, 26),
      m_currentThemeBackgroundColor(247, 247, 247),
      m_areNonEditorWidgetsVisible(true),
      m_isFrameRightTopWidgetsVisible(true)
{
    ui->setupUi(this);
    setupMainWindow();
    setupFonts();
    setupKeyboardShortcuts();
    setupNewNoteButtonAndTrahButton();
    setupSplitter();
    setupLine();
    setupRightFrame();
    setupTitleBarButtons();
    setupSearchEdit();
    setupDatabases();
    setupModelView();
    setupTextEdit();
    restoreStates();
    setupSignalsSlots();
#if defined(UPDATE_CHECKER)
    autoCheckForUpdates();
#endif

    QTimer::singleShot(200, this, SLOT(InitData()));
}

/*!
 * \brief MainWindow::InitData
 * Init the data from database and select the first note if there is one
 */
void MainWindow::InitData()
{
    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());
    QString oldNoteDBPath(dir.path() + QStringLiteral("/Notes.ini"));
    QString oldTrashDBPath(dir.path() + QStringLiteral("/Trash.ini"));

    bool isV0_9_0 = (QFile::exists(oldNoteDBPath) || QFile::exists(oldTrashDBPath));
    if (isV0_9_0) {
        QProgressDialog *pd =
                new QProgressDialog(tr("Migrating database, please wait."), QString(), 0, 0, this);
        pd->setCancelButton(nullptr);
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
        pd->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::FramelessWindowHint);
#else
        pd->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
#endif
        pd->setMinimumDuration(0);
        pd->show();

        setButtonsAndFieldsEnabled(false);

        QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
        connect(watcher, &QFutureWatcher<void>::finished, this, [&, pd]() {
            pd->deleteLater();
            setButtonsAndFieldsEnabled(true);
        });

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QFuture<void> migration = QtConcurrent::run(&MainWindow::migrateFromV0_9_0, this);
#else
        QFuture<void> migration = QtConcurrent::run(this, &MainWindow::migrateFromV0_9_0);
#endif
        watcher->setFuture(migration);
    }
    /// Check if it is running with an argument (ex. hide)
    if (qApp->arguments().contains(QStringLiteral("--autostart"))
        && QSystemTrayIcon::isSystemTrayAvailable()) {
        setMainWindowVisibility(false);
    }

    // init tree view
    emit requestNodesTree();
}

/*!
 * \brief Toggles visibility of the main window upon system tray activation
 * \param reason The reason the system tray was activated
 */
void MainWindow::onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        setMainWindowVisibility(!isVisible());
    }
}

/*!
 * \brief MainWindow::setMainWindowVisibility
 * \param state
 */
void MainWindow::setMainWindowVisibility(bool state)
{
    if (state) {
        show();
        raise();
        activateWindow();
#if !defined(Q_OS_MAC)
        m_restoreAction->setText(tr("&Hide Notes"));
#endif
    } else {
#if !defined(Q_OS_MAC)
        m_restoreAction->setText(tr("&Show Notes"));
#endif
        hide();
    }
}

void MainWindow::saveLastSelectedFolderTags(bool isFolder, const QString &folderPath,
                                            const QSet<int> &tagId)
{
    m_settingsDatabase->setValue("isSelectingFolder", isFolder);
    m_settingsDatabase->setValue("currentSelectFolder", folderPath);
    QStringList sl;
    for (const auto &id : tagId) {
        sl.append(QString::number(id));
    }
    m_settingsDatabase->setValue("currentSelectTagsId", sl);
}

void MainWindow::saveExpandedFolder(const QStringList &folderPaths)
{
    m_settingsDatabase->setValue("currentExpandedFolder", folderPaths);
}

void MainWindow::saveLastSelectedNote(const QSet<int> &notesId)
{
    QStringList sl;
    for (const auto &id : notesId) {
        sl.append(QString::number(id));
    }
    m_settingsDatabase->setValue("currentSelectNotesId", sl);
}

/*!
 * \brief MainWindow::paintEvent
 * \param event
 */
void MainWindow::paintEvent(QPaintEvent *event)
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    if (!m_useNativeWindowFrame) {
        QPainter painter(this);
        painter.save();

        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);

        dropShadow(painter, ShadowType::Linear, ShadowSide::Left);
        dropShadow(painter, ShadowType::Linear, ShadowSide::Top);
        dropShadow(painter, ShadowType::Linear, ShadowSide::Right);
        dropShadow(painter, ShadowType::Linear, ShadowSide::Bottom);

        dropShadow(painter, ShadowType::Radial, ShadowSide::TopLeft);
        dropShadow(painter, ShadowType::Radial, ShadowSide::TopRight);
        dropShadow(painter, ShadowType::Radial, ShadowSide::BottomRight);
        dropShadow(painter, ShadowType::Radial, ShadowSide::BottomLeft);

        painter.restore();
    }
#endif

    QMainWindow::paintEvent(event);
}

/*!
 * \brief MainWindow::resizeEvent
 * \param event
 */
void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (m_splitter) {
        // restore note list width
        updateFrame();
    }

    QMainWindow::resizeEvent(event);
}

/*!
 * \brief MainWindow::~MainWindow
 * Deconstructor of the class
 */
MainWindow::~MainWindow()
{
    delete ui;
    m_dbThread->quit();
    m_dbThread->wait();
    delete m_dbThread;
}

/*!
 * \brief MainWindow::setupMainWindow
 * Setting up main window prefrences like frameless window and the minimum size of the window
 * Setting the window background color to be white
 */
void MainWindow::setupMainWindow()
{
#if !defined(Q_OS_MAC)
    auto flags = Qt::Window | Qt::CustomizeWindowHint;
#  if defined(Q_OS_UNIX)
    flags |= Qt::FramelessWindowHint;
#  endif
    setWindowFlags(flags);
#endif

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    setAttribute(Qt::WA_TranslucentBackground);
#endif

    m_greenMaximizeButton = new QPushButton(this);
    m_redCloseButton = new QPushButton(this);
    m_yellowMinimizeButton = new QPushButton(this);
#ifndef __APPLE__
    //    If we want to align window buttons with searchEdit and notesList
    //    QSpacerItem *horizontialSpacer = new QSpacerItem(3, 0, QSizePolicy::Minimum,
    //    QSizePolicy::Minimum); m_trafficLightLayout.addSpacerItem(horizontialSpacer);
    m_trafficLightLayout.addWidget(m_redCloseButton);
    m_trafficLightLayout.addWidget(m_yellowMinimizeButton);
    m_trafficLightLayout.addWidget(m_greenMaximizeButton);
#else
    setCloseBtnQuit(false);
    m_layoutMargin = 0;
    m_greenMaximizeButton->setVisible(false);
    m_redCloseButton->setVisible(false);
    m_yellowMinimizeButton->setVisible(false);
#endif

#ifdef _WIN32
    m_layoutMargin = 0;
    m_trafficLightLayout.setSpacing(0);
    m_trafficLightLayout.setContentsMargins(QMargins(0, 0, 0, 0));
    m_trafficLightLayout.setGeometry(QRect(2, 2, 90, 16));
#endif

    m_newNoteButton = ui->newNoteButton;
    m_trashButton = ui->trashButton;
    m_dotsButton = ui->dotsButton;
    m_styleEditorButton = ui->styleEditorButton;
    m_searchEdit = ui->searchEdit;
    m_textEdit = ui->textEdit;
    m_editorDateLabel = ui->editorDateLabel;
    m_splitter = ui->splitter;
    m_foldersWidget = ui->frameLeft;
    m_noteListWidget = ui->frameMiddle;
    // don't resize first two panes when resizing
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 0);
    m_splitter->setStretchFactor(2, 1);

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    QMargins margins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin);
    setMargins(margins);
#elif _WIN32
    ui->verticalSpacer->changeSize(0, 7, QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->verticalSpacer_upEditorDateLabel->changeSize(0, 27, QSizePolicy::Fixed, QSizePolicy::Fixed);
#endif
    ui->frame->installEventFilter(this);
    ui->centralWidget->setMouseTracking(true);
    setMouseTracking(true);
    QPalette pal(palette());
    pal.setColor(QPalette::Window, QColor(248, 248, 248));
    setAutoFillBackground(true);
    setPalette(pal);

    m_newNoteButton->setToolTip(tr("Create New Note"));
    m_trashButton->setToolTip(tr("Delete Selected Note"));
    m_dotsButton->setToolTip(tr("Open Menu"));
    m_styleEditorButton->setToolTip(tr("Style The Editor"));

    m_styleEditorButton->setText(QStringLiteral("Aa"));
#if __APPLE__
    m_styleEditorButton->setFont(QFont(QStringLiteral("Roboto"), 20, QFont::Bold));
#else
    m_styleEditorButton->setFont(QFont(QStringLiteral("Roboto"), 16, QFont::Bold));

#endif
    QString ss = QStringLiteral("QPushButton { "
                                "  border: none; "
                                "  padding: 0px; "
                                "  color: rgb(68, 138, 201);"
                                "}");
    m_styleEditorButton->setStyleSheet(ss);
    m_styleEditorButton->installEventFilter(this);

    ui->toggleTreeViewButton->setMaximumSize({ 33, 25 });
    ui->toggleTreeViewButton->setMinimumSize({ 33, 25 });
    ui->toggleTreeViewButton->setCursor(QCursor(Qt::PointingHandCursor));
    ui->toggleTreeViewButton->setFocusPolicy(Qt::TabFocus);
    ui->toggleTreeViewButton->setIconSize(QSize(16, 16));
    ui->toggleTreeViewButton->setStyleSheet(QStringLiteral(R"(QPushButton { )"
                                                           R"(    border: none; )"
                                                           R"(    padding: 0px; )"
                                                           R"(})"));
    ui->toggleTreeViewButton->setHoveredIcon(QIcon(QString::fromUtf8(":/images/drawer_icon.png")));
    ui->toggleTreeViewButton->setNormalIcon(QIcon(QString::fromUtf8(":/images/drawer_icon.png")));
    ui->toggleTreeViewButton->setPressedIcon(QIcon(QString::fromUtf8(":/images/drawer_icon.png")));
    ui->listviewLabel2->setMinimumSize({ 40, 25 });
    ui->listviewLabel2->setMaximumSize({ 40, 25 });

#ifdef __APPLE__
    QFont titleFont(m_displayFont, 13, QFont::DemiBold);
#else
    QFont titleFont(m_displayFont, 10, QFont::DemiBold);
#endif
    ui->listviewLabel1->setFont(titleFont);
    ui->listviewLabel2->setFont(titleFont);
    ui->listviewLabel1->setStyleSheet("QLabel { color :  rgb(0, 0, 0); }");
    ui->listviewLabel2->setStyleSheet("QLabel { color :  rgb(132, 132, 132); }");
    m_splitterStyle = new SplitterStyle();
    m_splitter->setStyle(m_splitterStyle);
    m_splitter->setHandleWidth(0);
    setNoteListLoading();
#ifdef __APPLE__
    ui->searchEdit->setFocus();
    QTimer::singleShot(16, ui->searchEdit, &QTextEdit::clearFocus);
#endif
    setWindowIcon(QIcon(QStringLiteral(":images/notes_icon.ico")));
}

/*!
 * \brief MainWindow::setupFonts
 */
void MainWindow::setupFonts()
{
#ifdef __APPLE__
    m_searchEdit->setFont(QFont(m_displayFont, 12));
    m_editorDateLabel->setFont(QFont(m_displayFont, 12, QFont::Bold));
#else
    m_searchEdit->setFont(QFont(m_displayFont, 10));
    m_editorDateLabel->setFont(QFont(m_displayFont, 10, QFont::Bold));
#endif
}

/*!
 * \brief MainWindow::setupTrayIcon
 */
void MainWindow::setupTrayIcon()
{
#if !defined(Q_OS_MAC)
    auto trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(m_restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(m_quitAction);
    m_trayIcon->setContextMenu(trayIconMenu);
#endif

    QIcon icon(QStringLiteral(":images/notes_system_tray_icon.png"));
    m_trayIcon->setIcon(icon);
    m_trayIcon->show();
}

/*!
 * \brief MainWindow::setupKeyboardShortcuts
 * Setting up the keyboard shortcuts
 */
void MainWindow::setupKeyboardShortcuts()
{
    new QShortcut(QKeySequence(Qt::Key_F10), this, SLOT(onDotsButtonClicked()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this, SLOT(onNewNoteButtonClicked()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_D), this, SLOT(deleteSelectedNote()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), m_searchEdit, SLOT(setFocus()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_E), m_searchEdit, SLOT(clear()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Down), this, SLOT(selectNoteDown()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Up), this, SLOT(selectNoteUp()));
    new QShortcut(QKeySequence(Qt::Key_Down), this, SLOT(selectNoteDown()));
    new QShortcut(QKeySequence(Qt::Key_Up), this, SLOT(selectNoteUp()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Enter), this, SLOT(setFocusOnText()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), this, SLOT(setFocusOnText()));
    // new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(setFocusOnText()));
    // new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(setFocusOnText()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F), this, SLOT(fullscreenWindow()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L), this, SLOT(maximizeWindow()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M), this, SLOT(minimizeWindow()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this, SLOT(QuitApplication()));
#if defined(Q_OS_MACOS) || defined(Q_OS_WINDOWS)
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_K), this, SLOT(toggleStayOnTop()));
#endif
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_J), this, SLOT(toggleNoteList()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), this,
                  SLOT(onStyleEditorButtonClicked()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_J), this, SLOT(toggleFolderTree()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_A), this, SLOT(selectAllNotesInList()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_QuoteLeft), this, SLOT(makeCode()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_B), this, SLOT(makeBold()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_I), this, SLOT(makeItalic()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this, SLOT(makeStrikethrough()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Minus), this,
                  SLOT(decreaseHeading()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Equal), this,
                  SLOT(increaseHeading()));
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_0), this), &QShortcut::activated, this,
            [=]() { setHeading(0); });
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_1), this), &QShortcut::activated, this,
            [=]() { setHeading(1); });
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_2), this), &QShortcut::activated, this,
            [=]() { setHeading(2); });
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_3), this), &QShortcut::activated, this,
            [=]() { setHeading(3); });
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_4), this), &QShortcut::activated, this,
            [=]() { setHeading(4); });
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_5), this), &QShortcut::activated, this,
            [=]() { setHeading(5); });
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_6), this), &QShortcut::activated, this,
            [=]() { setHeading(6); });
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Backslash), this, SLOT(resetBlockFormat()));

    QxtGlobalShortcut *shortcut = new QxtGlobalShortcut(this);
#if defined(Q_OS_MACOS)
    shortcut->setShortcut(QKeySequence(Qt::META | Qt::Key_N));
#else
    shortcut->setShortcut(QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_N));
#endif
    connect(shortcut, &QxtGlobalShortcut::activated, this, [=]() {
        // workaround prevent textEdit and searchEdit
        // from taking 'N' from shortcut
        m_textEdit->setDisabled(true);
        m_searchEdit->setDisabled(true);
        setMainWindowVisibility(isHidden() || windowState() == Qt::WindowMinimized
                                || qApp->applicationState() == Qt::ApplicationInactive);
        if (isHidden() || windowState() == Qt::WindowMinimized
            || qApp->applicationState() == Qt::ApplicationInactive)
#ifdef __APPLE__
            raise();
#else
            activateWindow();
#endif
        m_textEdit->setDisabled(false);
        m_searchEdit->setDisabled(false);
    });
}

/*!
 * \brief MainWindow::setupNewNoteButtonAndTrahButton
 * We need to set up some different values when using apple os x
 * This is because if we want to get the native button look in os x,
 * due to some bug in Qt, I think, the values of width and height of buttons
 * needs to be more than 50 and less than 34 respectively.
 * So some modifications needs to be done.
 */
void MainWindow::setupNewNoteButtonAndTrahButton()
{
    QString ss = QStringLiteral("QPushButton { "
                                "  border: none; "
                                "  padding: 0px; "
                                "}");

    m_newNoteButton->setStyleSheet(ss);
    m_trashButton->setStyleSheet(ss);
    m_dotsButton->setStyleSheet(ss);

    m_newNoteButton->installEventFilter(this);
    m_trashButton->installEventFilter(this);
    m_dotsButton->installEventFilter(this);
    m_styleEditorButton->installEventFilter(this);
}

/*!
 * \brief MainWindow::setupSplitter
 * Set up the splitter that control the size of the scrollArea and the textEdit
 */
void MainWindow::setupSplitter()
{
    m_splitter->setCollapsible(0, false);
    m_splitter->setCollapsible(1, false);
    m_splitter->setCollapsible(2, false);
}

/*!
 * \brief MainWindow::setupLine
 * Set up the vertical line that seperate between the scrollArea to the textEdit
 */
void MainWindow::setupLine()
{
    ui->line->setStyleSheet(QStringLiteral("border: 1px solid rgb(191, 191, 191)"));
    ui->line_2->setStyleSheet(QStringLiteral("border: 1px solid rgb(191, 191, 191)"));
}

/*!
 * \brief MainWindow::setupRightFrame
 * Set up a frame above textEdit and behind the other widgets for a unifed background
 * in thet editor section
 */
void MainWindow::setupRightFrame()
{
    ui->frameRightTop->installEventFilter(this);

    QString ss = QStringLiteral("QFrame{ "
                                "  background-color: %1; "
                                "  border: none;"
                                "}")
                         .arg(m_currentRightFrameColor.name());
    ui->frameRight->setStyleSheet(ss);
}

/*!
 * \brief MainWindow::setupTitleBarButtons
 * Setting up the red (close), yellow (minimize), and green (maximize) buttons
 * Make only the buttons icon visible
 * And install this class event filter to them, to act when hovering on one of them
 */
void MainWindow::setupTitleBarButtons()
{
    QString ss = QStringLiteral("QPushButton { "
                                "  border: none; "
                                "  padding: 0px; "
                                "}");

    m_redCloseButton->setStyleSheet(ss);
    m_yellowMinimizeButton->setStyleSheet(ss);
    m_greenMaximizeButton->setStyleSheet(ss);

#ifdef _WIN32
    m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/windows_close_regular.png")));
    m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/windows_maximize_regular.png")));
    m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/windows_minimize_regular.png")));

    m_redCloseButton->setIconSize(QSize(34, 16));
    m_yellowMinimizeButton->setIconSize(QSize(28, 16));
    m_greenMaximizeButton->setIconSize(QSize(28, 16));
#endif

    m_redCloseButton->installEventFilter(this);
    m_yellowMinimizeButton->installEventFilter(this);
    m_greenMaximizeButton->installEventFilter(this);
}

/*!
 * \brief MainWindow::setupSignalsSlots
 * connect between signals and slots
 */
void MainWindow::setupSignalsSlots()
{
#if defined(UPDATE_CHECKER)
    connect(&m_updater, &UpdaterWindow::dontShowUpdateWindowChanged, this,
            [=](bool state) { m_dontShowUpdateWindow = state; });
#endif
    // Style Editor Window
    connect(&m_styleEditorWindow, &StyleEditorWindow::changeFontType, this,
            [=](FontTypeface fontType) { changeEditorFontTypeFromStyleButtons(fontType); });
    connect(&m_styleEditorWindow, &StyleEditorWindow::changeFontSize, this,
            [=](FontSizeAction fontSizeAction) {
                changeEditorFontSizeFromStyleButtons(fontSizeAction);
            });
    connect(&m_styleEditorWindow, &StyleEditorWindow::changeEditorTextWidth, this,
            [=](EditorTextWidth editorTextWidth) {
                changeEditorTextWidthFromStyleButtons(editorTextWidth);
            });
    connect(&m_styleEditorWindow, &StyleEditorWindow::resetEditorToDefaultSettings, this,
            [=]() { resetEditorToDefaultSettings(); });
    connect(&m_styleEditorWindow, &StyleEditorWindow::changeTheme, this,
            [=](Theme theme) { setTheme(theme); });
    // actions
    // connect(rightToLeftActionion, &QAction::triggered, this, );
    // connect(checkForUpdatesAction, &QAction::triggered, this, );
    // green button
    connect(m_greenMaximizeButton, &QPushButton::pressed, this,
            &MainWindow::onGreenMaximizeButtonPressed);
    connect(m_greenMaximizeButton, &QPushButton::clicked, this,
            &MainWindow::onGreenMaximizeButtonClicked);
    // red button
    connect(m_redCloseButton, &QPushButton::pressed, this, &MainWindow::onRedCloseButtonPressed);
    connect(m_redCloseButton, &QPushButton::clicked, this, &MainWindow::onRedCloseButtonClicked);
    // yellow button
    connect(m_yellowMinimizeButton, &QPushButton::pressed, this,
            &MainWindow::onYellowMinimizeButtonPressed);
    connect(m_yellowMinimizeButton, &QPushButton::clicked, this,
            &MainWindow::onYellowMinimizeButtonClicked);
    // new note button
    connect(m_newNoteButton, &QPushButton::pressed, this, &MainWindow::onNewNoteButtonPressed);
    connect(m_newNoteButton, &QPushButton::clicked, this, &MainWindow::onNewNoteButtonClicked);
    // delete note button
    connect(m_trashButton, &QPushButton::pressed, this, &MainWindow::onTrashButtonPressed);
    connect(m_trashButton, &QPushButton::clicked, this, &MainWindow::onTrashButtonClicked);
    connect(m_listModel, &NoteListModel::rowsRemoved, this,
            [this]() { m_trashButton->setEnabled(true); });
    // 3 dots button
    connect(m_dotsButton, &QPushButton::pressed, this, &MainWindow::onDotsButtonPressed);
    connect(m_dotsButton, &QPushButton::clicked, this, &MainWindow::onDotsButtonClicked);
    // Style Editor Button
    connect(m_styleEditorButton, &QPushButton::pressed, this,
            &MainWindow::onStyleEditorButtonPressed);
    connect(m_styleEditorButton, &QPushButton::clicked, this,
            &MainWindow::onStyleEditorButtonClicked);
    // textEdit scrollbar triggered
    connect(m_textEdit->verticalScrollBar(), &QAbstractSlider::actionTriggered, this, [=]() {
        if (m_isFrameRightTopWidgetsVisible)
            setVisibilityOfFrameRightNonEditor(false);
    });
    // line edit text changed
    connect(m_searchEdit, &QLineEdit::textChanged, m_listViewLogic,
            &ListViewLogic::onSearchEditTextChanged);
    // line edit enter key pressed
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &MainWindow::onSearchEditReturnPressed);
    // clear button
    connect(m_clearButton, &QToolButton::clicked, this, &MainWindow::onClearButtonClicked);
    // System tray activation
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onSystemTrayIconActivated);

#if !defined(Q_OS_MAC)
    // System tray context menu action: "Show/Hide Notes"
    connect(m_restoreAction, &QAction::triggered, this, [this]() {
        setMainWindowVisibility(isHidden() || windowState() == Qt::WindowMinimized
                                || (qApp->applicationState() == Qt::ApplicationInactive));
    });
    // System tray context menu action: "Quit"
    connect(m_quitAction, &QAction::triggered, this, &MainWindow::QuitApplication);
    // Application state changed
    connect(qApp, &QApplication::applicationStateChanged, this,
            [this]() { m_listView->update(m_listView->currentIndex()); });
#endif

    // MainWindow <-> DBManager
    connect(this, &MainWindow::requestNodesTree, m_dbManager, &DBManager::onNodeTagTreeRequested,
            Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestRestoreNotes, m_dbManager,
            &DBManager::onRestoreNotesRequested, Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestImportNotes, m_dbManager, &DBManager::onImportNotesRequested,
            Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestExportNotes, m_dbManager, &DBManager::onExportNotesRequested,
            Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestMigrateNotesFromV0_9_0, m_dbManager,
            &DBManager::onMigrateNotesFromV0_9_0Requested, Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestMigrateTrashFromV0_9_0, m_dbManager,
            &DBManager::onMigrateTrashFrom0_9_0Requested, Qt::BlockingQueuedConnection);

    connect(m_listViewLogic, &ListViewLogic::showNotesInEditor, m_noteEditorLogic,
            &NoteEditorLogic::showNotesInEditor);
    connect(m_listViewLogic, &ListViewLogic::closeNoteEditor, m_noteEditorLogic,
            &NoteEditorLogic::closeEditor);
    connect(m_noteEditorLogic, &NoteEditorLogic::setVisibilityOfFrameRightNonEditor, this,
            [this](bool vl) { setVisibilityOfFrameRightNonEditor(vl); });
    connect(m_noteEditorLogic, &NoteEditorLogic::moveNoteToListViewTop, m_listViewLogic,
            &ListViewLogic::moveNoteToTop);
    connect(m_noteEditorLogic, &NoteEditorLogic::updateNoteDataInList, m_listViewLogic,
            &ListViewLogic::setNoteData);
    connect(m_noteEditorLogic, &NoteEditorLogic::deleteNoteRequested, m_listViewLogic,
            &ListViewLogic::deleteNoteRequested);
    connect(m_listViewLogic, &ListViewLogic::noteTagListChanged, m_noteEditorLogic,
            &NoteEditorLogic::onNoteTagListChanged);
    connect(m_noteEditorLogic, &NoteEditorLogic::noteEditClosed, m_listViewLogic,
            &ListViewLogic::onNoteEditClosed);
    connect(m_listViewLogic, &ListViewLogic::requestClearSearchUI, this, &MainWindow::clearSearch);
    connect(m_treeViewLogic, &TreeViewLogic::addNoteToTag, m_listViewLogic,
            &ListViewLogic::onAddTagRequestD);
    connect(m_listViewLogic, &ListViewLogic::listViewLabelChanged, this,
            [this](const QString &l1, const QString &l2) {
                ui->listviewLabel1->setText(l1);
                ui->listviewLabel2->setText(l2);
                m_splitter->setHandleWidth(0);
            });
    connect(ui->toggleTreeViewButton, &QPushButton::pressed, this, &MainWindow::toggleFolderTree);
    connect(m_dbManager, &DBManager::showErrorMessage, this, &MainWindow::showErrorMessage,
            Qt::QueuedConnection);
    connect(m_listViewLogic, &ListViewLogic::requestNewNote, this,
            &MainWindow::onNewNoteButtonClicked);
    connect(m_listViewLogic, &ListViewLogic::moveNoteRequested, this, [this](int id, int target) {
        m_treeViewLogic->onMoveNodeRequested(id, target);
        m_treeViewLogic->openFolder(target);
    });
    connect(m_listViewLogic, &ListViewLogic::setNewNoteButtonVisible, this,
            [this](bool visible) { ui->newNoteButton->setVisible(visible); });
    connect(m_treeViewLogic, &TreeViewLogic::noteMoved, m_listViewLogic,
            &ListViewLogic::onNoteMovedOut);

    connect(m_listViewLogic, &ListViewLogic::requestClearSearchDb, this,
            &MainWindow::setNoteListLoading);
    connect(m_treeView, &NodeTreeView::loadNotesInTagsRequested, this,
            &MainWindow::setNoteListLoading);
    connect(m_treeView, &NodeTreeView::loadNotesInFolderRequested, this,
            &MainWindow::setNoteListLoading);
#ifdef __APPLE__
    // Replace setUseNativeWindowFrame with just the part that handles pushing things up
    connect(this, &MainWindow::toggleFullScreen, this,
            [=](bool isFullScreen) { adjustUpperWidgets(isFullScreen); });
#endif
    connect(m_treeView, &NodeTreeView::saveExpand, this, &MainWindow::saveExpandedFolder);
    connect(m_treeView, &NodeTreeView::saveSelected, this, &MainWindow::saveLastSelectedFolderTags);
    connect(m_listView, &NoteListView::saveSelectedNote, this, &MainWindow::saveLastSelectedNote);
    connect(m_treeView, &NodeTreeView::saveLastSelectedNote, m_listViewLogic,
            &ListViewLogic::setLastSelectedNote);
    connect(m_treeView, &NodeTreeView::requestLoadLastSelectedNote, m_listViewLogic,
            &ListViewLogic::loadLastSelectedNoteRequested);
    connect(m_treeView, &NodeTreeView::loadNotesInFolderRequested, m_listViewLogic,
            &ListViewLogic::onNotesListInFolderRequested);
    connect(m_treeView, &NodeTreeView::loadNotesInTagsRequested, m_listViewLogic,
            &ListViewLogic::onNotesListInTagsRequested);
    connect(this, &MainWindow::requestChangeDatabasePath, m_dbManager,
            &DBManager::onChangeDatabasePathRequested, Qt::QueuedConnection);
}

/*!
 * \brief MainWindow::autoCheckForUpdates
 * Checks for updates, if an update is found, then the updater dialog will show
 * up, otherwise, no notification shall be showed
 */
#if defined(UPDATE_CHECKER)
void MainWindow::autoCheckForUpdates()
{
    m_updater.installEventFilter(this);
    m_updater.setShowWindowDisable(m_dontShowUpdateWindow);
    m_updater.checkForUpdates(false);
}
#endif

void MainWindow::setSearchEditStyleSheet(bool isFocused = false)
{
    m_searchEdit->setStyleSheet(
            QStringLiteral("QLineEdit{ "
                           "  color: %3;"
                           "  padding-left: 21px;"
                           "  padding-right: 19px;"
                           "  border: %2;"
                           "  border-radius: 3px;"
                           "  background: %1;"
                           "  selection-background-color: rgb(61, 155, 218);"
                           "} "
                           "QLineEdit:focus { "
                           "  border: 2px solid rgb(61, 155, 218);"
                           "}"
                           "QToolButton { "
                           "  border: none; "
                           "  padding: 0px;"
                           "}")
                    .arg(m_currentThemeBackgroundColor.name(),
                         isFocused ? "2px solid rgb(61, 155, 218)" : "1px solid rgb(205, 205, 205)",
                         m_currentEditorTextColor.name()));
}

/*!
 * \brief MainWindow::setupSearchEdit
 * Set the lineedit to start a bit to the right and end a bit to the left (pedding)
 */
void MainWindow::setupSearchEdit()
{
    //    QLineEdit* searchEdit = m_searchEdit;

    m_searchEdit->setAttribute(Qt::WA_MacShowFocusRect, 0);

    // clear button
    m_clearButton = new QToolButton(m_searchEdit);
    QPixmap pixmap(QStringLiteral(":images/closeButton.png"));
    m_clearButton->setIcon(QIcon(pixmap));
    QSize clearSize(15, 15);
    m_clearButton->setIconSize(clearSize);
    m_clearButton->setCursor(Qt::ArrowCursor);
    m_clearButton->hide();

    // search button
    QToolButton *searchButton = new QToolButton(m_searchEdit);
    QPixmap newPixmap(QStringLiteral(":images/magnifyingGlass.png"));
    searchButton->setIcon(QIcon(newPixmap));
    QSize searchSize(18, 18);
    searchButton->setIconSize(searchSize);
    searchButton->setCursor(Qt::ArrowCursor);

    // layout
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::RightToLeft, m_searchEdit);
    layout->setContentsMargins(2, 0, 3, 0);
    layout->addWidget(m_clearButton);
    layout->addStretch();
    layout->addWidget(searchButton);
    m_searchEdit->setLayout(layout);

    m_searchEdit->installEventFilter(this);

    setSearchEditStyleSheet(false);
}

/*!
 * \brief MainWindow::setCurrentFontBasedOnTypeface
 * Set the current font based on a given typeface
 */
void MainWindow::setCurrentFontBasedOnTypeface(FontTypeface selectedFontTypeFace)
{
    m_currentFontTypeface = selectedFontTypeFace;
    switch (selectedFontTypeFace) {
    case FontTypeface::Mono:
        m_currentFontFamily = m_listOfMonoFonts.at(m_chosenMonoFontIndex);
        m_textEdit->setLineWrapColumnOrWidth(m_currentCharsLimitPerFont.mono);
        break;
    case FontTypeface::Serif:
        m_currentFontFamily = m_listOfSerifFonts.at(m_chosenSerifFontIndex);
        m_textEdit->setLineWrapColumnOrWidth(m_currentCharsLimitPerFont.serif);
        break;
    case FontTypeface::SansSerif:
        m_currentFontFamily = m_listOfSansSerifFonts.at(m_chosenSansSerifFontIndex);
        m_textEdit->setLineWrapColumnOrWidth(m_currentCharsLimitPerFont.sansSerif);
        break;
    }

#ifdef __APPLE__
    int increaseSize = 2;
#else
    int increaseSize = 1;
#endif

    if (m_textEdit->width() < m_smallEditorWidth) {
        m_currentFontPointSize = m_editorMediumFontSize - 2;
    } else if (m_textEdit->width() > m_smallEditorWidth
               && m_textEdit->width() < m_largeEditorWidth) {
        m_currentFontPointSize = m_editorMediumFontSize;
    } else if (m_textEdit->width() > m_largeEditorWidth) {
        m_currentFontPointSize = m_editorMediumFontSize + increaseSize;
    }

    m_currentSelectedFont = QFont(m_currentFontFamily, m_currentFontPointSize, QFont::Normal);
    m_textEdit->setFont(m_currentSelectedFont);

    // Set tab width
    QFontMetrics currentFontMetrics(m_currentSelectedFont);
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
    m_textEdit->setTabStopWidth(4 * currentFontMetrics.width(' '));
#else
    m_textEdit->setTabStopDistance(4 * currentFontMetrics.horizontalAdvance(QLatin1Char(' ')));
#endif

    alignTextEditText();
}

/*!
 * \brief MainWindow::resetEditorSettings
 * Reset editor settings to default options
 */
void MainWindow::resetEditorSettings()
{
    m_currentFontTypeface = FontTypeface::SansSerif;
    m_chosenMonoFontIndex = 0;
    m_chosenSerifFontIndex = 0;
    m_chosenSansSerifFontIndex = 0;
#ifdef __APPLE__
    m_editorMediumFontSize = 17;
#else
    m_editorMediumFontSize = 13;
#endif
    m_currentFontPointSize = m_editorMediumFontSize;
    m_currentCharsLimitPerFont.mono = 64;
    m_currentCharsLimitPerFont.serif = 80;
    m_currentCharsLimitPerFont.sansSerif = 80;
    m_textEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
    m_textEdit->setWordWrapMode(QTextOption::WordWrap);
    m_currentTheme = Theme::Light;

    m_styleEditorWindow.changeSelectedFont(FontTypeface::Mono,
                                           m_listOfMonoFonts.at(m_chosenMonoFontIndex));
    m_styleEditorWindow.changeSelectedFont(FontTypeface::Serif,
                                           m_listOfSerifFonts.at(m_chosenSerifFontIndex));
    m_styleEditorWindow.changeSelectedFont(FontTypeface::SansSerif,
                                           m_listOfSansSerifFonts.at(m_chosenSansSerifFontIndex));

    setCurrentFontBasedOnTypeface(m_currentFontTypeface);
    setTheme(m_currentTheme);
    m_styleEditorWindow.restoreSelectedOptions(false, m_currentFontTypeface, m_currentTheme);
}

void MainWindow::setupTextEditStyleSheet(int paddingLeft, int paddingRight)
{
    m_textEdit->setDocumentPadding(paddingLeft, 0, paddingRight, 2);

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    QString ss =
            QString("QTextEdit {background-color: %1;} "
                    "QTextEdit{selection-background-color: rgb(63, 99, 139);}"
                    "QTextEdit{color: %2}"
                    "QScrollBar::handle:vertical:hover { background: rgba(40, 40, 40, 0.5); }"
                    "QScrollBar::handle:vertical:pressed { background: rgba(40, 40, 40, 0.5); }"
                    "QScrollBar::handle:vertical { border-radius: 4px; background: rgba(100, 100, "
                    "100, 0.5); min-height: 20px; }"
                    "QScrollBar::vertical {border-radius: 6px; width: 10px; color: rgba(255, 255, "
                    "255,0);}"
                    "QScrollBar {margin-right: 2px; background: transparent;}"
                    "QScrollBar::add-line:vertical { width:0px; height: 0px; subcontrol-position: "
                    "bottom; subcontrol-origin: margin; }"
                    "QScrollBar::sub-line:vertical { width:0px; height: 0px; subcontrol-position: "
                    "top; subcontrol-origin: margin; }")
                    .arg(m_currentEditorBackgroundColor.name(), m_currentEditorTextColor.name());
#else
    QString ss =
            QString("QTextEdit {background-color: %1;} "
                    "QTextEdit{selection-background-color: rgb(63, 99, 139);}"
                    "QTextEdit{color: %2}")
                    .arg(m_currentEditorBackgroundColor.name(), m_currentEditorTextColor.name());
#endif

    m_textEdit->setStyleSheet(ss);
}

/*!
 * \brief MainWindow::alignTextEditText
 * If textEdit's text can be contained (enough space for current chars limit per font)
 * then, align textEdit's text to the center by padding textEdit's margins
 */
void MainWindow::alignTextEditText()
{
    if (m_textEdit->lineWrapMode() == QTextEdit::WidgetWidth) {
        setupTextEditStyleSheet(m_noteEditorLogic->currentMinimumEditorPadding(),
                                m_noteEditorLogic->currentMinimumEditorPadding());
        return;
    }

    QFontMetricsF fm(m_currentSelectedFont);
    QString limitingStringSample =
            QString("The quick brown fox jumps over the lazy dog the quick brown fox jumps over "
                    "the lazy dog the quick brown fox jumps over the lazy dog");
    limitingStringSample.truncate(m_textEdit->lineWrapColumnOrWidth());
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
    qreal textSamplePixelsWidth = fm.width(limitingStringSample);
#else
    qreal textSamplePixelsWidth = fm.horizontalAdvance(limitingStringSample);
#endif
    m_noteEditorLogic->setCurrentAdaptableEditorPadding(
            (m_textEdit->width() - textSamplePixelsWidth) / 2 - 10);

    if (m_textEdit->width() - m_noteEditorLogic->currentMinimumEditorPadding() * 2
                > textSamplePixelsWidth
        && m_noteEditorLogic->currentAdaptableEditorPadding() > 0
        && m_noteEditorLogic->currentAdaptableEditorPadding()
                > m_noteEditorLogic->currentMinimumEditorPadding()) {
        setupTextEditStyleSheet(m_noteEditorLogic->currentAdaptableEditorPadding(),
                                m_noteEditorLogic->currentAdaptableEditorPadding());
    } else {
        setupTextEditStyleSheet(m_noteEditorLogic->currentMinimumEditorPadding(),
                                m_noteEditorLogic->currentMinimumEditorPadding());
    }
}

/*!
 * \brief MainWindow::setupTextEdit
 * Setting up textEdit:
 * Setup the style of the scrollBar and set textEdit background to an image
 * Make the textEdit pedding few pixels right and left, to compel with a beautiful proportional grid
 * And install this class event filter to catch when text edit is having focus
 */
void MainWindow::setupTextEdit()
{
    setupTextEditStyleSheet(0, 0);
    m_textEdit->installEventFilter(this);
    m_textEdit->verticalScrollBar()->installEventFilter(this);
    m_textEdit->setCursorWidth(2);
    setupTextEditStyleSheet(m_noteEditorLogic->currentMinimumEditorPadding(),
                            m_noteEditorLogic->currentMinimumEditorPadding());
    m_textEdit->setWordWrapMode(QTextOption::WordWrap);

#ifdef __APPLE__
    if (QFont("Helvetica Neue").exactMatch()) {
        m_listOfSansSerifFonts.push_front("Helvetica Neue");
    } else if (QFont("Helvetica").exactMatch()) {
        m_listOfSansSerifFonts.push_front("Helvetica");
    }

    if (QFont("SF Pro Text").exactMatch()) {
        m_listOfSansSerifFonts.push_front("SF Pro Text");
    }

    if (QFont("Avenir Next").exactMatch()) {
        m_listOfSansSerifFonts.push_front("Avenir Next");
    } else if (QFont("Avenir").exactMatch()) {
        m_listOfSansSerifFonts.push_front("Avenir");
    }
#elif _WIN32
    if (QFont("Calibri").exactMatch())
        m_listOfSansSerifFonts.push_front("Calibri");

    if (QFont("Arial").exactMatch())
        m_listOfSansSerifFonts.push_front("Arial");

    if (QFont("Segoe UI").exactMatch())
        m_listOfSansSerifFonts.push_front("Segoe UI");
#endif

    // This is done because for now where we're only handling plain text,
    // and we don't want people to past rich text and get something wrong.
    // In future versions, where we'll support rich text, we'll need to change that.
    m_textEdit->setAcceptRichText(false);
}

/*!
 * \brief MainWindow::initializeSettingsDatabase
 */
void MainWindow::initializeSettingsDatabase()
{
    // Why are we not updating the app version in Settings?
    if (m_settingsDatabase->value(QStringLiteral("version"), "NULL") == "NULL")
        m_settingsDatabase->setValue(QStringLiteral("version"), qApp->applicationVersion());

#if defined(UPDATE_CHECKER)
    if (m_settingsDatabase->value(QStringLiteral("dontShowUpdateWindow"), "NULL") == "NULL")
        m_settingsDatabase->setValue(QStringLiteral("dontShowUpdateWindow"),
                                     m_dontShowUpdateWindow);
#endif

    if (m_settingsDatabase->value(QStringLiteral("windowGeometry"), "NULL") == "NULL") {
        int initWidth = 870;
        int initHeight = 630;
        QPoint center = qApp->primaryScreen()->geometry().center();
        QRect rect(center.x() - initWidth / 2, center.y() - initHeight / 2, initWidth, initHeight);
        setGeometry(rect);
        m_settingsDatabase->setValue(QStringLiteral("windowGeometry"), saveGeometry());
    }

    if (m_settingsDatabase->value(QStringLiteral("splitterSizes"), "NULL") == "NULL") {
        m_splitter->resize(width() - 2 * m_layoutMargin, height() - 2 * m_layoutMargin);
        updateFrame();
        m_settingsDatabase->setValue(QStringLiteral("splitterSizes"), m_splitter->saveState());
    }
}

/*!
 * \brief MainWindow::setupDatabases
 * Setting up the database:
 */
void MainWindow::setupDatabases()
{
    m_settingsDatabase =
            new QSettings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("Awesomeness"),
                          QStringLiteral("Settings"), this);
    m_settingsDatabase->setFallbacksEnabled(false);
    bool needMigrateFromV1_5_0 = false;
    if (m_settingsDatabase->value(QStringLiteral("version"), "NULL") == "NULL") {
        needMigrateFromV1_5_0 = true;
    }
    auto versionString = m_settingsDatabase->value(QStringLiteral("version")).toString();
    auto major = versionString.split(".").first().toInt();
    if (major < 2) {
        needMigrateFromV1_5_0 = true;
    }
    initializeSettingsDatabase();

    bool doCreate = false;
    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());
    bool folderCreated = dir.mkpath(QStringLiteral("."));
    if (!folderCreated)
        qFatal("ERROR: Can't create settings folder : %s",
               dir.absolutePath().toStdString().c_str());
    QString defaultDBPath = dir.path() + QDir::separator() + QStringLiteral("notes.db");

    QString noteDBFilePath =
            m_settingsDatabase->value(QStringLiteral("noteDBFilePath"), QString()).toString();
    if (noteDBFilePath.isEmpty()) {
        noteDBFilePath = defaultDBPath;
    }
    QFileInfo noteDBFilePathInf(noteDBFilePath);
    QFileInfo defaultDBPathInf(defaultDBPath);
    if ((!noteDBFilePathInf.exists()) && (defaultDBPathInf.exists())) {
        QDir().mkpath(noteDBFilePathInf.absolutePath());
        QFile defaultDBFile(defaultDBPath);
        defaultDBFile.rename(noteDBFilePath);
    }
    if (QFile::exists(noteDBFilePath) && needMigrateFromV1_5_0) {
        {
            auto m_db = QSqlDatabase::addDatabase("QSQLITE", DEFAULT_DATABASE_NAME);
            m_db.setDatabaseName(noteDBFilePath);
            if (m_db.open()) {
                QSqlQuery query(m_db);
                if (query.exec("SELECT name FROM sqlite_master WHERE type='table' AND "
                               "name='tag_table';")) {
                    if (query.next() && query.value(0).toString() == "tag_table") {
                        needMigrateFromV1_5_0 = false;
                    }
                }
                m_db.close();
            }
            m_db = QSqlDatabase::database();
        }
        QSqlDatabase::removeDatabase(DEFAULT_DATABASE_NAME);
    }
    if (!QFile::exists(noteDBFilePath)) {
        QFile noteDBFile(noteDBFilePath);
        if (!noteDBFile.open(QIODevice::WriteOnly))
            qFatal("ERROR : Can't create database file");

        noteDBFile.close();
        doCreate = true;
        needMigrateFromV1_5_0 = false;
    } else if (needMigrateFromV1_5_0) {
        QFile noteDBFile(noteDBFilePath);
        noteDBFile.rename(dir.path() + QDir::separator() + QStringLiteral("oldNotes.db"));
        noteDBFile.setFileName(noteDBFilePath);
        if (!noteDBFile.open(QIODevice::WriteOnly))
            qFatal("ERROR : Can't create database file");

        noteDBFile.close();
        doCreate = true;
    }

    if (needMigrateFromV1_5_0) {
        m_settingsDatabase->setValue(QStringLiteral("version"), qApp->applicationVersion());
    }
    m_dbManager = new DBManager;
    m_dbThread = new QThread;
    m_dbThread->setObjectName(QStringLiteral("dbThread"));
    m_dbManager->moveToThread(m_dbThread);
    connect(m_dbThread, &QThread::started, this, [=]() {
        setTheme(m_currentTheme);
        emit requestOpenDBManager(noteDBFilePath, doCreate);
        if (needMigrateFromV1_5_0) {
            emit requestMigrateNotesFromV1_5_0(dir.path() + QDir::separator()
                                               + QStringLiteral("oldNotes.db"));
        }
    });
    connect(this, &MainWindow::requestOpenDBManager, m_dbManager,
            &DBManager::onOpenDBManagerRequested, Qt::QueuedConnection);
    connect(this, &MainWindow::requestMigrateNotesFromV1_5_0, m_dbManager,
            &DBManager::onMigrateNotesFrom1_5_0Requested, Qt::QueuedConnection);
    connect(m_dbThread, &QThread::finished, m_dbManager, &QObject::deleteLater);
    m_dbThread->start();
}

/*!
 * \brief MainWindow::setupModelView
 */
void MainWindow::setupModelView()
{
    m_listView = ui->listView;
    m_tagPool = new TagPool(m_dbManager);
    m_listModel = new NoteListModel(m_listView);
    m_listView->setTagPool(m_tagPool);
    m_listView->setModel(m_listModel);
    m_listViewLogic = new ListViewLogic(m_listView, m_listModel, m_searchEdit, m_clearButton,
                                        m_tagPool, m_dbManager, this);
    m_treeView = static_cast<NodeTreeView *>(ui->treeView);
    m_treeView->setModel(m_treeModel);
    m_treeViewLogic = new TreeViewLogic(m_treeView, m_treeModel, m_dbManager, this);
    m_noteEditorLogic = new NoteEditorLogic(m_textEdit, m_editorDateLabel, m_searchEdit,
                                            static_cast<TagListView *>(ui->tagListView), m_tagPool,
                                            m_dbManager, this);
}

/*!
 * \brief MainWindow::restoreStates
 * Restore the latest states (if there are any) of the window and the splitter from
 * the settings database
 */
void MainWindow::restoreStates()
{
    setUseNativeWindowFrame(
            m_settingsDatabase->value(QStringLiteral("useNativeWindowFrame"), false).toBool());

    setHideToTray(m_settingsDatabase->value(QStringLiteral("hideToTray"), true).toBool());
    if (m_hideToTray) {
        setupTrayIcon();
    }

    if (m_settingsDatabase->value(QStringLiteral("windowGeometry"), "NULL") != "NULL")
        restoreGeometry(m_settingsDatabase->value(QStringLiteral("windowGeometry")).toByteArray());

    if (m_settingsDatabase->value(QStringLiteral("editorSettingsWindowGeometry"), "NULL") != "NULL")
        m_styleEditorWindow.restoreGeometry(
                m_settingsDatabase->value(QStringLiteral("editorSettingsWindowGeometry"))
                        .toByteArray());

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    // Set margin to zero if the window is maximized
    if (isMaximized()) {
        setMargins(QMargins());
    }
#endif

#if defined(UPDATE_CHECKER)
    if (m_settingsDatabase->value(QStringLiteral("dontShowUpdateWindow"), "NULL") != "NULL")
        m_dontShowUpdateWindow =
                m_settingsDatabase->value(QStringLiteral("dontShowUpdateWindow")).toBool();
#endif

    m_splitter->setCollapsible(0, true);
    m_splitter->setCollapsible(1, true);
    m_splitter->resize(width() - m_layoutMargin, height() - m_layoutMargin);

    if (m_settingsDatabase->value(QStringLiteral("splitterSizes"), "NULL") != "NULL")
        m_splitter->restoreState(
                m_settingsDatabase->value(QStringLiteral("splitterSizes")).toByteArray());

    m_foldersWidget->setHidden(
            m_settingsDatabase->value(QStringLiteral("isTreeCollapsed")).toBool());
    m_noteListWidget->setHidden(
            m_settingsDatabase->value(QStringLiteral("isNoteListCollapsed")).toBool());

    m_splitter->setCollapsible(0, false);
    m_splitter->setCollapsible(1, false);

    QString selectedFontTypefaceFromDatabase =
            m_settingsDatabase->value(QStringLiteral("selectedFontTypeface"), "NULL").toString();
    if (selectedFontTypefaceFromDatabase != "NULL") {
        if (selectedFontTypefaceFromDatabase == "Mono") {
            m_currentFontTypeface = FontTypeface::Mono;
        } else if (selectedFontTypefaceFromDatabase == "Serif") {
            m_currentFontTypeface = FontTypeface::Serif;
        } else if (selectedFontTypefaceFromDatabase == "SansSerif") {
            m_currentFontTypeface = FontTypeface::SansSerif;
        }
    }

    if (m_settingsDatabase->value(QStringLiteral("editorMediumFontSize"), "NULL") != "NULL") {
        m_editorMediumFontSize =
                m_settingsDatabase->value(QStringLiteral("editorMediumFontSize")).toInt();
    } else {
#ifdef __APPLE__
        m_editorMediumFontSize = 17;
#else
        m_editorMediumFontSize = 13;
#endif
    }
    m_currentFontPointSize = m_editorMediumFontSize;

    bool isTextFullWidth = false;
    if (m_settingsDatabase->value(QStringLiteral("isTextFullWidth"), "NULL") != "NULL") {
        isTextFullWidth = m_settingsDatabase->value(QStringLiteral("isTextFullWidth")).toBool();
        if (isTextFullWidth) {
            m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
        } else {
            m_textEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
        }
    } else {
        m_textEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
    }

    if (m_settingsDatabase->value(QStringLiteral("charsLimitPerFontMono"), "NULL") != "NULL")
        m_currentCharsLimitPerFont.mono =
                m_settingsDatabase->value(QStringLiteral("charsLimitPerFontMono")).toInt();
    if (m_settingsDatabase->value(QStringLiteral("charsLimitPerFontSerif"), "NULL") != "NULL")
        m_currentCharsLimitPerFont.serif =
                m_settingsDatabase->value(QStringLiteral("charsLimitPerFontSerif")).toInt();
    if (m_settingsDatabase->value(QStringLiteral("charsLimitPerFontSansSerif"), "NULL") != "NULL")
        m_currentCharsLimitPerFont.sansSerif =
                m_settingsDatabase->value(QStringLiteral("charsLimitPerFontSansSerif")).toInt();

    if (m_settingsDatabase->value(QStringLiteral("chosenMonoFont"), "NULL") != "NULL") {
        QString fontName = m_settingsDatabase->value(QStringLiteral("chosenMonoFont")).toString();
        int fontIndex = m_listOfMonoFonts.indexOf(fontName);
        if (fontIndex != -1) {
            m_chosenMonoFontIndex = fontIndex;
        }
    }
    if (m_settingsDatabase->value(QStringLiteral("chosenSerifFont"), "NULL") != "NULL") {
        QString fontName = m_settingsDatabase->value(QStringLiteral("chosenSerifFont")).toString();
        int fontIndex = m_listOfSerifFonts.indexOf(fontName);
        if (fontIndex != -1) {
            m_chosenSerifFontIndex = fontIndex;
        }
    }
    if (m_settingsDatabase->value(QStringLiteral("chosenSansSerifFont"), "NULL") != "NULL") {
        QString fontName =
                m_settingsDatabase->value(QStringLiteral("chosenSansSerifFont")).toString();
        int fontIndex = m_listOfSansSerifFonts.indexOf(fontName);
        if (fontIndex != -1) {
            m_chosenSansSerifFontIndex = fontIndex;
        }
    }

    if (m_settingsDatabase->value(QStringLiteral("theme"), "NULL") != "NULL") {
        QString chosenTheme = m_settingsDatabase->value(QStringLiteral("theme")).toString();
        if (chosenTheme == "Light") {
            m_currentTheme = Theme::Light;
        } else if (chosenTheme == "Dark") {
            m_currentTheme = Theme::Dark;
        } else if (chosenTheme == "Sepia") {
            m_currentTheme = Theme::Sepia;
        }
    }

    m_styleEditorWindow.changeSelectedFont(FontTypeface::Mono,
                                           m_listOfMonoFonts.at(m_chosenMonoFontIndex));
    m_styleEditorWindow.changeSelectedFont(FontTypeface::Serif,
                                           m_listOfSerifFonts.at(m_chosenSerifFontIndex));
    m_styleEditorWindow.changeSelectedFont(FontTypeface::SansSerif,
                                           m_listOfSansSerifFonts.at(m_chosenSansSerifFontIndex));

    setCurrentFontBasedOnTypeface(m_currentFontTypeface);
    setTheme(m_currentTheme);
    m_styleEditorWindow.restoreSelectedOptions(isTextFullWidth, m_currentFontTypeface,
                                               m_currentTheme);

    auto expandedFolder =
            m_settingsDatabase->value(QStringLiteral("currentExpandedFolder"), QStringList{})
                    .toStringList();
    auto isSelectingFolder =
            m_settingsDatabase->value(QStringLiteral("isSelectingFolder"), true).toBool();
    auto currentSelectFolder =
            m_settingsDatabase->value(QStringLiteral("currentSelectFolder"), QString{}).toString();
    auto currentSelectTagsId =
            m_settingsDatabase->value(QStringLiteral("currentSelectTagsId"), QStringList{})
                    .toStringList();
    QSet<int> tags;
    for (const auto &tagId : qAsConst(currentSelectTagsId)) {
        tags.insert(tagId.toInt());
    }
    m_treeViewLogic->setLastSavedState(isSelectingFolder, currentSelectFolder, tags,
                                       expandedFolder);
    auto currentSelectNotes =
            m_settingsDatabase->value(QStringLiteral("currentSelectNotesId"), QStringList{})
                    .toStringList();
    QSet<int> notesId;
    for (const auto &id : qAsConst(currentSelectNotes)) {
        notesId.insert(id.toInt());
    }
    m_listViewLogic->setLastSavedState(notesId);
}

/*!
 * \brief MainWindow::setButtonsAndFieldsEnabled
 * \param doEnable
 */
void MainWindow::setButtonsAndFieldsEnabled(bool doEnable)
{
    m_greenMaximizeButton->setEnabled(doEnable);
    m_redCloseButton->setEnabled(doEnable);
    m_yellowMinimizeButton->setEnabled(doEnable);
    m_newNoteButton->setEnabled(doEnable);
    m_trashButton->setEnabled(doEnable);
    m_searchEdit->setEnabled(doEnable);
    m_textEdit->setEnabled(doEnable);
    m_dotsButton->setEnabled(doEnable);
    m_styleEditorButton->setEnabled(doEnable);
}

/*!
 * \brief MainWindow::resetBlockFormat
 * Removes applied formmatting: bold, italic, strikethrough, heading of the current line
 * or selected text (selected text has to include the formatting chars)
 */
void MainWindow::resetBlockFormat()
{
    QTextCursor cursor = m_textEdit->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::BlockUnderCursor);
        if (!cursor.hasSelection()) {
            return;
        }
    }
    QString selectedText = cursor.selectedText();
    selectedText.remove(QRegularExpression("`|\\*|~|#"));
    cursor.insertText(selectedText);
}

/*!
 * \brief MainWindow::resetFormat
 * \param formatChars
 * Removes applied formmatting: bold, italic, strikethrough
 */
void MainWindow::resetFormat(const QString &formatChars)
{
    QTextCursor cursor = m_textEdit->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
        if (!cursor.hasSelection()) {
            for (int i = 0; i < 2; ++i) {
                QTextDocument *doc = m_textEdit->document();
                cursor = doc->find(QRegularExpression(QRegularExpression::escape(formatChars)),
                                   cursor.selectionStart(), QTextDocument::FindBackward);
                if (!cursor.isNull()) {
                    cursor.deleteChar();
                }
            }
            return;
        }
    }
    QString selectedText = cursor.selectedText();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    if (selectedText.startsWith(formatChars) && selectedText.endsWith(formatChars)) {
        if (selectedText.length() == formatChars.length()) {
            return;
        }
        start += formatChars.length();
        end -= formatChars.length();
    } else if (selectedText.startsWith(formatChars) || selectedText.endsWith(formatChars)) {
        return;
    }
    cursor.beginEditBlock();
    cursor.setPosition(start - formatChars.length(), QTextCursor::MoveAnchor);
    cursor.setPosition(start, QTextCursor::KeepAnchor);
    cursor.deleteChar();
    cursor.setPosition(end - formatChars.length(), QTextCursor::MoveAnchor);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    cursor.deleteChar();
    cursor.endEditBlock();
}

/*!
 * \brief MainWindow::onNewNoteButtonPressed
 * When the new-note button is pressed, set it's icon accordingly
 */
void MainWindow::onNewNoteButtonPressed()
{
    m_newNoteButton->setIcon(QIcon(QStringLiteral(":/images/newNote_Pressed.png")));
}

/*!
 * \brief MainWindow::onNewNoteButtonClicked
 * Create a new note when clicking the 'new note' button
 */
void MainWindow::onNewNoteButtonClicked()
{
    if (!m_newNoteButton->isVisible()) {
        return;
    }
    if (m_listViewLogic->isAnimationRunning()) {
        return;
    }
    m_newNoteButton->setIcon(QIcon(QStringLiteral(":/images/newNote_Regular.png")));

    // save the data of the previous selected
    m_noteEditorLogic->saveNoteToDB();

    if (!m_searchEdit->text().isEmpty()) {
        m_listViewLogic->clearSearch(true);
    } else {
        createNewNote();
    }
}

/*!
 * \brief MainWindow::onTrashButtonPressed
 * When the trash button is pressed, set it's icon accordingly
 */
void MainWindow::onTrashButtonPressed()
{
    m_trashButton->setIcon(QIcon(QStringLiteral(":/images/trashCan_Pressed.png")));
}

/*!
 * \brief MainWindow::onTrashButtonClicked
 * Delete selected note when clicking the 'delete note' button
 */
void MainWindow::onTrashButtonClicked()
{
    m_trashButton->setIcon(QIcon(QStringLiteral(":/images/trashCan_Regular.png")));

    m_trashButton->blockSignals(true);
    m_noteEditorLogic->deleteCurrentNote();
    m_trashButton->blockSignals(false);
}

/*!
 * \brief MainWindow::onDotsButtonPressed
 * When the 3 dots button is pressed, set it's icon accordingly
 */
void MainWindow::onDotsButtonPressed()
{
    m_dotsButton->setIcon(QIcon(QStringLiteral(":/images/3dots_Pressed.png")));
}

/*!
 * \brief MainWindow::onDotsButtonClicked
 * Open up the menu when clicking the 3 dots button
 */
void MainWindow::onDotsButtonClicked()
{
    m_dotsButton->setIcon(QIcon(QStringLiteral(":/images/3dots_Regular.png")));

    QMenu mainMenu;
    QMenu *viewMenu = mainMenu.addMenu(tr("&View"));
    QMenu *importExportNotesMenu = mainMenu.addMenu(tr("&Import/Export Notes"));
    importExportNotesMenu->setToolTipsVisible(true);
    viewMenu->setToolTipsVisible(true);
    mainMenu.setToolTipsVisible(true);

    QShortcut *closeMenu = new QShortcut(Qt::Key_F10, &mainMenu);
    closeMenu->setContext(Qt::ApplicationShortcut);
    connect(closeMenu, &QShortcut::activated, &mainMenu, &QMenu::close);

#if defined(Q_OS_WINDOWS) || defined(Q_OS_WIN)
    mainMenu.setStyleSheet(QStringLiteral("QMenu { "
                                          "  background-color: rgb(255, 255, 255); "
                                          "  border: 1px solid #C7C7C7; "
                                          "  }"
                                          "QMenu::item:selected { "
                                          "  background: 1px solid #308CC6; "
                                          "}"));
#endif

#ifdef __APPLE__
    mainMenu.setFont(QFont(m_displayFont, 13));
    viewMenu->setFont(QFont(m_displayFont, 13));
    importExportNotesMenu->setFont(QFont(m_displayFont, 13));
#else
    mainMenu.setFont(QFont(m_displayFont, 10, QFont::Normal));
    viewMenu->setFont(QFont(m_displayFont, 10, QFont::Normal));
    importExportNotesMenu->setFont(QFont(m_displayFont, 10, QFont::Normal));
#endif

    // note list visiblity action
    bool isNoteListCollapsed = (m_noteListWidget->isHidden());
    QString actionLabel = isNoteListCollapsed ? tr("Show &notes list") : tr("Hide &notes list");

    QAction *noteListVisbilityAction = viewMenu->addAction(actionLabel);
    noteListVisbilityAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_J));
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    noteListVisbilityAction->setShortcutVisibleInContextMenu(true);
#endif
    if (isNoteListCollapsed) {
        connect(noteListVisbilityAction, &QAction::triggered, this, &MainWindow::expandNoteList);
    } else {
        connect(noteListVisbilityAction, &QAction::triggered, this, &MainWindow::collapseNoteList);
    }

    // folder tree view visiblity action
    bool isFolderTreeCollapsed = (m_foldersWidget->isHidden());
    QString factionLabel =
            isFolderTreeCollapsed ? tr("Show &folders tree") : tr("Hide &folders tree");

    QAction *folderTreeVisibilityAction = viewMenu->addAction(factionLabel);
    folderTreeVisibilityAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_J));
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    folderTreeVisibilityAction->setShortcutVisibleInContextMenu(true);
#endif
    if (isFolderTreeCollapsed) {
        connect(folderTreeVisibilityAction, &QAction::triggered, this,
                &MainWindow::expandFolderTree);
    } else {
        connect(folderTreeVisibilityAction, &QAction::triggered, this,
                &MainWindow::collapseFolderTree);
    }

    // Enable or Disable markdown
    QString markDownLabel =
            m_noteEditorLogic->markdownEnabled() ? tr("Disable &Markdown") : tr("Enable &Markdown");

    QAction *noteMarkdownVisibiltyAction = viewMenu->addAction(markDownLabel);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    noteMarkdownVisibiltyAction->setShortcutVisibleInContextMenu(true);
#endif

    connect(noteMarkdownVisibiltyAction, &QAction::triggered, m_noteEditorLogic, [this] {
        m_noteEditorLogic->setMarkdownEnabled(!m_noteEditorLogic->markdownEnabled());
    });

#if defined(UPDATE_CHECKER)
    // Check for update action
    QAction *checkForUpdatesAction = mainMenu.addAction(tr("Check For &Updates"));
    connect(checkForUpdatesAction, &QAction::triggered, this, &MainWindow::checkForUpdates);
#endif

    // Autostart
    QAction *autostartAction = mainMenu.addAction(tr("&Start automatically"));
    connect(autostartAction, &QAction::triggered, this,
            [=]() { m_autostart.setAutostart(autostartAction->isChecked()); });
    autostartAction->setCheckable(true);
    autostartAction->setChecked(m_autostart.isAutostart());

    // hide to tray
    QAction *hideToTrayAction = mainMenu.addAction(tr("&Hide to tray"));
    connect(hideToTrayAction, &QAction::triggered, this, [=]() {
        m_settingsDatabase->setValue(QStringLiteral("hideToTray"), hideToTrayAction->isChecked());
    });
    hideToTrayAction->setCheckable(true);
    hideToTrayAction->setChecked(m_hideToTray);
    connect(hideToTrayAction, &QAction::triggered, this, [this]() {
        setHideToTray(!m_hideToTray);
        if (m_hideToTray) {
            setupTrayIcon();
        } else {
            m_trayIcon->hide();
        }
    });

    QAction *changeDBPathAction = mainMenu.addAction(tr("&Change database path"));
    connect(changeDBPathAction, &QAction::triggered, this, [=]() {
        auto btn = QMessageBox::question(this, "Are you sure you want to change the database path?",
                                         "Are you sure you want to change the database path?");
        if (btn == QMessageBox::Yes) {
            auto newDbPath = QFileDialog::getSaveFileName(this, "New Database path", "notes.db");
            if (!newDbPath.isEmpty()) {
                m_settingsDatabase->setValue(QStringLiteral("noteDBFilePath"), newDbPath);
                QFileInfo noteDBFilePathInf(newDbPath);
                QDir().mkpath(noteDBFilePathInf.absolutePath());
                emit requestChangeDatabasePath(newDbPath);
            }
        }
    });

    // About Notes
    QAction *aboutAction = mainMenu.addAction(tr("&About Notes"));
    connect(aboutAction, &QAction::triggered, this, [&]() { m_aboutWindow.show(); });

    mainMenu.addSeparator();

    // Close the app
    QAction *quitAppAction = mainMenu.addAction(tr("&Quit"));
    connect(quitAppAction, &QAction::triggered, this, &MainWindow::QuitApplication);

    // Export notes action
    QAction *exportNotesFileAction = importExportNotesMenu->addAction(tr("&Export"));
    exportNotesFileAction->setToolTip(tr("Save notes to a file"));
    connect(exportNotesFileAction, &QAction::triggered, this, &MainWindow::exportNotesFile);

    // Import notes action
    QAction *importNotesFileAction = importExportNotesMenu->addAction(tr("&Import"));
    importNotesFileAction->setToolTip(tr("Add notes from a file"));
    connect(importNotesFileAction, &QAction::triggered, this, &MainWindow::importNotesFile);

    // Restore notes action
    QAction *restoreNotesFileAction = importExportNotesMenu->addAction(tr("&Restore"));
    restoreNotesFileAction->setToolTip(tr("Replace all notes with notes from a file"));
    connect(restoreNotesFileAction, &QAction::triggered, this, &MainWindow::restoreNotesFile);

#if defined(Q_OS_MACOS) || defined(Q_OS_WINDOWS)
    // Stay on top action
    QAction *stayOnTopAction = viewMenu->addAction(tr("Always stay on top"));
    stayOnTopAction->setToolTip(tr("Always keep the notes application on top of all windows"));
    stayOnTopAction->setCheckable(true);
    stayOnTopAction->setChecked(m_alwaysStayOnTop);
    connect(stayOnTopAction, &QAction::triggered, this, &MainWindow::toggleStayOnTop);
#endif

#ifndef __APPLE__
    // Use native frame action
    QAction *useNativeFrameAction = viewMenu->addAction(tr("&Use native window frame"));
    useNativeFrameAction->setToolTip(tr("Use the window frame provided by the window manager"));
    useNativeFrameAction->setCheckable(true);
    useNativeFrameAction->setChecked(m_useNativeWindowFrame);
    connect(useNativeFrameAction, &QAction::triggered, this,
            [this]() { setUseNativeWindowFrame(!m_useNativeWindowFrame); });
#endif

    mainMenu.exec(m_dotsButton->mapToGlobal(QPoint(0, m_dotsButton->height())));
}

/*!
 * \brief MainWindow::onStyleEditorButtonPressed
 * When the Style Editor Button is pressed, set it's icon accordingly
 */
void MainWindow::onStyleEditorButtonPressed()
{
    QString ss = QStringLiteral("QPushButton { "
                                "  border: none; "
                                "  padding: 0px; "
                                "  color: rgb(39, 85, 125);"
                                "}");
    m_styleEditorButton->setStyleSheet(ss);
}

/*!
 * \brief MainWindow::onStyleEditorButtonClicked
 * Open up the editor's styling menu when clicking the Style Editor Button
 */
void MainWindow::onStyleEditorButtonClicked()
{
    QString ss = QStringLiteral("QPushButton { "
                                "  border: none; "
                                "  padding: 0px; "
                                "  color: rgb(68, 138, 201);"
                                "}");
    m_styleEditorButton->setStyleSheet(ss);

    if (m_settingsDatabase->value(QStringLiteral("editorSettingsWindowGeometry"), "NULL") == "NULL")
        m_styleEditorWindow.move(m_newNoteButton->mapToGlobal(
                QPoint(-m_styleEditorWindow.width() - m_newNoteButton->width(),
                       m_newNoteButton->height())));

    if (m_styleEditorWindow.isVisible()) {
        m_styleEditorWindow.hide();
    } else {
        m_styleEditorWindow.show();
        m_styleEditorWindow.setFocus();
    }
}

/*!
 * \brief MainWindow::changeEditorFontTypeFromStyleButtons
 * Change the font based on the type passed from the Style Editor Window
 */
void MainWindow::changeEditorFontTypeFromStyleButtons(FontTypeface fontTypeface)
{
    if (m_currentFontTypeface == fontTypeface) {
        switch (fontTypeface) {
        case FontTypeface::Mono:
            m_chosenMonoFontIndex = m_chosenMonoFontIndex < m_listOfMonoFonts.length() - 1
                    ? m_chosenMonoFontIndex + 1
                    : 0;
            break;
        case FontTypeface::Serif:
            m_chosenSerifFontIndex = m_chosenSerifFontIndex < m_listOfSerifFonts.length() - 1
                    ? m_chosenSerifFontIndex + 1
                    : 0;
            break;
        case FontTypeface::SansSerif:
            m_chosenSansSerifFontIndex =
                    m_chosenSansSerifFontIndex < m_listOfSansSerifFonts.length() - 1
                    ? m_chosenSansSerifFontIndex + 1
                    : 0;
            break;
        }
    }

    setCurrentFontBasedOnTypeface(fontTypeface);

    m_styleEditorWindow.changeSelectedFont(fontTypeface, m_currentFontFamily);
}

/*!
 * \brief MainWindow::changeEditorFontSizeFromStyleButtons
 * Change the font size based on the button pressed in the Style Editor Window
 * Increase / Decrease
 */
void MainWindow::changeEditorFontSizeFromStyleButtons(FontSizeAction fontSizeAction)
{
    switch (fontSizeAction) {
    case FontSizeAction::Increase:
        m_editorMediumFontSize += 1;
        setCurrentFontBasedOnTypeface(m_currentFontTypeface);
        break;
    case FontSizeAction::Decrease:
        m_editorMediumFontSize -= 1;
        setCurrentFontBasedOnTypeface(m_currentFontTypeface);
        break;
    }
}

/*!
 * \brief MainWindow::changeEditorTextWidthFromStyleButtons
 * Change the text width of the text editor
 * FullWidth / Increase / Decrease
 */
void MainWindow::changeEditorTextWidthFromStyleButtons(EditorTextWidth editorTextWidth)
{
    switch (editorTextWidth) {
    case EditorTextWidth::FullWidth:
        if (m_textEdit->lineWrapMode() != QTextEdit::WidgetWidth)
            m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
        else
            m_textEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
        break;
    case EditorTextWidth::Increase:
        m_textEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
        switch (m_currentFontTypeface) {
        case FontTypeface::Mono:
            m_currentCharsLimitPerFont.mono = m_currentCharsLimitPerFont.mono + 1;
            break;
        case FontTypeface::Serif:
            m_currentCharsLimitPerFont.serif = m_currentCharsLimitPerFont.serif + 1;
            break;
        case FontTypeface::SansSerif:
            m_currentCharsLimitPerFont.sansSerif = m_currentCharsLimitPerFont.sansSerif + 1;
            break;
        }
        break;
    case EditorTextWidth::Decrease:
        m_textEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
        switch (m_currentFontTypeface) {
        case FontTypeface::Mono:
            m_currentCharsLimitPerFont.mono -= 1;
            break;
        case FontTypeface::Serif:
            m_currentCharsLimitPerFont.serif -= 1;
            break;
        case FontTypeface::SansSerif:
            m_currentCharsLimitPerFont.sansSerif -= 1;
            break;
        }
        break;
    }

    setCurrentFontBasedOnTypeface(m_currentFontTypeface);
}

/*!
 * \brief MainWindow::resetEditorToDefaultSettings
 * Reset text editor to default settings
 */
void MainWindow::resetEditorToDefaultSettings()
{
    resetEditorSettings();
}

/*!
 * \brief MainWindow::setTheme
 * Changes the app theme
 */
void MainWindow::setTheme(Theme theme)
{
    m_currentTheme = theme;
    switch (theme) {
    case Theme::Light: {
        m_currentThemeBackgroundColor = QColor(247, 247, 247);
        m_currentEditorTextColor = QColor(26, 26, 26);
        m_currentEditorBackgroundColor = m_currentThemeBackgroundColor;
        m_currentRightFrameColor = m_currentThemeBackgroundColor;
        setStyleSheet(QStringLiteral("QMainWindow { background-color: rgb(247, 247, 247);}"));
        ui->verticalSpacer_upSearchEdit->setStyleSheet(
                QStringLiteral("QWidget{ background-color: %1;}")
                        .arg(m_currentThemeBackgroundColor.name()));
        ui->verticalSpacer_upSearchEdit2->setStyleSheet(
                QStringLiteral("QWidget{ background-color: %1;}")
                        .arg(m_currentThemeBackgroundColor.name()));
        ui->verticalSpacer_upTreeView->setStyleSheet(
                QStringLiteral("QWidget{ background-color: %1;}")
                        .arg(m_currentThemeBackgroundColor.name()));
        setupTextEditStyleSheet(m_noteEditorLogic->currentMinimumEditorPadding(),
                                m_noteEditorLogic->currentMinimumEditorPadding());
        m_listViewLogic->setTheme(Theme::Light);
        m_styleEditorWindow.setTheme(Theme::Light, m_currentThemeBackgroundColor,
                                     m_currentEditorTextColor);
        m_aboutWindow.setTheme(m_currentThemeBackgroundColor, m_currentEditorTextColor);
        ui->listviewLabel1->setStyleSheet(
                QStringLiteral("QLabel { color : %1; }").arg(QColor(26, 26, 26).name()));
        m_treeViewLogic->setTheme(theme);
        break;
    }
    case Theme::Dark: {
        m_currentThemeBackgroundColor = QColor(30, 30, 30);
        m_currentEditorTextColor = QColor(204, 204, 204);
        m_currentEditorBackgroundColor = m_currentThemeBackgroundColor;
        m_currentRightFrameColor = m_currentThemeBackgroundColor;
        setStyleSheet(QStringLiteral("QMainWindow { background-color: rgb(26, 26, 26); }"));
        ui->verticalSpacer_upSearchEdit->setStyleSheet(
                QStringLiteral("QWidget{ background-color: %1;}")
                        .arg(m_currentThemeBackgroundColor.name()));
        ui->verticalSpacer_upSearchEdit2->setStyleSheet(
                QStringLiteral("QWidget{ background-color: %1;}")
                        .arg(m_currentThemeBackgroundColor.name()));
        ui->verticalSpacer_upTreeView->setStyleSheet(
                QStringLiteral("QWidget{ background-color: %1;}")
                        .arg(m_currentThemeBackgroundColor.name()));
        setupTextEditStyleSheet(m_noteEditorLogic->currentMinimumEditorPadding(),
                                m_noteEditorLogic->currentMinimumEditorPadding());
        m_listViewLogic->setTheme(Theme::Dark);
        m_styleEditorWindow.setTheme(Theme::Dark, m_currentThemeBackgroundColor,
                                     m_currentEditorTextColor);
        m_aboutWindow.setTheme(m_currentThemeBackgroundColor, m_currentEditorTextColor);
        ui->listviewLabel1->setStyleSheet(
                QStringLiteral("QLabel { color : %1; }").arg(QColor(204, 204, 204).name()));
        m_treeViewLogic->setTheme(theme);
        break;
    }
    case Theme::Sepia: {
        m_currentThemeBackgroundColor = QColor(251, 240, 217);
        m_currentEditorTextColor = QColor(95, 74, 50);
        m_currentEditorBackgroundColor = m_currentThemeBackgroundColor;
        m_currentRightFrameColor = m_currentThemeBackgroundColor;
        setStyleSheet(QStringLiteral("QMainWindow { background-color: rgb(251, 240, 217); }"));
        ui->verticalSpacer_upSearchEdit->setStyleSheet(
                QStringLiteral("QWidget{ background-color: %1;}")
                        .arg(m_currentThemeBackgroundColor.name()));
        ui->verticalSpacer_upSearchEdit2->setStyleSheet(
                QStringLiteral("QWidget{ background-color: %1;}")
                        .arg(m_currentThemeBackgroundColor.name()));
        ui->verticalSpacer_upTreeView->setStyleSheet(
                QStringLiteral("QWidget{ background-color: %1;}")
                        .arg(m_currentThemeBackgroundColor.name()));
        setupTextEditStyleSheet(m_noteEditorLogic->currentMinimumEditorPadding(),
                                m_noteEditorLogic->currentMinimumEditorPadding());
        m_listViewLogic->setTheme(Theme::Sepia);
        m_styleEditorWindow.setTheme(Theme::Sepia, m_currentThemeBackgroundColor,
                                     QColor(26, 26, 26));
        m_aboutWindow.setTheme(m_currentThemeBackgroundColor, QColor(26, 26, 26));
        ui->listviewLabel1->setStyleSheet(
                QStringLiteral("QLabel { color : %1; }").arg(QColor(26, 26, 26).name()));
        m_treeViewLogic->setTheme(theme);
        break;
    }
    }
    ui->tagListView->setBackground(m_currentThemeBackgroundColor);
    m_noteEditorLogic->setTheme(theme, m_currentEditorTextColor);

    setSearchEditStyleSheet(false);
    alignTextEditText();
    setupRightFrame();
}

void MainWindow::deleteSelectedNote()
{
    m_noteEditorLogic->deleteCurrentNote();
}

/*!
 * \brief MainWindow::onClearButtonClicked
 * clears the search and
 * select the note that was selected before searching if it is still valid.
 */
void MainWindow::onClearButtonClicked()
{
    m_listViewLogic->clearSearch();
}

/*!
 * \brief MainWindow::createNewNote
 * create a new note
 * add it to the database
 * add it to the scrollArea
 */
void MainWindow::createNewNote()
{
    m_listView->scrollToTop();
    QModelIndex newNoteIndex;
    if (!m_noteEditorLogic->isTempNote()) {
        // clear the textEdit
        m_noteEditorLogic->closeEditor();

        NodeData tmpNote;
        tmpNote.setNodeType(NodeData::Note);
        QDateTime noteDate = QDateTime::currentDateTime();
        tmpNote.setCreationDateTime(noteDate);
        tmpNote.setLastModificationDateTime(noteDate);
        tmpNote.setFullTitle(QStringLiteral("New Note"));
        auto inf = m_listViewLogic->listViewInfo();
        if ((!inf.isInTag) && (inf.parentFolderId > SpecialNodeID::RootFolder)) {
            NodeData parent;
            QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                      Q_RETURN_ARG(NodeData, parent),
                                      Q_ARG(int, inf.parentFolderId));
            if (parent.nodeType() == NodeData::Folder) {
                tmpNote.setParentId(parent.id());
                tmpNote.setParentName(parent.fullTitle());
            } else {
                tmpNote.setParentId(SpecialNodeID::DefaultNotesFolder);
                tmpNote.setParentName("Notes");
            }
        } else {
            tmpNote.setParentId(SpecialNodeID::DefaultNotesFolder);
            tmpNote.setParentName("Notes");
        }
        int noteId = SpecialNodeID::InvalidNodeId;
        QMetaObject::invokeMethod(m_dbManager, "nextAvailableNodeId", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(int, noteId));
        tmpNote.setId(noteId);
        tmpNote.setIsTempNote(true);
        if (inf.isInTag) {
            tmpNote.setTagIds(inf.currentTagList);
        }
        // insert the new note to NoteListModel
        newNoteIndex = m_listModel->insertNote(tmpNote, 0);

        // update the editor
        m_noteEditorLogic->showNotesInEditor({ tmpNote });
    } else {
        newNoteIndex = m_listModel->getNoteIndex(m_noteEditorLogic->currentEditingNoteId());
        m_listView->animateAddedRow({ newNoteIndex });
    }
    // update the current selected index
    m_listView->setCurrentIndexC(newNoteIndex);
    m_textEdit->setFocus();
}

void MainWindow::selectNoteDown()
{
    m_listViewLogic->selectNoteDown();
}

/*!
 * \brief MainWindow::setFocusOnText
 * Set focus on textEdit
 */
void MainWindow::setFocusOnText()
{
    if (m_noteEditorLogic->currentEditingNoteId() != SpecialNodeID::InvalidNodeId
        && !m_textEdit->hasFocus()) {
        m_listView->setCurrentRowActive(true);
        m_textEdit->setFocus();
    }
}

void MainWindow::selectNoteUp()
{
    m_listViewLogic->selectNoteUp();
}

/*!
 * \brief MainWindow::fullscreenWindow
 * Switch to fullscreen mode
 */
void MainWindow::fullscreenWindow()
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    if (isFullScreen()) {
        if (!isMaximized()) {
            QMargins margins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin);
            setMargins(margins);
        }

        setWindowState(windowState() & ~Qt::WindowFullScreen);
    } else {
        setWindowState(windowState() | Qt::WindowFullScreen);
        setMargins(QMargins());
    }

#elif _WIN32
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
#endif
}

void MainWindow::makeCode()
{
    applyFormat("`");
}

void MainWindow::makeBold()
{
    applyFormat("**");
}

void MainWindow::makeItalic()
{
    applyFormat("*");
}

void MainWindow::makeStrikethrough()
{
    applyFormat("~");
}

/*!
 * \brief MainWindow::applyFormat
 * Make selected text bold, italic, or strikethrough it, by inserting the passed formatting char(s)
 * before and after the selection. If nothing is selected, insert formating char(s) before/after the
 * word under the cursor
 */
void MainWindow::applyFormat(const QString &formatChars)
{
    if (alreadyAppliedFormat(formatChars)) {
        resetFormat(formatChars);
        return;
    }

    QTextCursor cursor = m_textEdit->textCursor();
    bool selected = cursor.hasSelection();
    bool wordUnderCursor = false;
    if (!selected) {
        cursor.select(QTextCursor::WordUnderCursor);
        wordUnderCursor = cursor.hasSelection();
    }
    QString selectedText = cursor.selectedText();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    cursor.setPosition(start, QTextCursor::MoveAnchor);
    cursor.beginEditBlock();
    cursor.insertText(formatChars);
    cursor.setPosition(end + formatChars.length(), QTextCursor::MoveAnchor);
    cursor.insertText(formatChars);
    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor,
                        formatChars.length());
    cursor.endEditBlock();
    if (selected) {
        QTextDocument *doc = m_textEdit->document();
        QTextCursor found = doc->find(selectedText, start);
        m_textEdit->setTextCursor(found);
    } else if (!wordUnderCursor) {
        for (int i = 0; i < formatChars.length(); ++i) {
            m_textEdit->moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
        }
    }
}

/*!
 * \brief MainWindow::maximizeWindow
 * Maximize the window
 */
void MainWindow::maximizeWindow()
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    if (isMaximized()) {
        if (!isFullScreen()) {
            QMargins margins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin);

            setMargins(margins);
            setWindowState(windowState() & ~Qt::WindowMaximized);
        } else {
            setWindowState(windowState() & ~Qt::WindowFullScreen);
        }

    } else {
        setWindowState(windowState() | Qt::WindowMaximized);
        setMargins(QMargins());
    }
#elif _WIN32
    if (isMaximized()) {
        setWindowState(windowState() & ~Qt::WindowMaximized);
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    } else if (isFullScreen()) {
        setWindowState((windowState() | Qt::WindowMaximized) & ~Qt::WindowFullScreen);
        setGeometry(qApp->primaryScreen()->availableGeometry());
    } else {
        setWindowState(windowState() | Qt::WindowMaximized);
        setGeometry(qApp->primaryScreen()->availableGeometry());
    }
#endif
}

/*!
 * \brief MainWindow::minimizeWindow
 * Minimize the window
 */
void MainWindow::minimizeWindow()
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    QMargins margins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin);
    setMargins(margins);
#endif

    // BUG : QTBUG-57902 minimize doesn't store the window state before minimizing
    showMinimized();
}

/*!
 * \brief MainWindow::QuitApplication
 * Exit the application
 * Save the geometry of the app to the settings
 * Save the current note if it's note temporary one otherwise remove it from DB
 */
void MainWindow::QuitApplication()
{
    if (windowState() != Qt::WindowFullScreen) {
        m_settingsDatabase->setValue(QStringLiteral("windowGeometry"), saveGeometry());
        if (m_styleEditorWindow.windowState() != Qt::WindowFullScreen)
            m_settingsDatabase->setValue(QStringLiteral("editorSettingsWindowGeometry"),
                                         m_styleEditorWindow.saveGeometry());
    }

    m_noteEditorLogic->saveNoteToDB();

#if defined(UPDATE_CHECKER)
    m_settingsDatabase->setValue(QStringLiteral("dontShowUpdateWindow"), m_dontShowUpdateWindow);
#endif
    m_settingsDatabase->setValue(QStringLiteral("splitterSizes"), m_splitter->saveState());

    m_settingsDatabase->setValue(QStringLiteral("isTreeCollapsed"), m_foldersWidget->isHidden());
    m_settingsDatabase->setValue(QStringLiteral("isNoteListCollapsed"),
                                 m_noteListWidget->isHidden());

    QString currentFontTypefaceString;
    switch (m_currentFontTypeface) {
    case FontTypeface::Mono:
        currentFontTypefaceString = "Mono";
        break;
    case FontTypeface::Serif:
        currentFontTypefaceString = "Serif";
        break;
    case FontTypeface::SansSerif:
        currentFontTypefaceString = "SansSerif";
        break;
    }
    QString currentThemeString;
    switch (m_currentTheme) {
    case Theme::Light:
        currentThemeString = "Light";
        break;
    case Theme::Dark:
        currentThemeString = "Dark";
        break;
    case Theme::Sepia:
        currentThemeString = "Sepia";
        break;
    }
    m_settingsDatabase->setValue(QStringLiteral("selectedFontTypeface"), currentFontTypefaceString);
    m_settingsDatabase->setValue(QStringLiteral("editorMediumFontSize"), m_editorMediumFontSize);
    m_settingsDatabase->setValue(QStringLiteral("isTextFullWidth"),
                                 m_textEdit->lineWrapMode() == QTextEdit::WidgetWidth ? true
                                                                                      : false);
    m_settingsDatabase->setValue(QStringLiteral("charsLimitPerFontMono"),
                                 m_currentCharsLimitPerFont.mono);
    m_settingsDatabase->setValue(QStringLiteral("charsLimitPerFontSerif"),
                                 m_currentCharsLimitPerFont.serif);
    m_settingsDatabase->setValue(QStringLiteral("charsLimitPerFontSansSerif"),
                                 m_currentCharsLimitPerFont.sansSerif);
    m_settingsDatabase->setValue(QStringLiteral("theme"), currentThemeString);
    m_settingsDatabase->setValue(QStringLiteral("chosenMonoFont"),
                                 m_listOfMonoFonts.at(m_chosenMonoFontIndex));
    m_settingsDatabase->setValue(QStringLiteral("chosenSerifFont"),
                                 m_listOfSerifFonts.at(m_chosenSerifFontIndex));
    m_settingsDatabase->setValue(QStringLiteral("chosenSansSerifFont"),
                                 m_listOfSansSerifFonts.at(m_chosenSansSerifFontIndex));

    m_settingsDatabase->sync();

    m_noteEditorLogic->closeEditor();

    QCoreApplication::quit();
}

/*!
 * \brief MainWindow::checkForUpdates
 * Called when the "Check for Updates" menu item is clicked, this function
 * instructs the updater window to check if there are any updates available
 */
#if defined(UPDATE_CHECKER)
void MainWindow::checkForUpdates()
{
    m_updater.checkForUpdates(true);
}
#endif

/*!
 * \brief MainWindow::importNotesFile
 * Called when the "Import Notes" menu button is clicked. this function will
 * prompt the user to select a file, attempt to load the file, and update the DB
 * if valid.
 * The user is presented with a dialog box if the upload/import fails for any reason.
 */
void MainWindow::importNotesFile()
{
    executeImport(false);
}

/*!
 * \brief MainWindow::restoreNotesFile
 * Called when the "Restore Notes" menu button is clicked. this function will
 * prompt the user to select a file, attempt to load the file, and update the DB
 * if valid.
 * The user is presented with a dialog box if the upload/import/restore fails for any reason.
 */
void MainWindow::restoreNotesFile()
{
    if (m_listModel->rowCount() > 0) {
        QMessageBox msgBox;
        msgBox.setText(tr("Warning: All current notes will be lost. Make sure to create a backup "
                          "copy before proceeding."));
        msgBox.setInformativeText(tr("Would you like to continue?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() != QMessageBox::Yes) {
            return;
        }
    }
    executeImport(true);
}

/*!
 * \brief MainWindow::executeImport
 * Executes the note import process. if replace is true all current notes will be
 * removed otherwise current notes will be kept.
 * \param replace
 */
void MainWindow::executeImport(const bool replace)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Notes Backup File"), "",
                                                    tr("Notes Backup File (*.nbk)"));
    if (fileName.isEmpty()) {
        return;
    } else {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            return;
        }
        file.close();

        setButtonsAndFieldsEnabled(false);
        if (replace) {
            emit requestRestoreNotes(fileName);
        } else {
            emit requestImportNotes(fileName);
        }
        setButtonsAndFieldsEnabled(true);
        //        emit requestNotesList(SpecialNodeID::RootFolder, true);
    }
}

/*!
 * \brief MainWindow::exportNotesFile
 * Called when the "Export Notes" menu button is clicked. this function will
 * prompt the user to select a location for the export file, and then builds
 * the file.
 * The user is presented with a dialog box if the file cannot be opened for any reason.
 * \param clicked
 */
void MainWindow::exportNotesFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Notes"), "notes.nbk",
                                                    tr("Notes Backup File (*.nbk)"));
    if (fileName.isEmpty()) {
        return;
    } else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            return;
        }
        file.close();
        emit requestExportNotes(fileName);
    }
}

void MainWindow::toggleFolderTree()
{
    if (m_foldersWidget->isHidden()) {
        expandFolderTree();
    } else {
        collapseFolderTree();
    }
}

void MainWindow::collapseFolderTree()
{
    m_foldersWidget->setHidden(true);
    updateFrame();
}

void MainWindow::expandFolderTree()
{
    m_foldersWidget->setHidden(false);
    updateFrame();
}

void MainWindow::toggleNoteList()
{
    if (m_noteListWidget->isHidden()) {
        expandNoteList();
    } else {
        collapseNoteList();
    }
}

void MainWindow::collapseNoteList()
{
    m_noteListWidget->setHidden(true);
    updateFrame();
}

void MainWindow::expandNoteList()
{
    m_noteListWidget->setHidden(false);
    updateFrame();
}

/*!
 * \brief MainWindow::onGreenMaximizeButtonPressed
 * When the green button is pressed set it's icon accordingly
 */
void MainWindow::onGreenMaximizeButtonPressed()
{
#ifdef _WIN32
    m_greenMaximizeButton->setIcon(QIcon(":images/windows_minimize_pressed.png"));
#else
    if (windowState() == Qt::WindowFullScreen)
        m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/greenInPressed.png")));
    else
        m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/greenPressed.png")));
#endif
}

/*!
 * \brief MainWindow::onYellowMinimizeButtonPressed
 * When the yellow button is pressed set it's icon accordingly
 */
void MainWindow::onYellowMinimizeButtonPressed()
{
#ifdef _WIN32
    if (windowState() == Qt::WindowFullScreen) {
        m_yellowMinimizeButton->setIcon(
                QIcon(QStringLiteral(":images/windows_de-maximize_pressed.png")));
    } else {
        m_yellowMinimizeButton->setIcon(
                QIcon(QStringLiteral(":images/windows_maximize_pressed.png")));
    }
#else
    m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/yellowPressed.png")));
#endif
}

/*!
 * \brief MainWindow::onRedCloseButtonPressed
 * When the red button is pressed set it's icon accordingly
 */
void MainWindow::onRedCloseButtonPressed()
{
#ifdef _WIN32
    m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/windows_close_pressed.png")));
#else
    m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/redPressed.png")));
#endif
}

/*!
 * \brief MainWindow::onGreenMaximizeButtonClicked
 * When the green button is released the window goes fullscrren
 */
void MainWindow::onGreenMaximizeButtonClicked()
{
#ifdef _WIN32
    m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/windows_minimize_regular.png")));

    minimizeWindow();
    m_restoreAction->setText(tr("&Show Notes"));
#else
    m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/green.png")));

    fullscreenWindow();
#endif
}

/*!
 * \brief MainWindow::onYellowMinimizeButtonClicked
 * When yellow button is released the window is minimized
 */
void MainWindow::onYellowMinimizeButtonClicked()
{
#ifdef _WIN32
    m_yellowMinimizeButton->setIcon(
            QIcon(QStringLiteral(":images/windows_de-maximize_regular.png")));

    fullscreenWindow();
#else
    m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/yellow.png")));

    minimizeWindow();

#  if !defined(Q_OS_MAC)
    m_restoreAction->setText(tr("&Show Notes"));
#  endif
#endif
}

/*!
 * \brief MainWindow::onRedCloseButtonClicked
 * When red button is released the window get closed
 * If a new note created but wasn't edited, delete it from the database
 */
void MainWindow::onRedCloseButtonClicked()
{
#ifdef _WIN32
    m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/windows_close_regular.png")));
#else
    m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/red.png")));
#endif

    if (m_hideToTray && m_trayIcon->isVisible() && QSystemTrayIcon::isSystemTrayAvailable()) {
        setMainWindowVisibility(false);
    } else {
        QuitApplication();
    }
}

/*!
 * \brief MainWindow::closeEvent
 * Called when the window is about to close
 * \param event
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
    if (m_hideToTray && m_trayIcon->isVisible() && QSystemTrayIcon::isSystemTrayAvailable()) {
        // don't close the application, just hide to tray
        setMainWindowVisibility(false);
        event->ignore();
    } else {
        // save states and quit application
        QuitApplication();
    }
}

#ifndef _WIN32
/*!
 * \brief MainWindow::mousePressEvent
 * Set variables to the position of the window when the mouse is pressed
 * \param event
 */
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (m_useNativeWindowFrame) {
        MainWindowBase::mousePressEvent(event);
        return;
    }

    m_mousePressX = event->pos().x();
    m_mousePressY = event->pos().y();

    if (event->buttons() == Qt::LeftButton) {
        if (isTitleBar(m_mousePressX, m_mousePressY)) {

#  if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            m_canMoveWindow = !window()->windowHandle()->startSystemMove();
#  else
            m_canMoveWindow = true;
#  endif

#  ifndef __APPLE__
        } else if (!isMaximized() && !isFullScreen()) {
            m_canStretchWindow = true;

            if ((m_mousePressX < width() && m_mousePressX > width() - m_layoutMargin)
                && (m_mousePressY < m_layoutMargin && m_mousePressY > 0)) {
                m_stretchSide = StretchSide::TopRight;
            } else if ((m_mousePressX < width() && m_mousePressX > width() - m_layoutMargin)
                       && (m_mousePressY < height() && m_mousePressY > height() - m_layoutMargin)) {
                m_stretchSide = StretchSide::BottomRight;
            } else if ((m_mousePressX < m_layoutMargin && m_mousePressX > 0)
                       && (m_mousePressY < m_layoutMargin && m_mousePressY > 0)) {
                m_stretchSide = StretchSide::TopLeft;
            } else if ((m_mousePressX < m_layoutMargin && m_mousePressX > 0)
                       && (m_mousePressY < height() && m_mousePressY > height() - m_layoutMargin)) {
                m_stretchSide = StretchSide::BottomLeft;
            } else if (m_mousePressX < width() && m_mousePressX > width() - m_layoutMargin) {
                m_stretchSide = StretchSide::Right;
            } else if (m_mousePressX < m_layoutMargin && m_mousePressX > 0) {
                m_stretchSide = StretchSide::Left;
            } else if (m_mousePressY < height() && m_mousePressY > height() - m_layoutMargin) {
                m_stretchSide = StretchSide::Bottom;
            } else if (m_mousePressY < m_layoutMargin && m_mousePressY > 0) {
                m_stretchSide = StretchSide::Top;
            } else {
                m_stretchSide = StretchSide::None;
            }
#  endif
        }

        event->accept();

    } else {
        MainWindowBase::mousePressEvent(event);
    }
}

/*!
 * \brief MainWindow::mouseMoveEvent
 * Move the window according to the mouse positions
 * \param event
 */
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_useNativeWindowFrame) {
        MainWindowBase::mouseMoveEvent(event);
        return;
    }

#  ifndef __APPLE__
    if (!m_canStretchWindow && !m_canMoveWindow) {
        m_mousePressX = event->pos().x();
        m_mousePressY = event->pos().y();

        if ((m_mousePressX < width() && m_mousePressX > width() - m_layoutMargin)
            && (m_mousePressY < m_layoutMargin && m_mousePressY > 0)) {
            m_stretchSide = StretchSide::TopRight;
        } else if ((m_mousePressX < width() && m_mousePressX > width() - m_layoutMargin)
                   && (m_mousePressY < height() && m_mousePressY > height() - m_layoutMargin)) {
            m_stretchSide = StretchSide::BottomRight;
        } else if ((m_mousePressX < m_layoutMargin && m_mousePressX > 0)
                   && (m_mousePressY < m_layoutMargin && m_mousePressY > 0)) {
            m_stretchSide = StretchSide::TopLeft;
        } else if ((m_mousePressX < m_layoutMargin && m_mousePressX > 0)
                   && (m_mousePressY < height() && m_mousePressY > height() - m_layoutMargin)) {
            m_stretchSide = StretchSide::BottomLeft;
        } else if (m_mousePressX < width() && m_mousePressX > width() - m_layoutMargin) {
            m_stretchSide = StretchSide::Right;
        } else if (m_mousePressX < m_layoutMargin && m_mousePressX > 0) {
            m_stretchSide = StretchSide::Left;
        } else if (m_mousePressY < height() && m_mousePressY > height() - m_layoutMargin) {
            m_stretchSide = StretchSide::Bottom;
        } else if (m_mousePressY < m_layoutMargin && m_mousePressY > 0) {
            m_stretchSide = StretchSide::Top;
        } else {
            m_stretchSide = StretchSide::None;
        }
    }

    if (!m_canMoveWindow && !isMaximized() && !isFullScreen()) {
        switch (m_stretchSide) {
        case StretchSide::Right:
        case StretchSide::Left:
            ui->centralWidget->setCursor(Qt::SizeHorCursor);
            break;
        case StretchSide::Top:
        case StretchSide::Bottom:
            ui->centralWidget->setCursor(Qt::SizeVerCursor);
            break;
        case StretchSide::TopRight:
        case StretchSide::BottomLeft:
            ui->centralWidget->setCursor(Qt::SizeBDiagCursor);
            break;
        case StretchSide::TopLeft:
        case StretchSide::BottomRight:
            ui->centralWidget->setCursor(Qt::SizeFDiagCursor);
            break;
        default:
            if (!m_canStretchWindow)
                ui->centralWidget->setCursor(Qt::ArrowCursor);
            break;
        }
    }
#  endif

    if (m_canMoveWindow) {
        int dx = event->globalX() - m_mousePressX;
        int dy = event->globalY() - m_mousePressY;
        move(dx, dy);
    }

#  ifndef __APPLE__
    else if (m_canStretchWindow && !isMaximized() && !isFullScreen()) {
        int newX = x();
        int newY = y();
        int newWidth = width();
        int newHeight = height();

        int minY = QApplication::primaryScreen()->availableGeometry().y();

        switch (m_stretchSide) {
        case StretchSide::Right:
            newWidth = abs(event->globalPos().x() - x() + 1);
            newWidth = newWidth < minimumWidth() ? minimumWidth() : newWidth;
            break;
        case StretchSide::Left:
            newX = event->globalPos().x() - m_mousePressX;
            newX = newX > 0 ? newX : 0;
            newX = newX > geometry().bottomRight().x() - minimumWidth()
                    ? geometry().bottomRight().x() - minimumWidth()
                    : newX;
            newWidth = geometry().topRight().x() - newX + 1;
            newWidth = newWidth < minimumWidth() ? minimumWidth() : newWidth;
            break;
        case StretchSide::Top:
            newY = event->globalY() - m_mousePressY;
            newY = newY < minY ? minY : newY;
            newY = newY > geometry().bottomRight().y() - minimumHeight()
                    ? geometry().bottomRight().y() - minimumHeight()
                    : newY;
            newHeight = geometry().bottomLeft().y() - newY + 1;
            newHeight = newHeight < minimumHeight() ? minimumHeight() : newHeight;

            break;
        case StretchSide::Bottom:
            newHeight = abs(event->globalY() - y() + 1);
            newHeight = newHeight < minimumHeight() ? minimumHeight() : newHeight;

            break;
        case StretchSide::TopLeft:
            newX = event->globalPos().x() - m_mousePressX;
            newX = newX < 0 ? 0 : newX;
            newX = newX > geometry().bottomRight().x() - minimumWidth()
                    ? geometry().bottomRight().x() - minimumWidth()
                    : newX;

            newY = event->globalY() - m_mousePressY;
            newY = newY < minY ? minY : newY;
            newY = newY > geometry().bottomRight().y() - minimumHeight()
                    ? geometry().bottomRight().y() - minimumHeight()
                    : newY;

            newWidth = geometry().bottomRight().x() - newX + 1;
            newWidth = newWidth < minimumWidth() ? minimumWidth() : newWidth;

            newHeight = geometry().bottomRight().y() - newY + 1;
            newHeight = newHeight < minimumHeight() ? minimumHeight() : newHeight;

            break;
        case StretchSide::BottomLeft:
            newX = event->globalPos().x() - m_mousePressX;
            newX = newX < 0 ? 0 : newX;
            newX = newX > geometry().bottomRight().x() - minimumWidth()
                    ? geometry().bottomRight().x() - minimumWidth()
                    : newX;

            newWidth = geometry().bottomRight().x() - newX + 1;
            newWidth = newWidth < minimumWidth() ? minimumWidth() : newWidth;

            newHeight = event->globalY() - y() + 1;
            newHeight = newHeight < minimumHeight() ? minimumHeight() : newHeight;

            break;
        case StretchSide::TopRight:
            newY = event->globalY() - m_mousePressY;
            newY = newY > geometry().bottomRight().y() - minimumHeight()
                    ? geometry().bottomRight().y() - minimumHeight()
                    : newY;
            newY = newY < minY ? minY : newY;

            newWidth = event->globalPos().x() - x() + 1;
            newWidth = newWidth < minimumWidth() ? minimumWidth() : newWidth;

            newHeight = geometry().bottomRight().y() - newY + 1;
            newHeight = newHeight < minimumHeight() ? minimumHeight() : newHeight;

            break;
        case StretchSide::BottomRight:
            newWidth = event->globalPos().x() - x() + 1;
            newWidth = newWidth < minimumWidth() ? minimumWidth() : newWidth;

            newHeight = event->globalY() - y() + 1;
            newHeight = newHeight < minimumHeight() ? minimumHeight() : newHeight;

            break;
        default:
            break;
        }

        setGeometry(newX, newY, newWidth, newHeight);
    }
#  endif
    event->accept();
}

/*!
 * \brief MainWindow::mouseReleaseEvent
 * Initialize flags
 * \param event
 */
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_useNativeWindowFrame) {
        MainWindowBase::mouseReleaseEvent(event);
        return;
    }

    m_canMoveWindow = false;
    m_canStretchWindow = false;
    QApplication::restoreOverrideCursor();
    event->accept();
}
#else
/*!
 * \brief MainWindow::mousePressEvent
 * Set variables to the position of the window when the mouse is pressed
 * \param event
 */
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (m_useNativeWindowFrame) {
        MainWindowBase::mousePressEvent(event);
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (isTitleBar(event->pos().x(), event->pos().y())) {

#  if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            m_canMoveWindow = !window()->windowHandle()->startSystemMove();
#  else
            m_canMoveWindow = true;
#  endif
            m_mousePressX = event->pos().x();
            m_mousePressY = event->pos().y();
        }
    }

    event->accept();
}

/*!
 * \brief MainWindow::mouseMoveEvent
 * Move the window according to the mouse positions
 * \param event
 */
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_useNativeWindowFrame) {
        MainWindowBase::mouseMoveEvent(event);
        return;
    }

    if (m_canMoveWindow) {
        //        setCursor(Qt::ClosedHandCursor);
        int dx = event->globalPos().x() - m_mousePressX;
        int dy = event->globalY() - m_mousePressY;
        move(dx, dy);
    }
}

/*!
 * \brief MainWindow::mouseReleaseEvent
 * Initialize flags
 * \param event
 */
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_useNativeWindowFrame) {
        MainWindowBase::mouseReleaseEvent(event);
        return;
    }

    m_canMoveWindow = false;
    //    unsetCursor();
    event->accept();
}
#endif

/*!
 * \brief MainWindow::clearSearch
 */
void MainWindow::clearSearch()
{
    m_listView->setFocusPolicy(Qt::StrongFocus);

    m_searchEdit->blockSignals(true);
    m_searchEdit->clear();
    m_searchEdit->blockSignals(false);

    m_textEdit->blockSignals(true);
    m_textEdit->clear();
    m_textEdit->clearFocus();
    m_editorDateLabel->clear();
    m_textEdit->blockSignals(false);

    m_clearButton->hide();
    m_searchEdit->setFocus();
}

void MainWindow::showErrorMessage(const QString &title, const QString &content)
{
    QMessageBox::information(this, title, content);
}

void MainWindow::setNoteListLoading()
{
    ui->listviewLabel1->setText("Loading…");
    ui->listviewLabel2->setText("");
}

void MainWindow::selectAllNotesInList()
{
    m_listViewLogic->selectAllNotes();
}

void MainWindow::updateFrame()
{
    if (m_foldersWidget->isHidden() && m_noteListWidget->isHidden()) {
        setWindowButtonsVisible(false);
    } else {
        setWindowButtonsVisible(true);
    }
}

/*!
 * \brief MainWindow::checkMigration
 */
void MainWindow::migrateFromV0_9_0()
{
    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());

    QString oldNoteDBPath(dir.path() + QDir::separator() + "Notes.ini");
    if (QFile::exists(oldNoteDBPath)) {
        migrateNoteFromV0_9_0(oldNoteDBPath);
    }

    QString oldTrashDBPath(dir.path() + QDir::separator() + "Trash.ini");
    if (QFile::exists(oldTrashDBPath)) {
        migrateTrashFromV0_9_0(oldTrashDBPath);
    }
}

/*!
 * \brief MainWindow::migrateNote
 * \param notePath
 */
void MainWindow::migrateNoteFromV0_9_0(const QString &notePath)
{
    QSettings notesIni(notePath, QSettings::IniFormat);
    QStringList dbKeys = notesIni.allKeys();
    QVector<NodeData> noteList;

    auto it = dbKeys.begin();
    for (; it < dbKeys.end() - 1; it += 3) {
        QString noteName = it->split(QStringLiteral("/"))[0];
        int id = noteName.split(QStringLiteral("_"))[1].toInt();
        NodeData newNote;
        newNote.setId(id);
        QString createdDateDB =
                notesIni.value(noteName + QStringLiteral("/dateCreated"), "Error").toString();
        newNote.setCreationDateTime(QDateTime::fromString(createdDateDB, Qt::ISODate));
        QString lastEditedDateDB =
                notesIni.value(noteName + QStringLiteral("/dateEdited"), "Error").toString();
        newNote.setLastModificationDateTime(QDateTime::fromString(lastEditedDateDB, Qt::ISODate));
        QString contentText =
                notesIni.value(noteName + QStringLiteral("/content"), "Error").toString();
        newNote.setContent(contentText);
        QString firstLine = NoteEditorLogic::getFirstLine(contentText);
        newNote.setFullTitle(firstLine);
        noteList.append(newNote);
    }

    if (!noteList.isEmpty()) {
        emit requestMigrateNotesFromV0_9_0(noteList);
    }

    QFile oldNoteDBFile(notePath);
    oldNoteDBFile.rename(QFileInfo(notePath).dir().path() + QDir::separator()
                         + QStringLiteral("oldNotes.ini"));
}

/*!
 * \brief MainWindow::migrateTrash
 * \param trashPath
 */
void MainWindow::migrateTrashFromV0_9_0(const QString &trashPath)
{
    QSettings trashIni(trashPath, QSettings::IniFormat);
    QStringList dbKeys = trashIni.allKeys();

    QVector<NodeData> noteList;

    auto it = dbKeys.begin();
    for (; it < dbKeys.end() - 1; it += 3) {
        QString noteName = it->split(QStringLiteral("/"))[0];
        int id = noteName.split(QStringLiteral("_"))[1].toInt();
        NodeData newNote;
        newNote.setId(id);
        QString createdDateDB =
                trashIni.value(noteName + QStringLiteral("/dateCreated"), "Error").toString();
        newNote.setCreationDateTime(QDateTime::fromString(createdDateDB, Qt::ISODate));
        QString lastEditedDateDB =
                trashIni.value(noteName + QStringLiteral("/dateEdited"), "Error").toString();
        newNote.setLastModificationDateTime(QDateTime::fromString(lastEditedDateDB, Qt::ISODate));
        QString contentText =
                trashIni.value(noteName + QStringLiteral("/content"), "Error").toString();
        newNote.setContent(contentText);
        QString firstLine = NoteEditorLogic::getFirstLine(contentText);
        newNote.setFullTitle(firstLine);
        noteList.append(newNote);
    }

    if (!noteList.isEmpty()) {
        emit requestMigrateTrashFromV0_9_0(noteList);
    }
    QFile oldTrashDBFile(trashPath);
    oldTrashDBFile.rename(QFileInfo(trashPath).dir().path() + QDir::separator()
                          + QStringLiteral("oldTrash.ini"));
}

/*!
 * \brief MainWindow::dropShadow
 * \param painter
 * \param type
 * \param side
 */
void MainWindow::dropShadow(QPainter &painter, ShadowType type, MainWindow::ShadowSide side)
{
    int resizedShadowWidth = m_shadowWidth > m_layoutMargin ? m_layoutMargin : m_shadowWidth;

    QRect mainRect = rect();

    QRect innerRect(m_layoutMargin, m_layoutMargin, mainRect.width() - 2 * resizedShadowWidth + 1,
                    mainRect.height() - 2 * resizedShadowWidth + 1);
    QRect outerRect(innerRect.x() - resizedShadowWidth, innerRect.y() - resizedShadowWidth,
                    innerRect.width() + 2 * resizedShadowWidth,
                    innerRect.height() + 2 * resizedShadowWidth);

    QPoint center;
    QPoint topLeft;
    QPoint bottomRight;
    QPoint shadowStart;
    QPoint shadowStop;
    QRadialGradient radialGradient;
    QLinearGradient linearGradient;

    switch (side) {
    case ShadowSide::Left:
        topLeft = QPoint(outerRect.left(), innerRect.top() + 1);
        bottomRight = QPoint(innerRect.left(), innerRect.bottom() - 1);
        shadowStart = QPoint(innerRect.left(), innerRect.top() + 1);
        shadowStop = QPoint(outerRect.left(), innerRect.top() + 1);
        break;
    case ShadowSide::Top:
        topLeft = QPoint(innerRect.left() + 1, outerRect.top());
        bottomRight = QPoint(innerRect.right() - 1, innerRect.top());
        shadowStart = QPoint(innerRect.left() + 1, innerRect.top());
        shadowStop = QPoint(innerRect.left() + 1, outerRect.top());
        break;
    case ShadowSide::Right:
        topLeft = QPoint(innerRect.right(), innerRect.top() + 1);
        bottomRight = QPoint(outerRect.right(), innerRect.bottom() - 1);
        shadowStart = QPoint(innerRect.right(), innerRect.top() + 1);
        shadowStop = QPoint(outerRect.right(), innerRect.top() + 1);
        break;
    case ShadowSide::Bottom:
        topLeft = QPoint(innerRect.left() + 1, innerRect.bottom());
        bottomRight = QPoint(innerRect.right() - 1, outerRect.bottom());
        shadowStart = QPoint(innerRect.left() + 1, innerRect.bottom());
        shadowStop = QPoint(innerRect.left() + 1, outerRect.bottom());
        break;
    case ShadowSide::TopLeft:
        topLeft = outerRect.topLeft();
        bottomRight = innerRect.topLeft();
        center = innerRect.topLeft();
        break;
    case ShadowSide::TopRight:
        topLeft = QPoint(innerRect.right(), outerRect.top());
        bottomRight = QPoint(outerRect.right(), innerRect.top());
        center = innerRect.topRight();
        break;
    case ShadowSide::BottomRight:
        topLeft = innerRect.bottomRight();
        bottomRight = outerRect.bottomRight();
        center = innerRect.bottomRight();
        break;
    case ShadowSide::BottomLeft:
        topLeft = QPoint(outerRect.left(), innerRect.bottom());
        bottomRight = QPoint(innerRect.left(), outerRect.bottom());
        center = innerRect.bottomLeft();
        break;
    }

    QRect zone(topLeft, bottomRight);
    radialGradient = QRadialGradient(center, resizedShadowWidth, center);

    linearGradient.setStart(shadowStart);
    linearGradient.setFinalStop(shadowStop);

    switch (type) {
    case ShadowType::Radial:
        fillRectWithGradient(painter, zone, radialGradient);
        break;
    case ShadowType::Linear:
        fillRectWithGradient(painter, zone, linearGradient);
        break;
    }
}

/*!
 * \brief MainWindow::fillRectWithGradient
 * \param painter
 * \param rect
 * \param gradient
 */
void MainWindow::fillRectWithGradient(QPainter &painter, QRect rect, QGradient &gradient)
{
    double variance = 0.2;
    double xMax = 1.10;
    double q = 70 / gaussianDist(0, 0, sqrt(variance));
    double nPt = 100.0;

    for (int i = 0; i <= nPt; i++) {
        double v = gaussianDist(i * xMax / nPt, 0, sqrt(variance));

        QColor c(168, 168, 168, int(q * v));
        gradient.setColorAt(i / nPt, c);
    }

    painter.fillRect(rect, gradient);
}

/*!
 * \brief MainWindow::gaussianDist
 * \param x
 * \param center
 * \param sigma
 * \return
 */
double MainWindow::gaussianDist(double x, const double center, double sigma) const
{
    return (1.0 / (2 * M_PI * pow(sigma, 2)) * exp(-pow(x - center, 2) / (2 * pow(sigma, 2))));
}

/*!
 * \brief MainWindow::mouseDoubleClickEvent
 * When the blank area at the top of window is double-clicked the window get maximized
 * \param event
 */
void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_useNativeWindowFrame || event->buttons() != Qt::LeftButton
        || !isTitleBar(event->pos().x(), event->pos().y())) {
        MainWindowBase::mouseDoubleClickEvent(event);
        return;
    }

#ifndef __APPLE__
    maximizeWindow();
#else
    maximizeWindowMac();
#endif
    event->accept();
}

/*!
 * \brief MainWindow::leaveEvent
 */
void MainWindow::leaveEvent(QEvent *)
{
    unsetCursor();
}

/*!
 * \brief MainWindow::changeEvent
 */
void MainWindow::changeEvent(QEvent *event)
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    if (event->type() == QEvent::WindowStateChange && !m_useNativeWindowFrame) {
        if (isMaximized())
            setMargins(QMargins());
        else
            setMargins(QMargins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin));
    }
#endif
    MainWindowBase::changeEvent(event);
}

/*!
 * \brief MainWindow::setVisibilityOfFrameRightNonEditor
 * Either show or hide all widgets which are not m_textEdit
 */
void MainWindow::setVisibilityOfFrameRightNonEditor(bool isVisible)
{
    m_isFrameRightTopWidgetsVisible = isVisible;
    m_areNonEditorWidgetsVisible = isVisible;

    m_editorDateLabel->setVisible(isVisible);
    m_trashButton->setVisible(isVisible);
    m_dotsButton->setVisible(isVisible);
    m_styleEditorButton->setVisible(isVisible);

    // If the notes list is collapsed, hide the window buttons
    if (m_splitter) {
        if (m_foldersWidget->isHidden() && m_noteListWidget->isHidden()) {
            setWindowButtonsVisible(isVisible);
        }
    }
}

void MainWindow::setWindowButtonsVisible(bool isVisible)
{
#ifdef __APPLE__
    setStandardWindowButtonsMacVisibility(isVisible);
#else
    bool visible = !m_useNativeWindowFrame && isVisible;
    m_redCloseButton->setVisible(visible);
    m_yellowMinimizeButton->setVisible(visible);
    m_greenMaximizeButton->setVisible(visible);
#endif
}

/*!
 * \brief MainWindow::eventFilter
 * Mostly take care on the event happened on widget whose filter installed to tht mainwindow
 * \param object
 * \param event
 * \return
 */
bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter: {
        if (qApp->applicationState() == Qt::ApplicationActive) {
#ifdef _WIN32
            if (object == m_redCloseButton) {
                m_redCloseButton->setIcon(
                        QIcon(QStringLiteral(":images/windows_close_hovered.png")));
            }

            if (object == m_yellowMinimizeButton) {
                if (windowState() == Qt::WindowFullScreen) {
                    m_yellowMinimizeButton->setIcon(
                            QIcon(QStringLiteral(":images/windows_de-maximize_hovered.png")));
                } else {
                    m_yellowMinimizeButton->setIcon(
                            QIcon(QStringLiteral(":images/windows_maximize_hovered.png")));
                }
            }

            if (object == m_greenMaximizeButton) {
                m_greenMaximizeButton->setIcon(
                        QIcon(QStringLiteral(":images/windows_minimize_hovered.png")));
            }
#else
            // When hovering one of the traffic light buttons (red, yellow, green),
            // set new icons to show their function
            if (object == m_redCloseButton || object == m_yellowMinimizeButton
                || object == m_greenMaximizeButton) {

                m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/redHovered.png")));
                m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/yellowHovered.png")));
                if (windowState() == Qt::WindowFullScreen) {
                    m_greenMaximizeButton->setIcon(
                            QIcon(QStringLiteral(":images/greenInHovered.png")));
                } else {
                    m_greenMaximizeButton->setIcon(
                            QIcon(QStringLiteral(":images/greenHovered.png")));
                }
            }
#endif

            if (object == m_newNoteButton) {
                setCursor(Qt::PointingHandCursor);
                m_newNoteButton->setIcon(QIcon(QStringLiteral(":/images/newNote_Hovered.png")));
            }

            if (object == m_trashButton) {
                setCursor(Qt::PointingHandCursor);
                m_trashButton->setIcon(QIcon(QStringLiteral(":/images/trashCan_Hovered.png")));
            }

            if (object == m_dotsButton) {
                setCursor(Qt::PointingHandCursor);
                m_dotsButton->setIcon(QIcon(QStringLiteral(":/images/3dots_Hovered.png")));
            }

            if (object == m_styleEditorButton) {
                setCursor(Qt::PointingHandCursor);
                QString ss = QStringLiteral("QPushButton { "
                                            "  border: none; "
                                            "  padding: 0px; "
                                            "  color: rgb(51, 110, 162);"
                                            "}");
                m_styleEditorButton->setStyleSheet(ss);
            }

            if (object == ui->frameRightTop && !m_areNonEditorWidgetsVisible) {
                setVisibilityOfFrameRightNonEditor(true);
            }
        }

        if (object == ui->frame) {
            ui->centralWidget->setCursor(Qt::ArrowCursor);
        }

        break;
    }
    case QEvent::Leave: {
        if (qApp->applicationState() == Qt::ApplicationActive) {
            // When not hovering, change back the icons of the traffic lights to their default icon
            if (object == m_redCloseButton || object == m_yellowMinimizeButton
                || object == m_greenMaximizeButton) {

#ifdef _WIN32
                m_redCloseButton->setIcon(
                        QIcon(QStringLiteral(":images/windows_close_regular.png")));
                m_greenMaximizeButton->setIcon(
                        QIcon(QStringLiteral(":images/windows_minimize_regular.png")));

                if (windowState() == Qt::WindowFullScreen) {
                    m_yellowMinimizeButton->setIcon(
                            QIcon(QStringLiteral(":images/windows_de-maximize_regular.png")));
                } else {
                    m_yellowMinimizeButton->setIcon(
                            QIcon(QStringLiteral(":images/windows_maximize_regular.png")));
                }
#else
                m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/red.png")));
                m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/yellow.png")));
                m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/green.png")));
#endif
            }

            if (object == m_newNoteButton) {
                unsetCursor();
                m_newNoteButton->setIcon(QIcon(QStringLiteral(":/images/newNote_Regular.png")));
            }

            if (object == m_trashButton) {
                unsetCursor();
                m_trashButton->setIcon(QIcon(QStringLiteral(":/images/trashCan_Regular.png")));
            }

            if (object == m_dotsButton) {
                unsetCursor();
                m_dotsButton->setIcon(QIcon(QStringLiteral(":/images/3dots_Regular.png")));
            }

            if (object == m_styleEditorButton) {
                unsetCursor();
                QString ss = QStringLiteral("QPushButton { "
                                            "  border: none; "
                                            "  padding: 0px; "
                                            "  color: rgb(68, 138, 201);"
                                            "}");
                m_styleEditorButton->setStyleSheet(ss);
            }
        }
        break;
    }
    case QEvent::WindowDeactivate: {

        m_canMoveWindow = false;
        m_canStretchWindow = false;
        QApplication::restoreOverrideCursor();

#ifndef _WIN32
        m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/unfocusedButton")));
        m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/unfocusedButton")));
        m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/unfocusedButton")));
#endif
        m_newNoteButton->setIcon(QIcon(QStringLiteral(":/images/newNote_Regular.png")));
        m_trashButton->setIcon(QIcon(QStringLiteral(":/images/trashCan_Regular.png")));
        m_dotsButton->setIcon(QIcon(QStringLiteral(":/images/3dots_Regular.png")));
        break;
    }
    case QEvent::WindowActivate: {
#ifdef _WIN32
        m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/windows_close_regular.png")));
        m_greenMaximizeButton->setIcon(
                QIcon(QStringLiteral(":images/windows_minimize_regular.png")));

        if (windowState() == Qt::WindowFullScreen) {
            m_yellowMinimizeButton->setIcon(
                    QIcon(QStringLiteral(":images/windows_de-maximize_regular.png")));
        } else {
            m_yellowMinimizeButton->setIcon(
                    QIcon(QStringLiteral(":images/windows_maximize_regular.png")));
        }
#else
        m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/red.png")));
        m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/yellow.png")));
        m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/green.png")));
#endif
        m_newNoteButton->setIcon(QIcon(QStringLiteral(":/images/newNote_Regular.png")));
        m_trashButton->setIcon(QIcon(QStringLiteral(":/images/trashCan_Regular.png")));
        m_dotsButton->setIcon(QIcon(QStringLiteral(":/images/3dots_Regular.png")));
        break;
    }
#ifndef __APPLE__
    case QEvent::HoverEnter: {
        if (object == m_textEdit->verticalScrollBar()) {
            bool isSearching = !m_searchEdit->text().isEmpty();
            if (isSearching)
                m_textEdit->setFocusPolicy(Qt::NoFocus);
        }
        break;
    }
    case QEvent::HoverLeave: {
        bool isNoButtonClicked = qApp->mouseButtons() == Qt::NoButton;
        if (isNoButtonClicked) {
            if (object == m_textEdit->verticalScrollBar()) {
                m_textEdit->setFocusPolicy(Qt::StrongFocus);
            }
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        bool isMouseOnScrollBar = qApp->widgetAt(QCursor::pos()) != m_textEdit->verticalScrollBar();
        if (isMouseOnScrollBar) {
            if (object == m_textEdit->verticalScrollBar()) {
                m_textEdit->setFocusPolicy(Qt::StrongFocus);
            }
        }
        break;
    }
#endif
    case QEvent::FocusIn: {
        if (object == m_textEdit) {
            if (!m_isOperationRunning) {
                if (m_listModel->rowCount() == 0) {
                    if (!m_searchEdit->text().isEmpty()) {
                        m_listViewLogic->clearSearch(true);
                    } else {
                        createNewNote();
                    }
                }
            }
            m_listView->setCurrentRowActive(true);
            m_textEdit->setFocus();
        }

        if (object == m_searchEdit) {
            setSearchEditStyleSheet(true);
        }
        break;
    }
    case QEvent::FocusOut: {
        if (object == m_searchEdit) {
            setSearchEditStyleSheet(false);
        }
        break;
    }
    case QEvent::Resize: {
        if (m_textEdit->width() < m_smallEditorWidth) {
            m_noteEditorLogic->setCurrentMinimumEditorPadding(15);
        } else if (m_textEdit->width() > m_smallEditorWidth && m_textEdit->width() < 515) {
            m_noteEditorLogic->setCurrentMinimumEditorPadding(40);
        } else if (m_textEdit->width() > 515 && m_textEdit->width() < 755) {
            m_noteEditorLogic->setCurrentMinimumEditorPadding(50);
        } else if (m_textEdit->width() > 755 && m_textEdit->width() < 775) {
            m_noteEditorLogic->setCurrentMinimumEditorPadding(60);
        } else if (m_textEdit->width() > 775 && m_textEdit->width() < 800) {
            m_noteEditorLogic->setCurrentMinimumEditorPadding(70);
        } else if (m_textEdit->width() > 800) {
            m_noteEditorLogic->setCurrentMinimumEditorPadding(80);
        }

        setCurrentFontBasedOnTypeface(m_currentFontTypeface);

        //        alignTextEditText();
        break;
    }
    case QEvent::Show:
#if defined(UPDATE_CHECKER)
        if (object == &m_updater) {

            QRect rect = m_updater.geometry();
            QRect appRect = geometry();
            int titleBarHeight = 28;

            int x = int(appRect.x() + (appRect.width() - rect.width()) / 2.0);
            int y = int(appRect.y() + titleBarHeight + (appRect.height() - rect.height()) / 2.0);

            m_updater.setGeometry(QRect(x, y, rect.width(), rect.height()));
        }
#endif
        break;
    case QEvent::KeyPress: {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return && m_searchEdit->text().isEmpty()) {
            setFocusOnText();
        } else if (keyEvent->key() == Qt::Key_Return
                   && keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            setFocusOnText();
            return true;
        }
        break;
    }
    default:
        break;
    }

    return QObject::eventFilter(object, event);
}

/*!
 * \brief MainWindow::alreadyAppliedFormat
 * \param formatChars
 * Checks whether the bold/italic/strikethrough formatting was already applied
 */
bool MainWindow::alreadyAppliedFormat(const QString &formatChars)
{
    QTextCursor cursor = m_textEdit->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
        if (!cursor.hasSelection()) {
            QTextDocument *doc = m_textEdit->document();
            cursor = doc->find(QRegularExpression(QRegularExpression::escape(formatChars) + "[^ "
                                                  + formatChars + "]+"
                                                  + QRegularExpression::escape(formatChars) + "$"),
                               cursor.selectionStart(), QTextDocument::FindBackward);
            if (!cursor.isNull()) {
                // m_textEdit->setTextCursor(cursor);
                return true;
            }
            if (!cursor.hasSelection()) {
                return false;
            }
        }
    }
    if (cursor.selectedText().contains(formatChars)) {
        return true;
    }
    QString selectedText = cursor.selectedText();
    QTextDocument *doc = m_textEdit->document();
    cursor = doc->find(formatChars + selectedText + formatChars,
                       cursor.selectionStart() - formatChars.length());
    return cursor.hasSelection();
}

/*!
 * \brief MainWindow::stayOnTop
 * \param checked
 */
void MainWindow::stayOnTop(bool checked)
{
    m_alwaysStayOnTop = checked;

#ifndef __APPLE__
    Qt::WindowFlags flags = windowFlags();

    if (checked)
        flags |= Qt::WindowStaysOnTopHint;
    else
        flags &= ~Qt::WindowStaysOnTopHint;

    setWindowFlags(flags);
    setMainWindowVisibility(true);
#else
    setWindowAlwaysOnTopMac(checked);
#endif
}

/*!
 * \brief MainWindow::increaseHeading
 * Increase markdown heading level
 */
void MainWindow::increaseHeading()
{
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);
    QString selected_text = cursor.selectedText().trimmed();
    int count = QRegularExpression("^[#]*").match(selected_text).capturedLength();
    if (count < 6) {
        setHeading(++count);
    }
}

/*!
 * \brief MainWindow::decreaseHeading
 * Decrease markdown heading level
 */
void MainWindow::decreaseHeading()
{
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);
    QString selected_text = cursor.selectedText().trimmed();
    int count = QRegularExpression("^[#]*").match(selected_text).capturedLength();
    if (count > 0) {
        setHeading(--count);
    }
}

/*!
 * \brief MainWindow::setHeading
 * Set markdown heading level
 * \param level
 */
void MainWindow::setHeading(int level)
{
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);
    QString new_text = "\n";
    if (cursor.hasSelection()) {
        QString selected_text = cursor.selectedText();
        if (selected_text.at(0).unicode()
            != 8233) { // if it doesn't start with a paragraph delimiter (first line)
            new_text.clear();
        }
        selected_text = selected_text.trimmed().remove(QRegularExpression("^#*\\s?"));
        new_text += QString("#").repeated(level) + ((level == 0) ? "" : " ") + selected_text;
    } else {
        new_text = QString("#").repeated(level) + " ";
    }
    cursor.insertText(new_text);
}

/*!
 * \brief MainWindow::adjustUpperWidgets
 * Either push the widgets up or restore them to their original position
 * Needed for when using native window or going full screen
 * \param shouldPushUp
 */
void MainWindow::adjustUpperWidgets(bool shouldPushUp)
{

    // Adjust space above search field
    const QSizePolicy policy = ui->verticalSpacer_upSearchEdit->sizePolicy();
    ui->verticalSpacer_upSearchEdit->setMinimumSize(
            0, shouldPushUp ? ui->verticalSpacer_upScrollArea->sizeHint().height() : 25);
    ui->verticalSpacer_upTreeView->setMinimumSize(0, shouldPushUp ? 9 : 25);

    ui->verticalSpacer_upEditorDateLabel->changeSize(
            0, shouldPushUp ? ui->verticalSpacer_upScrollArea->sizeHint().height() : 25,
            policy.horizontalPolicy(), policy.verticalPolicy());

    // Force a full re-layout of the top right frame.
    // This fixes some widgets not properly updating after switching between native and non-native
    // window decoration modes.
    ui->frameRightTop->setStyleSheet(ui->frameRightTop->styleSheet());
}

/*!
 * \brief MainWindow::setUseNativeWindowFrame
 * \param useNativeWindowFrame
 */
void MainWindow::setUseNativeWindowFrame(bool useNativeWindowFrame)
{
    if (m_useNativeWindowFrame == useNativeWindowFrame)
        return;

    m_useNativeWindowFrame = useNativeWindowFrame;
    m_settingsDatabase->setValue(QStringLiteral("useNativeWindowFrame"), useNativeWindowFrame);

#ifndef __APPLE__
    m_greenMaximizeButton->setVisible(!useNativeWindowFrame);
    m_redCloseButton->setVisible(!useNativeWindowFrame);
    m_yellowMinimizeButton->setVisible(!useNativeWindowFrame);

    // Reset window flags to its initial state.
    Qt::WindowFlags flags = Qt::Window;

    if (!useNativeWindowFrame) {
        flags |= Qt::CustomizeWindowHint;
#  if defined(Q_OS_UNIX)
        flags |= Qt::FramelessWindowHint;
#  endif
    }

    setWindowFlags(flags);
#endif

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    if (useNativeWindowFrame || isMaximized()) {
        ui->centralWidget->layout()->setContentsMargins(QMargins());
    } else {
        QMargins margins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin);
        ui->centralWidget->layout()->setContentsMargins(margins);
    }
#endif

    adjustUpperWidgets(useNativeWindowFrame);

    setMainWindowVisibility(true);
}

void MainWindow::setHideToTray(bool enabled)
{
    m_hideToTray = enabled;
    m_settingsDatabase->setValue(QStringLiteral("hideToTrayEnabled"), enabled);
}

/*!
 * \brief MainWindow::toggleStayOnTop
 */
void MainWindow::toggleStayOnTop()
{
    stayOnTop(!m_alwaysStayOnTop);
}

/*!
 * \brief MainWindow::onSearchEditReturnPressed
 */
void MainWindow::onSearchEditReturnPressed()
{
    if (m_searchEdit->text().isEmpty())
        return;

    QString searchText = m_searchEdit->text();
    QTextDocument *doc = m_textEdit->document();
    // get current cursor
    QTextCursor from = m_textEdit->textCursor();
    // search
    QTextCursor found = doc->find(searchText, from);
    m_textEdit->setTextCursor(found);
}

/*!
 * \brief MainWindow::setMargins
 * \param margins
 */
void MainWindow::setMargins(QMargins margins)
{
    if (m_useNativeWindowFrame)
        return;

    ui->centralWidget->layout()->setContentsMargins(margins);
    m_trafficLightLayout.setGeometry(QRect(4 + margins.left(), 4 + margins.top(), 56, 16));
}

bool MainWindow::isTitleBar(int x, int y) const
{
    if (m_useNativeWindowFrame)
        return false;

    // The width of the title bar is essentially the width of the main window.
    int titleBarWidth = width();
    int titleBarHeight = ui->verticalSpacer_upTreeView->height();

    int adjustedX = x;
    int adjustedY = y;

    if (!isMaximized() && !isFullScreen()) {
        titleBarWidth -= m_layoutMargin * 2;
        adjustedX -= m_layoutMargin;
        adjustedY -= m_layoutMargin;
    }

    return (adjustedX >= 0 && adjustedX <= titleBarWidth && adjustedY >= 0
            && adjustedY <= titleBarHeight);
}
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
#include "editorsettingsoptions.h"
#include "fontloader.h"

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
      m_searchButton(nullptr),
      m_greenMaximizeButton(nullptr),
      m_redCloseButton(nullptr),
      m_yellowMinimizeButton(nullptr),
      m_trafficLightLayout(nullptr),
      m_newNoteButton(nullptr),
      m_dotsButton(nullptr),
      m_globalSettingsButton(nullptr),
      m_textEdit(nullptr),
      m_noteEditorLogic(nullptr),
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
      m_kanbanQuickView(nullptr),
      m_kanbanWidget(this),
#endif
      m_editorSettingsQuickView(nullptr),
      m_editorSettingsWidget(new QWidget(this)),
      m_tagPool(nullptr),
      m_dbManager(nullptr),
      m_dbThread(nullptr),
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
      m_currentTheme(Theme::Light),
      m_currentEditorTextColor(26, 26, 26),
      m_areNonEditorWidgetsVisible(true),
#if !defined(Q_OS_MAC)
      m_textEditScrollBarTimer(new QTimer(this)),
      m_textEditScrollBarTimerDuration(1000),
#endif
      m_isFrameRightTopWidgetsVisible(true),
      m_isEditorSettingsFromQuickViewVisible(false),
      m_isProVersionActivated(false),
      m_localLicenseData(nullptr),
      m_subscriptionWindowQuickView(nullptr),
      m_subscriptionWindowWidget(new QWidget(this)),
      m_purchaseDataAlt1(QStringLiteral("https://raw.githubusercontent.com/nuttyartist/notes/"
                                        "master/notes_purchase_data.json")),
      m_purchaseDataAlt2(
              QStringLiteral("https://www.rubymamistvalove.com/notes/notes_purchase_data.json")),
      m_dataBuffer(new QByteArray()),
      m_netManager(new QNetworkAccessManager(this)),
      m_reqAlt1(QNetworkRequest(QUrl(m_purchaseDataAlt1))),
      m_reqAlt2(QNetworkRequest(QUrl(m_purchaseDataAlt2))),
      m_netPurchaseDataReplyFirstAttempt(nullptr),
      m_netPurchaseDataReplySecondAttempt(nullptr),
      m_userLicenseKey(QStringLiteral("")),
      m_mainMenu(nullptr),
      m_buyOrManageSubscriptionAction(new QAction(this))
{
    ui->setupUi(this);
    setupMainWindow();
    setupFonts();
    setupSplitter();
    setupSearchEdit();
    setupEditorSettings();
    setupKeyboardShortcuts();
    setupDatabases();
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    setupSubscrirptionWindow();
    setupKanbanView();
#endif
    setupGlobalSettingsMenu();
    setupModelView();
    setupTextEdit();
    restoreStates();
    setupButtons();
    setupSignalsSlots();
#if defined(UPDATE_CHECKER)
    autoCheckForUpdates();
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    checkProVersion();
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
        pd->setWindowFlags(Qt::Window);
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

    resizeAndPositionEditorSettingsWindow();

    QJsonObject dataToSendToView{ { "parentWindowHeight", height() },
                                  { "parentWindowWidth", width() } };
    emit mainWindowResized(QVariant(dataToSendToView));

    QMainWindow::resizeEvent(event);
}

/*!
 * \brief MainWindow::resizeAndPositionEditorSettingsWindow
 */
void MainWindow::resizeAndPositionEditorSettingsWindow()
{
#if defined(Q_OS_WINDOWS) || (defined(Q_OS_UNIX) && !defined(Q_OS_MACOS))
    m_editorSettingsWidget->resize(240, 0.80 * this->height());
    QPoint buttonGlobalPos = m_dotsButton->mapToGlobal(QPoint(0, 0));
    m_editorSettingsWidget->move(buttonGlobalPos.x() - m_editorSettingsWidget->width()
                                         + m_dotsButton->width(),
                                 buttonGlobalPos.y() + m_dotsButton->height());
#else
    m_editorSettingsWidget->resize(300, 0.80 * this->height() + 40);
    QPoint buttonGlobalPos = m_dotsButton->mapToGlobal(QPoint(0, 0));
    m_editorSettingsWidget->move(buttonGlobalPos.x() - m_editorSettingsWidget->width() + 70,
                                 buttonGlobalPos.y() + m_dotsButton->height() - 10);
#endif
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
    //    flags |= Qt::FramelessWindowHint;
    flags = Qt::Window;
#  endif
    setWindowFlags(flags);
#endif

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    setAttribute(Qt::WA_TranslucentBackground);
#endif

    // load stylesheet
    QFile mainWindowStyleFile(QStringLiteral(":/styles/main-window.css"));
    mainWindowStyleFile.open(QFile::ReadOnly);
    m_styleSheet = QString::fromLatin1(mainWindowStyleFile.readAll());
    setStyleSheet(m_styleSheet);
    /**** Apply the stylesheet for all children we change classes for ****/

    // left frame
    ui->frameLeft->setStyleSheet(m_styleSheet);

    // middle frame
    ui->searchEdit->setStyleSheet(m_styleSheet);
    ui->verticalSpacer_upSearchEdit->setStyleSheet(m_styleSheet);
    ui->verticalSpacer_upSearchEdit2->setStyleSheet(m_styleSheet);
    ui->listviewLabel1->setStyleSheet(m_styleSheet);
    ui->listviewLabel2->setStyleSheet(m_styleSheet);

    // splitters
    ui->verticalSplitterLine_left->setStyleSheet(m_styleSheet);
    ui->verticalSplitterLine_middle->setStyleSheet(m_styleSheet);

    // buttons
    ui->toggleTreeViewButton->setStyleSheet(m_styleSheet);
    ui->newNoteButton->setStyleSheet(m_styleSheet);
    ui->dotsButton->setStyleSheet(m_styleSheet);
    ui->globalSettingsButton->setStyleSheet(m_styleSheet);
    ui->switchToTextViewButton->setStyleSheet(m_styleSheet);
    ui->switchToKanbanViewButton->setStyleSheet(m_styleSheet);

    // right frame (editor)
    ui->textEdit->setStyleSheet(m_styleSheet);
    ui->frameRight->setStyleSheet(m_styleSheet);
    ui->frameRightTop->setStyleSheet(m_styleSheet);

    // custom scrollbars on Linux and Windows
#if !defined(Q_OS_MACOS)
    QFile scollBarStyleFile(QStringLiteral(":/styles/components/custom-scrollbar.css"));
    scollBarStyleFile.open(QFile::ReadOnly);
    QString scrollbarStyleSheet = QString::fromLatin1(scollBarStyleFile.readAll());
    ui->textEdit->verticalScrollBar()->setStyleSheet(scrollbarStyleSheet);
    ui->textEdit->verticalScrollBar()->hide();
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

#if defined(Q_OS_WINDOWS)
    m_layoutMargin = 0;
    m_trafficLightLayout.setSpacing(0);
    m_trafficLightLayout.setContentsMargins(QMargins(0, 0, 0, 0));
    m_trafficLightLayout.setGeometry(QRect(2, 2, 90, 16));
#endif

    m_newNoteButton = ui->newNoteButton;
    m_dotsButton = ui->dotsButton;
    m_globalSettingsButton = ui->globalSettingsButton;
    m_searchEdit = ui->searchEdit;
    m_textEdit = ui->textEdit;
    m_editorDateLabel = ui->editorDateLabel;
    m_splitter = ui->splitter;
    m_foldersWidget = ui->frameLeft;
    m_noteListWidget = ui->frameMiddle;
    m_toggleTreeViewButton = ui->toggleTreeViewButton;
    m_switchToTextViewButton = ui->switchToTextViewButton;
    m_switchToKanbanViewButton = ui->switchToKanbanViewButton;
    // don't resize first two panes when resizing
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 0);
    m_splitter->setStretchFactor(2, 1);

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    QMargins margins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin);
    setMargins(margins);
#endif
    ui->frame->installEventFilter(this);
    ui->centralWidget->setMouseTracking(true);
    setMouseTracking(true);
    QPalette pal(palette());
    pal.setColor(QPalette::Window, QColor(248, 248, 248));
    setAutoFillBackground(true);
    setPalette(pal);

    m_newNoteButton->setToolTip(tr("Create New Note"));
    m_dotsButton->setToolTip(tr("Open Editor Settings"));
    m_globalSettingsButton->setToolTip(tr("Open App Settings"));
    m_toggleTreeViewButton->setToolTip("Toggle Folders Pane");
    m_switchToTextViewButton->setToolTip("Switch To Text View");
    m_switchToKanbanViewButton->setToolTip("Switch To Kanban View");

    ui->listviewLabel2->setMinimumSize({ 40, 25 });
    ui->listviewLabel2->setMaximumSize({ 40, 25 });

#ifdef __APPLE__
    QFont titleFont(m_displayFont, 13, QFont::DemiBold);
#else
    QFont titleFont(m_displayFont, 10, QFont::DemiBold);
#endif
    ui->listviewLabel1->setFont(titleFont);
    ui->listviewLabel2->setFont(titleFont);
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
    m_editorDateLabel->setFont(QFont(m_displayFont, 9, QFont::Bold));
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
    new QShortcut(Qt::Key_F11, this, SLOT(fullscreenWindow()));
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L), this),
            &QShortcut::activated, this, [=]() { m_listView->setFocus(); });
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M), this, SLOT(minimizeWindow()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this, SLOT(QuitApplication()));
#if defined(Q_OS_MACOS) || defined(Q_OS_WINDOWS)
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_K), this, SLOT(toggleStayOnTop()));
#endif
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_J), this, SLOT(toggleNoteList()));
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
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Plus), this, SLOT(increaseHeading()));
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), this),
            &QShortcut::activated, this, [=]() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
                if (m_kanbanWidget->isHidden()) {
                    if (m_editorSettingsWidget->isHidden()) {
                        showEditorSettings();
                    }
                } else {
                    emit toggleEditorSettingsKeyboardShorcutFired();
                };
#else
            if (m_editorSettingsWidget->isHidden()) {
                showEditorSettings();
            } else {
                m_editorSettingsWidget->close();
            }
#endif
            });
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), m_editorSettingsWidget),
            &QShortcut::activated, this, [=]() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
                if (m_kanbanWidget->isHidden()) {
                    if (!m_editorSettingsWidget->isHidden()) {
                        m_editorSettingsWidget->close();
                    }
                } else {
                    emit toggleEditorSettingsKeyboardShorcutFired();
                };
#else
                if (m_editorSettingsWidget->isVisible()) {
                    m_editorSettingsWidget->close();
                };
#endif
            });
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_K), this),
            &QShortcut::activated, this, [=]() {
                if (m_kanbanWidget->isHidden()) {
                    setKanbanVisibility(true);
                } else {
                    setKanbanVisibility(false);
                }
            });
#endif
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
 * \brief MainWindow::setupButtons
 * Setting up the red (close), yellow (minimize), and green (maximize) buttons
 * Make only the buttons icon visible
 * And install this class event filter to them, to act when hovering on one of them
 */
void MainWindow::setupButtons()
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

    QFont fontAwesomeIcon = FontLoader::getInstance().loadFont("Font Awesome 6 Free Solid", "", 16);
    QFont materialSymbols = FontLoader::getInstance().loadFont("Material Symbols Outlined", "", 30);

#if defined(Q_OS_MACOS)
    int pointSizeOffset = 0;
#else
    int pointSizeOffset = -4;
#endif

    fontAwesomeIcon.setPointSize(16 + pointSizeOffset);
    m_globalSettingsButton->setFont(fontAwesomeIcon);
    m_globalSettingsButton->setText(u8"\uf013"); // fa-gear

#if defined(Q_OS_MACOS)
    materialSymbols.setPointSize(30 + pointSizeOffset);
#else
    materialSymbols.setPointSize(30 + pointSizeOffset - 3);
#endif
    m_dotsButton->setFont(materialSymbols);
    m_dotsButton->setText(u8"\ue5d3"); // ellipsis_h

#if defined(Q_OS_MACOS)
    materialSymbols.setPointSize(24 + pointSizeOffset);
#else
    materialSymbols.setPointSize(21 + pointSizeOffset);
#endif
    m_switchToKanbanViewButton->setFont(materialSymbols);
    m_switchToKanbanViewButton->setText(u8"\ueb7f"); // view_kanban
    m_switchToTextViewButton->setFont(materialSymbols);
    m_switchToTextViewButton->setText(u8"\uef42"); // article

    materialSymbols.setPointSize(20 + pointSizeOffset);
    m_toggleTreeViewButton->setFont(materialSymbols);
    if (m_foldersWidget->isHidden()) {
        m_toggleTreeViewButton->setText(u8"\ue31c"); // keyboard_tab_rtl
    } else {
        m_toggleTreeViewButton->setText(u8"\uec73"); // keyboard_tab_rtl
    }

    fontAwesomeIcon.setPointSize(17 + pointSizeOffset);
    m_newNoteButton->setFont(fontAwesomeIcon);
    m_newNoteButton->setText(u8"\uf067"); // fa_plus

#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
    m_switchToTextViewButton->hide();
    m_switchToKanbanViewButton->hide();
#endif
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
    connect(m_newNoteButton, &QPushButton::clicked, this, &MainWindow::onNewNoteButtonClicked);
    // 3 dots button
    connect(m_dotsButton, &QPushButton::clicked, this, &MainWindow::onDotsButtonClicked);
    // switch to kanban view button
    connect(m_switchToKanbanViewButton, &QPushButton::clicked, this,
            &MainWindow::onSwitchToKanbanViewButtonClicked);
    // global settings button
    connect(m_globalSettingsButton, &QPushButton::clicked, this,
            &MainWindow::onGlobalSettingsButtonClicked);
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

    // TextEdit's scroll bar
#if !defined(Q_OS_MAC)
    connect(m_textEdit->verticalScrollBar(), &QAbstractSlider::actionTriggered, this, [=]() {
        m_textEdit->verticalScrollBar()->show();
        if (m_textEditScrollBarTimer->isActive()) {
            m_textEditScrollBarTimer->stop();
            m_textEditScrollBarTimer->start(m_textEditScrollBarTimerDuration);
        } else {
            m_textEditScrollBarTimer->start(m_textEditScrollBarTimerDuration);
        }
    });
    connect(m_textEditScrollBarTimer, &QTimer::timeout, this,
            [=]() { m_textEdit->verticalScrollBar()->hide(); });
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
    connect(m_noteEditorLogic, &NoteEditorLogic::setVisibilityOfFrameRightWidgets, this,
            [this](bool vl) { setVisibilityOfFrameRightWidgets(vl); });
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
    connect(m_noteEditorLogic, &NoteEditorLogic::showKanbanView, this, [this]() {
        if (!m_editorSettingsWidget->isHidden()) {
            m_editorSettingsWidget->close();
            //            emit toggleEditorSettingsKeyboardShorcutFired();
        }

        ui->frameRightTop->hide();
    });
    connect(m_noteEditorLogic, &NoteEditorLogic::hideKanbanView, this, [this]() {
        //        if (m_isEditorSettingsFromQuickViewVisible) {
        //            showEditorSettings();
        //        }

        ui->frameRightTop->show();
    });
    connect(m_listViewLogic, &ListViewLogic::requestClearSearchUI, this, &MainWindow::clearSearch);
    connect(m_treeViewLogic, &TreeViewLogic::addNoteToTag, m_listViewLogic,
            &ListViewLogic::onAddTagRequestD);
    connect(m_listViewLogic, &ListViewLogic::listViewLabelChanged, this,
            [this](const QString &l1, const QString &l2) {
                ui->listviewLabel1->setText(l1);
                ui->listviewLabel2->setText(l2);
                m_splitter->setHandleWidth(0);
            });
    connect(m_toggleTreeViewButton, &QPushButton::clicked, this, &MainWindow::toggleFolderTree);
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
    connect(m_textEdit, &CustomDocument::mouseMoved, this, [this]() {
        if (!m_areNonEditorWidgetsVisible) {
            setVisibilityOfFrameRightWidgets(true);
        }
#if !defined(Q_OS_MACOS)
        if (m_textEdit->verticalScrollBar()->isVisible()) {
            m_textEditScrollBarTimer->stop();
            m_textEditScrollBarTimer->start(m_textEditScrollBarTimerDuration);
        }
#endif
    });

#if defined(Q_OS_MACOS)
    connect(this, &MainWindowBase::toggleFullScreen, this, [this](bool isFullScreen) {
        if (isFullScreen) {
            ui->verticalSpacer_upSearchEdit->setMinimumHeight(0);
            ui->verticalSpacer_upSearchEdit->setMaximumHeight(0);
        } else {
            if (m_foldersWidget->isHidden()) {
                ui->verticalSpacer_upSearchEdit->setMinimumHeight(25);
                ui->verticalSpacer_upSearchEdit->setMaximumHeight(25);
            }
        }
    });
#endif
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

/*!
 * \brief MainWindow::setupSearchEdit
 * Set the lineedit to start a bit to the right and end a bit to the left (pedding)
 */
void MainWindow::setupSearchEdit()
{
    //    QLineEdit* searchEdit = m_searchEdit;

    m_searchEdit->setAttribute(Qt::WA_MacShowFocusRect, 0);

    QFont fontAwesomeIcon("Font Awesome 6 Free Solid");
#if defined(Q_OS_MACOS)
    int pointSizeOffset = 0;
#else
    int pointSizeOffset = -4;
#endif

    // clear button
    m_clearButton = new QToolButton(m_searchEdit);
    fontAwesomeIcon.setPointSize(15 + pointSizeOffset);
    m_clearButton->setStyleSheet("QToolButton { color: rgb(114, 114, 114) }");
    m_clearButton->setFont(fontAwesomeIcon);
    m_clearButton->setText(u8"\uf057"); // fa-circle-xmark
    m_clearButton->setCursor(Qt::ArrowCursor);
    m_clearButton->hide();

    // search button
    m_searchButton = new QToolButton(m_searchEdit);
    fontAwesomeIcon.setPointSize(9 + pointSizeOffset);
    m_searchButton->setStyleSheet("QToolButton { color: rgb(205, 205, 205) }");
    m_searchButton->setFont(fontAwesomeIcon);
    m_searchButton->setText(u8"\uf002"); // fa-magnifying-glass
    m_searchButton->setCursor(Qt::ArrowCursor);

    // layout
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::RightToLeft, m_searchEdit);
    layout->setContentsMargins(2, 0, 3, 0);
    layout->addWidget(m_clearButton);
    layout->addStretch();
    layout->addWidget(m_searchButton);
    m_searchEdit->setLayout(layout);

    m_searchEdit->installEventFilter(this);
}

void MainWindow::setupSubscrirptionWindow()
{
    SubscriptionStatus::registerEnum("nuttyartist.notes", 1, 0);

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    const QUrl url("qrc:/qt/qml/SubscriptionWindow.qml");
#else
    const QUrl url("qrc:/qml/SubscriptionWindow.qml");
#endif
    m_subscriptionWindowEngine.rootContext()->setContextProperty("mainWindow", this);
    m_subscriptionWindowEngine.load(url);
    QObject *rootObject = m_subscriptionWindowEngine.rootObjects().first();
    m_subscriptionWindow = qobject_cast<QWindow *>(rootObject);
    m_subscriptionWindow->hide();

    connect(this, &MainWindow::proVersionCheck, this, [this]() {
        m_buyOrManageSubscriptionAction->setVisible(true);
        if (m_isProVersionActivated) {
            m_buyOrManageSubscriptionAction->setText("&Manage Subscription...");
        } else {
            m_buyOrManageSubscriptionAction->setText("&Buy Subscription...");
        }
    });

    verifyLicenseSignalsSlots();
}

void MainWindow::setupEditorSettings()
{
    FontTypeface::registerEnum("nuttyartist.notes", 1, 0);
    FontSizeAction::registerEnum("nuttyartist.notes", 1, 0);
    EditorTextWidth::registerEnum("nuttyartist.notes", 1, 0);
    Theme::registerEnum("nuttyartist.notes", 1, 0);
    View::registerEnum("nuttyartist.notes", 1, 0);

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    QUrl source("qrc:/qt/qml/EditorSettings.qml");
#elif QT_VERSION > QT_VERSION_CHECK(5, 12, 8)
    QUrl source("qrc:/qml/EditorSettings.qml");
#else
    QUrl source("qrc:/qml/EditorSettingsQt512.qml");
#endif

    m_editorSettingsQuickView.rootContext()->setContextProperty("mainWindow", this);
    m_editorSettingsQuickView.rootContext()->setContextProperty("noteEditorLogic",
                                                                m_noteEditorLogic);
    m_editorSettingsQuickView.setSource(source);
    m_editorSettingsQuickView.setResizeMode(QQuickView::SizeViewToRootObject);
    m_editorSettingsQuickView.setFlags(Qt::FramelessWindowHint);
    m_editorSettingsQuickView.setColor(Qt::transparent);
    m_editorSettingsWidget = QWidget::createWindowContainer(&m_editorSettingsQuickView, nullptr);
#if defined(Q_OS_MACOS)
#  if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    m_editorSettingsWidget->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint
                                           | Qt::NoDropShadowWindowHint);
#  else
    m_editorSettingsWidget->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
    m_editorSettingsWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
#  endif
#elif _WIN32
    m_editorSettingsWidget->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
    m_editorSettingsWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
#else
    m_editorSettingsWidget->setWindowFlags(Qt::Tool);
    m_editorSettingsWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
#endif
    resizeAndPositionEditorSettingsWindow();
    m_editorSettingsWidget->setStyleSheet("background:transparent;");
    m_editorSettingsWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_editorSettingsWidget->hide();
    m_editorSettingsWidget->installEventFilter(this);

    QJsonObject dataToSendToView{ { "displayFont",
                                    QFont(QStringLiteral("SF Pro Text")).exactMatch()
                                            ? QStringLiteral("SF Pro Text")
                                            : QStringLiteral("Roboto") } };
    emit displayFontSet(QVariant(dataToSendToView));

#if defined(Q_OS_WINDOWS)
    emit platformSet(QVariant(QString("Windows")));
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    emit platformSet(QVariant(QString("Unix")));
#elif defined(Q_OS_MACOS)
    emit platformSet(QVariant(QString("Apple")));
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    emit qtVersionSet(QVariant(6));
#else
    emit qtVersionSet(QVariant(5));
#endif
}

/*!
 * \brief MainWindow::setCurrentFontBasedOnTypeface
 * Set the current font based on a given typeface
 */
void MainWindow::setCurrentFontBasedOnTypeface(FontTypeface::Value selectedFontTypeFace)
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

    QJsonObject dataToSendToView;
    dataToSendToView["listOfSansSerifFonts"] = QJsonArray::fromStringList(m_listOfSansSerifFonts);
    dataToSendToView["listOfSerifFonts"] = QJsonArray::fromStringList(m_listOfSerifFonts);
    dataToSendToView["listOfMonoFonts"] = QJsonArray::fromStringList(m_listOfMonoFonts);
    dataToSendToView["chosenSansSerifFontIndex"] = m_chosenSansSerifFontIndex;
    dataToSendToView["chosenSerifFontIndex"] = m_chosenSerifFontIndex;
    dataToSendToView["chosenMonoFontIndex"] = m_chosenMonoFontIndex;
    dataToSendToView["currentFontTypeface"] = to_string(m_currentFontTypeface).c_str();
    emit fontsChanged(QVariant(dataToSendToView));
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

    setCurrentFontBasedOnTypeface(m_currentFontTypeface);
    setTheme(m_currentTheme);
}

void MainWindow::setupTextEditStyleSheet(int paddingLeft, int paddingRight)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    m_textEdit->setDocumentPadding(paddingLeft, 10, paddingRight, 2);
#else
    m_textEdit->setDocumentPadding(paddingLeft, 0, paddingRight, 2);
#endif
    setCSSThemeAndUpdate(m_textEdit, m_currentTheme);
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
void MainWindow::setupKanbanView()
{
    // Using QQuickWidget -> some say it is slower but has better support?
    // Source:
    // https://doc.qt.io/qt-6/qtquick-quickwidgets-qquickwidgetversuswindow-opengl-example.html
    //    m_kanbanWidget.setParent(this);
    //    m_kanbanWidget.setSource(source);
    //    m_kanbanWidget.setResizeMode(QQuickWidget::SizeRootObjectToView);
    //    m_kanbanWidget.hide();
    //    m_kanbanWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //    ui->verticalLayout_textEdit->insertWidget(ui->verticalLayout_textEdit->indexOf(m_textEdit),
    //    &m_kanbanWidget);
    // Using QQuickView and QWindow -> some say it's faster with limitation?
    // Source:
    // https://doc.qt.io/qt-6/qtquick-quickwidgets-qquickwidgetversuswindow-opengl-example.html

    QUrl source("qrc:/qt/qml/kanbanMain.qml");
    m_kanbanQuickView.rootContext()->setContextProperty("noteEditorLogic", m_noteEditorLogic);
    m_kanbanQuickView.rootContext()->setContextProperty("mainWindow", this);
    m_kanbanQuickView.setSource(source);
    m_kanbanQuickView.setResizeMode(QQuickView::SizeRootObjectToView);
    m_kanbanWidget = QWidget::createWindowContainer(&m_kanbanQuickView);
    m_kanbanWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_kanbanWidget->hide();
    ui->verticalLayout_textEdit->insertWidget(ui->verticalLayout_textEdit->indexOf(m_textEdit),
                                              m_kanbanWidget);
#  if defined(Q_OS_WINDOWS)
    emit platformSet(QVariant(QString("Windows")));
#  elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    emit platformSet(QVariant(QString("Unix")));
#  elif defined(Q_OS_MACOS)
    emit platformSet(QVariant(QString("Apple")));
#  endif

    QJsonObject dataToSendToView{ { "displayFont",
                                    QFont(QStringLiteral("SF Pro Text")).exactMatch()
                                            ? QStringLiteral("SF Pro Text")
                                            : QStringLiteral("Roboto") } };
    emit displayFontSet(QVariant(dataToSendToView));
}
#endif

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
        int initWidth = 1106;
        int initHeight = 694;
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
 * \brief MainWindow::setActivationSuccessful
 */
void MainWindow::setActivationSuccessful(QString licenseKey, bool removeGracePeriodStartedDate)
{
    m_isProVersionActivated = true;
    emit proVersionCheck(QVariant(m_isProVersionActivated));
    m_localLicenseData->setValue(QStringLiteral("licenseKey"), licenseKey);
    if (removeGracePeriodStartedDate && m_localLicenseData->contains("gracePeriodStartedDate"))
        m_localLicenseData->remove("gracePeriodStartedDate");
    m_aboutWindow.setProVersion(m_isProVersionActivated);
}

/*!
 * \brief MainWindow::getPaymentDetails
 */
void MainWindow::getPaymentDetailsSignalsSlots()
{
    m_netPurchaseDataReplyFirstAttempt = m_netManager->get(m_reqAlt1);

    connect(m_netPurchaseDataReplyFirstAttempt, &QNetworkReply::readyRead, this,
            [this]() { m_dataBuffer->append(m_netPurchaseDataReplyFirstAttempt->readAll()); });
    connect(m_netPurchaseDataReplyFirstAttempt, &QNetworkReply::finished, this, [this]() {
        // Handle error
        if (m_netPurchaseDataReplyFirstAttempt->error() != QNetworkReply::NoError) {
            qDebug() << "Error : " << m_netPurchaseDataReplyFirstAttempt->errorString();
            qDebug() << "Failed first attempt at getting payment data. Trying second...";
            emit tryPurchaseDataSecondAlternative();
            m_netPurchaseDataReplyFirstAttempt->deleteLater();
            return;
        }

        // Handle success
        m_paymentDetails = QJsonDocument::fromJson(*m_dataBuffer).object();
        emit fetchingPaymentDetailsRemotelyFinished();
        m_netPurchaseDataReplyFirstAttempt->deleteLater();
    });
}

/*!
 * \brief MainWindow::getSubscriptionStatus
 */
void MainWindow::getSubscriptionStatus()
{
    QString validateLicenseEndpoint = m_paymentDetails["purchaseApiBase"].toString()
            + m_paymentDetails["validateLicenseEndpoint"].toString();
    QNetworkRequest request{ QUrl(validateLicenseEndpoint) };
    QJsonObject licenseDataObject;
    licenseDataObject["license_key"] = m_userLicenseKey;
    QJsonDocument licenseDataDoc(licenseDataObject);
    QByteArray postData = licenseDataDoc.toJson();
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_netManager->post(request, postData);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        bool skipSettingNotProOnError = false;
        bool showSubscriptionWindowWhenNotPro = true;

        if (reply->error() != QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonObject response = QJsonDocument::fromJson(responseData).object();

            if (response.isEmpty()) {
                // No Internet
                if (!m_localLicenseData->contains(QStringLiteral("gracePeriodStartedDate"))) {
                    // gracePeriodStartedDate not found, so entering new one now
                    m_localLicenseData->setValue(QStringLiteral("gracePeriodStartedDate"),
                                                 QDateTime::currentDateTime());
                    skipSettingNotProOnError = true;
                    setActivationSuccessful(m_userLicenseKey, false);
                    m_subscriptionStatus = SubscriptionStatus::EnteredGracePeriod;
                } else {
                    QDateTime gracePeriodStartedDate =
                            m_localLicenseData
                                    ->value(QStringLiteral("gracePeriodStartedDate"),
                                            QDateTime::currentDateTime())
                                    .toDateTime();
                    QDateTime dateAfterSevenDays =
                            gracePeriodStartedDate.addDays(7); // 7 days grace period offline usage
                    if (QDateTime::currentDateTime() > dateAfterSevenDays) {
                        // Show grace period is over window, you need internt connection
                        m_subscriptionStatus = SubscriptionStatus::GracePeriodOver;
                    } else {
                        qDebug() << "Inside grace period";
                        // Inside grace period - make Pro
                        m_subscriptionStatus = SubscriptionStatus::Active;
                        skipSettingNotProOnError = true;
                        setActivationSuccessful(m_userLicenseKey, false);
                    }
                }
            } else if (response.contains("valid") && response["valid"] == false
                       && response.contains("license_key")
                       && response[QStringLiteral("license_key")]
                                       .toObject()[QStringLiteral("status")]
                                       .toString()
                               == "expired") {
                // Show license expired window
                m_subscriptionStatus = SubscriptionStatus::Expired;
                showSubscriptionWindowWhenNotPro = false;
            } else if (response.contains("valid") && response["valid"] == false) {
                // Show license not valid window
                m_subscriptionStatus = SubscriptionStatus::Invalid;
            } else {
                // Handle error
                m_subscriptionStatus = SubscriptionStatus::UnknownError;
            }

            if (!skipSettingNotProOnError) {
                m_isProVersionActivated = false;
                emit proVersionCheck(QVariant(m_isProVersionActivated));
                m_aboutWindow.setProVersion(false);
            }
        } else {
            QByteArray responseData = reply->readAll();
            QJsonObject response = QJsonDocument::fromJson(responseData).object();

            if (response.contains("license_key") && response.contains("valid")
                && response["valid"] == true) {
                if (response[QStringLiteral("license_key")]
                            .toObject()[QStringLiteral("status")]
                            .toString()
                    == "inactive") {
                    int activationUsage = response[QStringLiteral("license_key")]
                                                  .toObject()[QStringLiteral("activation_usage")]
                                                  .toInt();
                    int activationLimit = response[QStringLiteral("license_key")]
                                                  .toObject()[QStringLiteral("activation_limit")]
                                                  .toInt();
                    if (activationUsage >= activationLimit) {
                        // Over activation limit
                        m_subscriptionStatus = SubscriptionStatus::ActivationLimitReached;
                        m_isProVersionActivated = false;
                        emit proVersionCheck(QVariant(m_isProVersionActivated));
                        m_aboutWindow.setProVersion(false);
                        showSubscriptionWindowWhenNotPro = false;
                    } else {
                        // License valid but not activated yet. Activate
                        // TODO: verify against some device ID
                        QString activateLicenseEndpoint =
                                m_paymentDetails["purchaseApiBase"].toString()
                                + m_paymentDetails["activateLicenseEndpoint"].toString();
                        QNetworkRequest requestActivate{ QUrl(activateLicenseEndpoint) };
                        QJsonObject licenseDataObject2;
                        licenseDataObject2["license_key"] = m_userLicenseKey;
                        licenseDataObject2["instance_name"] = "Notes_Pro";
                        QJsonDocument licenseDataDoc2(licenseDataObject2);
                        QByteArray postData2 = licenseDataDoc2.toJson();
                        requestActivate.setHeader(QNetworkRequest::ContentTypeHeader,
                                                  "application/json");
                        m_netManager->post(requestActivate, postData2);

                        m_subscriptionStatus = SubscriptionStatus::Active;
                        setActivationSuccessful(m_userLicenseKey);
                    }
                } else if (response[QStringLiteral("license_key")]
                                   .toObject()[QStringLiteral("status")]
                                   .toString()
                           == "active") {
                    // Lincense is active
                    // TODO: verify against device ID as well
                    m_subscriptionStatus = SubscriptionStatus::Active;
                    setActivationSuccessful(m_userLicenseKey);
                }
            } else {
                // Handle error
                m_subscriptionStatus = SubscriptionStatus::UnknownError;
                m_isProVersionActivated = false;
                emit proVersionCheck(QVariant(m_isProVersionActivated));
                m_aboutWindow.setProVersion(false);
            }
        }

        qDebug() << "m_subscriptionStatus: " << m_subscriptionStatus;
        emit subscriptionStatusChanged(QVariant(m_subscriptionStatus));

        if (!m_isProVersionActivated && showSubscriptionWindowWhenNotPro)
            m_subscriptionWindow->show();
    });
}

/*!
 * \brief MainWindow::verifyLicenseSignalsSlots
 */
void MainWindow::verifyLicenseSignalsSlots()
{
    connect(this, &MainWindow::tryPurchaseDataSecondAlternative, this, [this]() {
        m_dataBuffer->clear();
        m_netPurchaseDataReplySecondAttempt = m_netManager->get(m_reqAlt2);

        connect(m_netPurchaseDataReplySecondAttempt, &QNetworkReply::readyRead, this,
                [this]() { m_dataBuffer->append(m_netPurchaseDataReplySecondAttempt->readAll()); });
        connect(m_netPurchaseDataReplySecondAttempt, &QNetworkReply::finished, this, [this]() {
            // Handle success
            if (m_netPurchaseDataReplySecondAttempt->error() == QNetworkReply::NoError) {
                m_paymentDetails = QJsonDocument::fromJson(*m_dataBuffer).object();
            } else {
                // Handle error - ignore and use defaulte hard-coded/embedded payment data
                qDebug() << "Failed second attempt at getting payment data. Using default embedded "
                            "payment data...";
            }
            emit fetchingPaymentDetailsRemotelyFinished();
            m_netPurchaseDataReplySecondAttempt->deleteLater();
        });
    });

    connect(this, &MainWindow::fetchingPaymentDetailsRemotelyFinished, this, [this]() {
        if (m_paymentDetails.isEmpty()) {
            qDebug() << "Using default embedded payment data";
            QJsonObject paymentDetailsDefault;
            paymentDetailsDefault["purchase_pro_url"] = "https://www.get-notes.com/pricing";
            paymentDetailsDefault["purchaseApiBase"] = "https://api.lemonsqueezy.com";
            paymentDetailsDefault["activateLicenseEndpoint"] = "/v1/licenses/activate";
            paymentDetailsDefault["validateLicenseEndpoint"] = "/v1/licenses/validate";
            paymentDetailsDefault["deactivateLicenseEndpoint"] = "/v1/licenses/deactivate";
            m_paymentDetails = paymentDetailsDefault;
        }
        emit gettingPaymentDetailsFinished();
    });

    connect(this, &MainWindow::gettingPaymentDetailsFinished, this,
            [this]() { getSubscriptionStatus(); });
}

/*!
 * \brief MainWindow::checkProVersion
 */
void MainWindow::checkProVersion()
{
#if defined(PRO_VERSION)
    m_isProVersionActivated = true;
    emit proVersionCheck(QVariant(m_isProVersionActivated));
    m_aboutWindow.setProVersion(m_isProVersionActivated);
#else
    m_userLicenseKey = m_localLicenseData->value(QStringLiteral("licenseKey"), "NULL").toString();
    if (m_userLicenseKey != "NULL") {
        m_dataBuffer->clear();

        m_netPurchaseDataReplyFirstAttempt = m_netManager->get(m_reqAlt1);

        connect(m_netPurchaseDataReplyFirstAttempt, &QNetworkReply::readyRead, this,
                [this]() { m_dataBuffer->append(m_netPurchaseDataReplyFirstAttempt->readAll()); });

        connect(m_netPurchaseDataReplyFirstAttempt, &QNetworkReply::finished, this, [this]() {
            if (m_netPurchaseDataReplyFirstAttempt->error() != QNetworkReply::NoError) {
                qDebug() << "Error : " << m_netPurchaseDataReplyFirstAttempt->errorString();
                qDebug() << "Failed first attempt at getting data. Trying second...";
                emit tryPurchaseDataSecondAlternative();
                m_netPurchaseDataReplyFirstAttempt->deleteLater();
                return;
            }

            m_paymentDetails = QJsonDocument::fromJson(*m_dataBuffer).object();
            emit fetchingPaymentDetailsRemotelyFinished();
            m_netPurchaseDataReplyFirstAttempt->deleteLater();
        });
    } else {
        m_isProVersionActivated = false;
        emit proVersionCheck(QVariant(m_isProVersionActivated));
        m_aboutWindow.setProVersion(m_isProVersionActivated);
    }
#endif
}

QVariant MainWindow::getUserLicenseKey()
{
    return QVariant(m_userLicenseKey);
}

void MainWindow::openSubscriptionWindow()
{
    m_subscriptionWindow->show();
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

#if !defined(PRO_VERSION)
#  if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    m_localLicenseData =
            new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                          QStringLiteral("Awesomeness"), QStringLiteral(".notesLicenseData"), this);
#  else
    m_localLicenseData =
            new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                          QStringLiteral("Awesomeness"), QStringLiteral("notesLicenseData"), this);
#  endif
#endif

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
    m_treeViewLogic = new TreeViewLogic(m_treeView, m_treeModel, m_dbManager, m_listView, this);
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    m_noteEditorLogic = new NoteEditorLogic(
            m_textEdit, m_editorDateLabel, m_searchEdit, m_kanbanWidget,
            static_cast<TagListView *>(ui->tagListView), m_tagPool, m_dbManager, this);
    m_kanbanQuickView.rootContext()->setContextProperty("noteEditorLogic", m_noteEditorLogic);
#else
    m_noteEditorLogic = new NoteEditorLogic(m_textEdit, m_editorDateLabel, m_searchEdit,
                                            static_cast<TagListView *>(ui->tagListView), m_tagPool,
                                            m_dbManager, this);
#endif
    m_editorSettingsQuickView.rootContext()->setContextProperty("noteEditorLogic",
                                                                m_noteEditorLogic);
}

/*!
 * \brief MainWindow::restoreStates
 * Restore the latest states (if there are any) of the window and the splitter from
 * the settings database
 */
void MainWindow::restoreStates()
{
#if defined(Q_OS_MACOS)
    bool nativeByDefault = false;
#else
    bool nativeByDefault = true;
#endif
    setUseNativeWindowFrame(
            m_settingsDatabase->value(QStringLiteral("useNativeWindowFrame"), nativeByDefault)
                    .toBool());

    setHideToTray(m_settingsDatabase->value(QStringLiteral("hideToTray"), true).toBool());
    if (m_hideToTray) {
        setupTrayIcon();
    }

    if (m_settingsDatabase->value(QStringLiteral("windowGeometry"), "NULL") != "NULL")
        restoreGeometry(m_settingsDatabase->value(QStringLiteral("windowGeometry")).toByteArray());

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

    if (m_settingsDatabase->contains(QStringLiteral("splitterSizes"))) {
        m_splitter->restoreState(
                m_settingsDatabase->value(QStringLiteral("splitterSizes")).toByteArray());
        // in rare cases, the splitter sizes can be zero, which causes bugs (issue #531)
        auto splitterSizes = m_splitter->sizes();
        splitterSizes[0] = std::max(splitterSizes[0], m_foldersWidget->minimumWidth());
        splitterSizes[1] = std::max(splitterSizes[1], m_noteListWidget->minimumWidth());
        m_splitter->setSizes(splitterSizes);
    }

    m_foldersWidget->setHidden(
            m_settingsDatabase->value(QStringLiteral("isTreeCollapsed")).toBool());
    m_noteListWidget->setHidden(
            m_settingsDatabase->value(QStringLiteral("isNoteListCollapsed")).toBool());

#if defined(Q_OS_MACOS)
    if (m_foldersWidget->isHidden()) {
        ui->verticalSpacer_upSearchEdit->setMinimumHeight(25);
        ui->verticalSpacer_upSearchEdit->setMaximumHeight(25);
    }
#else
    if (!m_useNativeWindowFrame && m_foldersWidget->isHidden()) {
        ui->verticalSpacer_upSearchEdit->setMinimumHeight(25);
        ui->verticalSpacer_upSearchEdit->setMaximumHeight(25);
    }
#endif

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
        // Default option, Focus Mode (FixedColumnWidth) or Full Width (WidgetWidth)
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

    setCurrentFontBasedOnTypeface(m_currentFontTypeface);
    setTheme(m_currentTheme);

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

    updateSelectedOptionsEditorSettings();
    updateFrame();
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
    m_searchEdit->setEnabled(doEnable);
    m_textEdit->setEnabled(doEnable);
    m_dotsButton->setEnabled(doEnable);
    m_globalSettingsButton->setEnabled(doEnable);
}

/*!
 * \brief MainWindow::setKanbanVisibility
 * \param isVisible
 * Either shows or hide the kanban view based on isVisible
 */
void MainWindow::setKanbanVisibility(bool isVisible)
{
    auto inf = m_listViewLogic->listViewInfo();
    if (isVisible && inf.parentFolderId != SpecialNodeID::TrashFolder) {
        emit m_noteEditorLogic->showKanbanView();
    } else {
        emit m_noteEditorLogic->hideKanbanView();
#if defined(Q_OS_WINDOWS)
        if (m_editorSettingsWidget->isVisible()) {
            m_editorSettingsWidget->raise();
            m_editorSettingsWidget->activateWindow();
        } else {
            m_textEdit->setFocus();
        }
#else
        m_textEdit->setFocus();
#endif
    }

    updateSelectedOptionsEditorSettings();
}

/*!
 * \brief MainWindow::setEditorSettingsFromQuickViewVisibility
 * \param isVisible
 */
void MainWindow::setEditorSettingsFromQuickViewVisibility(bool isVisible)
{
    m_isEditorSettingsFromQuickViewVisible = isVisible;
}

/*!
 * \brief MainWindow::setEditorSettingsScrollBarPosition
 * \param isVisible
 */
void MainWindow::setEditorSettingsScrollBarPosition(double position)
{
    emit editorSettingsScrollBarPositionChanged(QVariant(position));
}

/*!
 * \brief MainWindow::toggleMarkdown
 * Enable or disable markdown
 */
void MainWindow::setMarkdownEnabled(bool isMarkdownEnabled)
{
    m_noteEditorLogic->setMarkdownEnabled(isMarkdownEnabled);
    updateSelectedOptionsEditorSettings();
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

    // save the data of the previous selected
    m_noteEditorLogic->saveNoteToDB();

    if (!m_searchEdit->text().isEmpty()) {
        m_listViewLogic->clearSearch(true);
    } else {
        createNewNote();
    }
}

/*!
 * \brief MainWindow::onDotsButtonClicked
 * Open up the menu when clicking the 3 dots button
 */
void MainWindow::onDotsButtonClicked()
{
    showEditorSettings();
}

/*!
 * \brief MainWindow::onSwitchToKanbanViewButtonClicked
 */
void MainWindow::onSwitchToKanbanViewButtonClicked()
{
    setKanbanVisibility(true);
}

/*!
 * \brief MainWindow::setupGlobalSettingsMenu
 */
void MainWindow::setupGlobalSettingsMenu()
{
#if !defined(Q_OS_MACOS)
    QMenu *viewMenu = m_mainMenu.addMenu(tr("&View"));
    viewMenu->setToolTipsVisible(true);
#endif

    QMenu *importExportNotesMenu = m_mainMenu.addMenu(tr("&Import/Export Notes"));
    importExportNotesMenu->setToolTipsVisible(true);
    m_mainMenu.setToolTipsVisible(true);

    QShortcut *closeMenu = new QShortcut(Qt::Key_F10, &m_mainMenu);
    closeMenu->setContext(Qt::ApplicationShortcut);
    connect(closeMenu, &QShortcut::activated, &m_mainMenu, &QMenu::close);

#if defined(Q_OS_WINDOWS) || defined(Q_OS_WIN)
    setStyleSheet(m_styleSheet);
    setCSSClassesAndUpdate(&m_mainMenu, "menu");
#endif

#ifdef __APPLE__
    m_mainMenu.setFont(QFont(m_displayFont, 13));
    importExportNotesMenu->setFont(QFont(m_displayFont, 13));
#else
    m_mainMenu.setFont(QFont(m_displayFont, 10, QFont::Normal));
    viewMenu->setFont(QFont(m_displayFont, 10, QFont::Normal));
    importExportNotesMenu->setFont(QFont(m_displayFont, 10, QFont::Normal));
#endif

#if defined(UPDATE_CHECKER)
    // Check for update action
    QAction *checkForUpdatesAction = m_mainMenu.addAction(tr("Check For &Updates"));
    connect(checkForUpdatesAction, &QAction::triggered, this, &MainWindow::checkForUpdates);
#endif

    // Autostart
    QAction *autostartAction = m_mainMenu.addAction(tr("&Start automatically"));
    connect(autostartAction, &QAction::triggered, this,
            [=]() { m_autostart.setAutostart(autostartAction->isChecked()); });
    autostartAction->setCheckable(true);
    autostartAction->setChecked(m_autostart.isAutostart());

    // hide to tray
    QAction *hideToTrayAction = m_mainMenu.addAction(tr("&Hide to tray"));
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

    QAction *changeDBPathAction = m_mainMenu.addAction(tr("&Change database path"));
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
    QAction *aboutAction = m_mainMenu.addAction(tr("&About Notes"));
    connect(aboutAction, &QAction::triggered, this, [&]() { m_aboutWindow.show(); });

    m_mainMenu.addSeparator();

#if !defined(PRO_VERSION)
    // Buy/Manage subscription
    m_buyOrManageSubscriptionAction = m_mainMenu.addAction(
            tr(m_isProVersionActivated ? "&Manage Subscription..." : "&Buy Subscription..."));
    m_buyOrManageSubscriptionAction->setVisible(false);
    connect(m_buyOrManageSubscriptionAction, &QAction::triggered, this,
            &MainWindow::openSubscriptionWindow);

    m_mainMenu.addSeparator();
#endif

    // Close the app
    QAction *quitAppAction = m_mainMenu.addAction(tr("&Quit"));
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

#if !defined(Q_OS_MACOS)
    // Use native frame action
    QAction *useNativeFrameAction = viewMenu->addAction(tr("&Use native window frame"));
    useNativeFrameAction->setToolTip(tr("Use the window frame provided by the window manager"));
    useNativeFrameAction->setCheckable(true);
    useNativeFrameAction->setChecked(m_useNativeWindowFrame);
    connect(useNativeFrameAction, &QAction::triggered, this,
            [this]() { setUseNativeWindowFrame(!m_useNativeWindowFrame); });
#endif
}

/*!
 * \brief MainWindow::onGlobalSettingsButtonClicked
 * Open up the menu when clicking the global settings button
 */
void MainWindow::onGlobalSettingsButtonClicked()
{
    m_mainMenu.exec(
            m_globalSettingsButton->mapToGlobal(QPoint(0, m_globalSettingsButton->height())));
}

/*!
 * \brief MainWindow::showEditorSettings
 * Shows the Editor Settings popup
 */
void MainWindow::showEditorSettings()
{
    resizeAndPositionEditorSettingsWindow();
    QJsonObject dataToSendToView;
    dataToSendToView["parentWindowHeight"] = this->height();
    emit editorSettingsShowed(QVariant(dataToSendToView));
    m_editorSettingsWidget->show();
#if defined(__WIN32__) || (defined(QT_VERSION) && QT_VERSION < QT_VERSION_CHECK(6, 2, 0))
    m_editorSettingsWidget->raise();
#elif !defined(Q_OS_MACOS)
    m_editorSettingsWidget->activateWindow();
#endif
}

/*!
 * \brief MainWindow::toggleEditorSettings
 * Toggle the Editor Settings popup
 */
void MainWindow::toggleEditorSettings()
{
    if (m_editorSettingsWidget->isHidden()) {
        showEditorSettings();
    } else {
        m_editorSettingsWidget->close();
    }
}

/*!
 * \brief MainWindow::updateSelectedOptionsEditorSettings
 */
void MainWindow::updateSelectedOptionsEditorSettings()
{
    QJsonObject dataToSendToView;
    dataToSendToView["currentFontTypeface"] = to_string(m_currentFontTypeface).c_str();
    dataToSendToView["currentTheme"] = to_string(m_currentTheme).c_str();
    dataToSendToView["isTextFullWidth"] = m_textEdit->lineWrapMode() == QTextEdit::WidgetWidth;
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    dataToSendToView["currentView"] = m_kanbanWidget->isHidden() ? "TextView" : "KanbanView";
#endif
    dataToSendToView["isNoteListCollapsed"] = m_noteListWidget->isHidden();
    dataToSendToView["isFoldersTreeCollapsed"] = m_foldersWidget->isHidden();
    dataToSendToView["isMarkdownDisabled"] = !m_noteEditorLogic->markdownEnabled();
    dataToSendToView["isStayOnTop"] = m_alwaysStayOnTop;
    emit settingsChanged(QVariant(dataToSendToView));
}

/*!
 * \brief MainWindow::changeEditorFontTypeFromStyleButtons
 * Change the font based on the type passed from the Style Editor Window
 */
void MainWindow::changeEditorFontTypeFromStyleButtons(FontTypeface::Value fontTypeface,
                                                      int chosenFontIndex)
{
    if (chosenFontIndex < 0)
        return;

    switch (fontTypeface) {
    case FontTypeface::Mono:
        if (chosenFontIndex > m_listOfMonoFonts.size() - 1)
            return;
        m_chosenMonoFontIndex = chosenFontIndex;
        break;
    case FontTypeface::Serif:
        if (chosenFontIndex > m_listOfSerifFonts.size() - 1)
            return;
        m_chosenSerifFontIndex = chosenFontIndex;
        break;
    case FontTypeface::SansSerif:
        if (chosenFontIndex > m_listOfSansSerifFonts.size() - 1)
            return;
        m_chosenSansSerifFontIndex = chosenFontIndex;
        break;
    }

    setCurrentFontBasedOnTypeface(fontTypeface);

    updateSelectedOptionsEditorSettings();
}

/*!
 * \brief MainWindow::changeEditorFontSizeFromStyleButtons
 * Change the font size based on the button pressed in the Style Editor Window
 * Increase / Decrease
 */
void MainWindow::changeEditorFontSizeFromStyleButtons(FontSizeAction::Value fontSizeAction)
{
    switch (fontSizeAction) {
    case FontSizeAction::FontSizeIncrease:
        m_editorMediumFontSize += 1;
        setCurrentFontBasedOnTypeface(m_currentFontTypeface);
        break;
    case FontSizeAction::FontSizeDecrease:
        m_editorMediumFontSize -= 1;
        setCurrentFontBasedOnTypeface(m_currentFontTypeface);
        break;
    }
    setTheme(m_currentTheme);
}

/*!
 * \brief MainWindow::changeEditorTextWidthFromStyleButtons
 * Change the text width of the text editor
 * FullWidth / Increase / Decrease
 */
void MainWindow::changeEditorTextWidthFromStyleButtons(EditorTextWidth::Value editorTextWidth)
{
    switch (editorTextWidth) {
    case EditorTextWidth::TextWidthFullWidth:
        if (m_textEdit->lineWrapMode() != QTextEdit::WidgetWidth)
            m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
        else
            m_textEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
        break;
    case EditorTextWidth::TextWidthIncrease:
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
    case EditorTextWidth::TextWidthDecrease:
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

    updateSelectedOptionsEditorSettings();
}

/*!
 * \brief MainWindow::setTheme
 * Changes the app theme
 */
void MainWindow::setTheme(Theme::Value theme)
{
    m_currentTheme = theme;

    setCSSThemeAndUpdate(this, theme);
    setCSSThemeAndUpdate(ui->verticalSpacer_upSearchEdit, theme);
    setCSSThemeAndUpdate(ui->verticalSpacer_upSearchEdit2, theme);
    setCSSThemeAndUpdate(ui->listviewLabel1, theme);
    setCSSThemeAndUpdate(ui->searchEdit, theme);
    setCSSThemeAndUpdate(ui->verticalSplitterLine_left, theme);
    setCSSThemeAndUpdate(ui->verticalSplitterLine_middle, theme);
    setCSSThemeAndUpdate(ui->frameLeft, m_currentTheme);
    setCSSThemeAndUpdate(ui->frameRight, m_currentTheme);
    setCSSThemeAndUpdate(ui->frameRightTop, m_currentTheme);

    switch (theme) {
    case Theme::Light: {
        QJsonObject themeData{ { "theme", QStringLiteral("Light") },
                               { "backgroundColor", "#f7f7f7" } };
        emit themeChanged(QVariant(themeData));
        m_currentEditorTextColor = QColor(26, 26, 26);
        m_searchButton->setStyleSheet("QToolButton { color: rgb(205, 205, 205) }");
        m_clearButton->setStyleSheet("QToolButton { color: rgb(114, 114, 114) }");
        m_switchToTextViewButton->setStyleSheet("QPushButton { color: rgb(215, 214, 213); }");
        break;
    }
    case Theme::Dark: {
        QJsonObject themeData{ { "theme", QStringLiteral("Dark") },
                               { "backgroundColor", "#191919" } };
        emit themeChanged(QVariant(themeData));
        m_currentEditorTextColor = QColor(212, 212, 212);
        m_searchButton->setStyleSheet("QToolButton { color: rgb(68, 68, 68) }");
        m_clearButton->setStyleSheet("QToolButton { color: rgb(147, 144, 147) }");
        m_switchToTextViewButton->setStyleSheet("QPushButton { color: rgb(37, 48, 66); }");
        break;
    }
    case Theme::Sepia: {
        QJsonObject themeData{ { "theme", QStringLiteral("Sepia") },
                               { "backgroundColor", "#fbf0d9" } };
        emit themeChanged(QVariant(themeData));
        m_currentEditorTextColor = QColor(50, 30, 3);
        m_searchButton->setStyleSheet("QToolButton { color: rgb(205, 205, 205) }");
        m_clearButton->setStyleSheet("QToolButton { color: rgb(114, 114, 114) }");
        m_switchToTextViewButton->setStyleSheet("QPushButton { color: rgb(215, 214, 213); }");
        break;
    }
    }
    setupTextEditStyleSheet(m_noteEditorLogic->currentMinimumEditorPadding(),
                            m_noteEditorLogic->currentMinimumEditorPadding());
    m_noteEditorLogic->setTheme(theme, m_currentEditorTextColor, m_editorMediumFontSize);
    m_listViewLogic->setTheme(theme);
    m_aboutWindow.setTheme(theme);
    m_treeViewLogic->setTheme(theme);
    ui->tagListView->setTheme(theme);

    alignTextEditText();

    updateSelectedOptionsEditorSettings();
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
 * before and after the selection. If nothing is selected, insert formatting char(s) before/after
 * the word under the cursor
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
    }

    m_noteEditorLogic->saveNoteToDB();

#if defined(UPDATE_CHECKER)
    m_settingsDatabase->setValue(QStringLiteral("dontShowUpdateWindow"), m_dontShowUpdateWindow);
#endif
    m_settingsDatabase->setValue(QStringLiteral("splitterSizes"), m_splitter->saveState());

    m_settingsDatabase->setValue(QStringLiteral("isTreeCollapsed"), m_foldersWidget->isHidden());
    m_settingsDatabase->setValue(QStringLiteral("isNoteListCollapsed"),
                                 m_noteListWidget->isHidden());

    m_settingsDatabase->setValue(QStringLiteral("selectedFontTypeface"),
                                 to_string(m_currentFontTypeface).c_str());
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
    m_settingsDatabase->setValue(QStringLiteral("theme"), to_string(m_currentTheme).c_str());
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
    updateSelectedOptionsEditorSettings();
}

void MainWindow::collapseFolderTree()
{
    m_toggleTreeViewButton->setText(u8"\ue31c"); // keyboard_tab
    m_foldersWidget->setHidden(true);
    updateFrame();
    updateSelectedOptionsEditorSettings();
#if defined(Q_OS_MACOS)
    if (windowState() != Qt::WindowFullScreen) {
        ui->verticalSpacer_upSearchEdit->setMinimumHeight(25);
        ui->verticalSpacer_upSearchEdit->setMaximumHeight(25);
    }
#else
    if (!m_useNativeWindowFrame && windowState() != Qt::WindowFullScreen) {
        ui->verticalSpacer_upSearchEdit->setMinimumHeight(25);
        ui->verticalSpacer_upSearchEdit->setMaximumHeight(25);
    }
#endif
}

void MainWindow::expandFolderTree()
{
    m_toggleTreeViewButton->setText(u8"\uec73"); // keyboard_tab_rtl
    m_foldersWidget->setHidden(false);
    updateFrame();
    updateSelectedOptionsEditorSettings();
#if defined(Q_OS_MACOS)
    ui->verticalSpacer_upSearchEdit->setMinimumHeight(0);
    ui->verticalSpacer_upSearchEdit->setMaximumHeight(0);
#else
    if (!m_useNativeWindowFrame) {
        ui->verticalSpacer_upSearchEdit->setMinimumHeight(0);
        ui->verticalSpacer_upSearchEdit->setMaximumHeight(0);
    }
#endif
}

void MainWindow::toggleNoteList()
{
    if (m_noteListWidget->isHidden()) {
        expandNoteList();
    } else {
        collapseNoteList();
    }
    updateSelectedOptionsEditorSettings();
}

void MainWindow::collapseNoteList()
{
    m_noteListWidget->setHidden(true);
    updateFrame();
    updateSelectedOptionsEditorSettings();
}

void MainWindow::expandNoteList()
{
    m_noteListWidget->setHidden(false);
    updateFrame();
    updateSelectedOptionsEditorSettings();
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
        QuitApplication();
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

/*!
 * \brief MainWindow::moveEvent
 * \param event
 */
void MainWindow::moveEvent(QMoveEvent *event)
{
    QJsonObject dataToSendToView{ { "parentWindowX", x() }, { "parentWindowY", y() } };
    emit mainWindowMoved(QVariant(dataToSendToView));

    event->accept();
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
 * \brief MainWindow::setVisibilityOfFrameRightWidgets
 * Either show or hide all widgets which are not m_textEdit
 */
void MainWindow::setVisibilityOfFrameRightWidgets(bool isVisible)
{
    m_isFrameRightTopWidgetsVisible = isVisible;
    m_areNonEditorWidgetsVisible = isVisible;

    m_editorDateLabel->setVisible(isVisible);
    m_dotsButton->setVisible(isVisible);

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    m_switchToTextViewButton->setVisible(isVisible);
    m_switchToKanbanViewButton->setVisible(isVisible);
#endif

    // If the notes list is collapsed, hide the window buttons
    if (m_splitter) {
        if (m_foldersWidget->isHidden() && m_noteListWidget->isHidden()) {
            setWindowButtonsVisible(false);
        }
    }
}

/*!
 * \brief MainWindow::setVisibilityOfFrameRightNonEditor
 * Either show or hide all widgets which are not m_textEdit
 */
void MainWindow::setVisibilityOfFrameRightNonEditor(bool isVisible)
{
    ui->frameRightTop->setVisible(isVisible);
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
 * Mostly take care on the event happened on widget whose filter installed to the mainwindow
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
        }

        if (object == ui->frame) {
            ui->centralWidget->setCursor(Qt::ArrowCursor);
        }

#if !defined(Q_OS_MACOS)
        if (object == m_textEdit->verticalScrollBar()) {
            m_textEditScrollBarTimer->stop();
        }
#endif

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
        }
        break;
    }
    case QEvent::ActivationChange: {
        if (m_editorSettingsWidget->isHidden()) {
            QApplication::setActiveWindow(
                    this); // TODO: The docs say this function is deprecated but it's the only one
                           // that works in returning the user input from m_editorSettingsWidget
                           // Qt::Popup
            m_textEdit->setFocus();
        }
        break;
    }
    case QEvent::WindowDeactivate: {
#if !defined(Q_OS_MACOS)
        if (object == m_editorSettingsWidget) {
            m_editorSettingsWidget->close();
        }
#endif

        m_canMoveWindow = false;
        m_canStretchWindow = false;
        QApplication::restoreOverrideCursor();

#ifndef _WIN32
        m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/unfocusedButton")));
        m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/unfocusedButton")));
        m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/unfocusedButton")));
#endif
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
            auto inf = m_listViewLogic->listViewInfo();
            if (!m_isOperationRunning) {
                if (m_listModel->rowCount() == 0) {
                    if (!m_searchEdit->text().isEmpty()) {
                        m_listViewLogic->clearSearch(true);
                    } else {
                        if (inf.parentFolderId != SpecialNodeID::TrashFolder) {
                            createNewNote();
                        }
                    }
                }
            }
            m_listView->setCurrentRowActive(true);
            if (inf.parentFolderId == SpecialNodeID::TrashFolder) {
                m_textEdit->setReadOnly(true);
            } else {
                m_textEdit->setFocus();
                m_textEdit->setReadOnly(false);
            }
        }
        break;
    }
    case QEvent::FocusOut: {
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
        } else if (keyEvent->key() == Qt::Key_Escape && isFullScreen()) {
            // exit fullscreen
            fullscreenWindow();
        }
        break;
    }
    case QEvent::MouseMove: {
        if (object == m_textEdit) {
            if (!m_areNonEditorWidgetsVisible) {
                setVisibilityOfFrameRightWidgets(true);
            }
#if !defined(Q_OS_MACOS)
            if (m_textEdit->verticalScrollBar()->isVisible()) {
                m_textEditScrollBarTimer->stop();
                m_textEditScrollBarTimer->start(m_textEditScrollBarTimerDuration);
            }
#endif
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

#if defined(Q_OS_MACOS)
    setWindowAlwaysOnTopMac(checked);
    updateSelectedOptionsEditorSettings();
#endif
}

/*!
 * \brief MainWindow::moveCurrentNoteToTrash
 */
void MainWindow::moveCurrentNoteToTrash()
{
    m_noteEditorLogic->deleteCurrentNote();
    //    m_editorSettingsWidget->close();
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
 * \brief MainWindow::setUseNativeWindowFrame
 * \param useNativeWindowFrame
 */
void MainWindow::setUseNativeWindowFrame(bool useNativeWindowFrame)
{
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

    setMainWindowVisibility(true);
}

void MainWindow::setHideToTray(bool enabled)
{
    m_hideToTray = enabled;
    m_settingsDatabase->setValue(QStringLiteral("hideToTray"), enabled);
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
    int titleBarHeight = ui->globalSettingsButton->height();

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

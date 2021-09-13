/**************************************************************************************
* We believe in the power of notes to help us record ideas and thoughts.
* We want people to have an easy, beautiful and simple way of doing that.
* And so we have Notes.
***************************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "notewidgetdelegate.h"
#include "qxtglobalshortcut.h"
#include "updaterwindow.h"

#include <QScrollBar>
#include <QShortcut>
#include <QTextStream>
#include <QScrollArea>
#include <QtConcurrent>
#include <QProgressDialog>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QList>
#include <QWidgetAction>

#define FIRST_LINE_MAX 80

/*!
 * \brief MainWindow::MainWindow
 * \param parent
 */
#if defined(Q_OS_LINUX)
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow (parent),
    #else
MainWindow::MainWindow(QWidget *parent) :
    CFramelessWindow (parent),
#endif
    ui (new Ui::MainWindow),
    m_autoSaveTimer(new QTimer(this)),
    m_settingsDatabase(Q_NULLPTR),
    m_clearButton(Q_NULLPTR),
    m_greenMaximizeButton(Q_NULLPTR),
    m_redCloseButton(Q_NULLPTR),
    m_yellowMinimizeButton(Q_NULLPTR),
    m_trafficLightLayout(Q_NULLPTR),
    m_newNoteButton(Q_NULLPTR),
    m_trashButton(Q_NULLPTR),
    m_dotsButton(Q_NULLPTR),
    m_styleEditorButton(Q_NULLPTR),
    m_textEdit(Q_NULLPTR),
    m_searchEdit(Q_NULLPTR),
    m_editorDateLabel(Q_NULLPTR),
    m_splitter(Q_NULLPTR),
    m_trayIcon(new QSystemTrayIcon(this)),
    m_restoreAction(new QAction(tr("&Hide Notes"), this)),
    m_quitAction(new QAction(tr("&Quit"), this)),
    m_trayIconMenu(new QMenu(this)),
    m_noteView(Q_NULLPTR),
    m_noteModel(new NoteModel(this)),
    m_deletedNotesModel(new NoteModel(this)),
    m_proxyModel(new QSortFilterProxyModel(this)),
    m_dbManager(Q_NULLPTR),
    m_dbThread(Q_NULLPTR),
    m_styleEditorWindow(this),
    m_aboutWindow(this),
    m_noteCounter(0),
    m_trashCounter(0),
    m_layoutMargin(10),
    m_shadowWidth(10),
    m_noteListWidth(200),
    m_smallEditorWidth(420),
    m_largeEditorWidth(1250),
    m_canMoveWindow(false),
    m_canStretchWindow(false),
    m_isTemp(false),
    m_isListViewScrollBarHidden(true),
    m_isContentModified(false),
    m_isOperationRunning(false),
    m_dontShowUpdateWindow(false),
    m_alwaysStayOnTop(false),
    m_useNativeWindowFrame(false),
    m_listOfSerifFonts({QStringLiteral("Trykker"), QStringLiteral("PT Serif"), QStringLiteral("Mate")}),
    m_listOfSansSerifFonts({QStringLiteral("Source Sans Pro"), QStringLiteral("Roboto"), QStringLiteral("Jost")}),
    m_listOfMonoFonts({QStringLiteral("iA Writer Mono S"), QStringLiteral("iA Writer Duo S"), QStringLiteral("iA Writer Quattro S")}),
    m_chosenSerifFontIndex(0),
    m_chosenSansSerifFontIndex(0),
    m_chosenMonoFontIndex(0),
    m_currentCharsLimitPerFont({ 64    // Mono    TODO: is this the proper way to initialize?
                               , 80    // Serif
                               , 80}), // SansSerif
    m_currentFontTypeface(FontTypeface::SansSerif),
#ifdef __APPLE__
    m_displayFont(QFont(QStringLiteral("SF Pro Text")).exactMatch() ? QStringLiteral("SF Pro Text") : QStringLiteral("Roboto")),
#elif _WIN32
    m_displayFont(QFont(QStringLiteral("Segoe UI")).exactMatch() ? QStringLiteral("Segoe UI") : QStringLiteral("Roboto")),
#else
    m_displayFont(QStringLiteral("Roboto")),
#endif
    m_currentEditorBackgroundColor(247, 247, 247),
    m_currentRightFrameColor(247, 247, 247),
    m_currentTheme(Theme::Light),
    m_currentEditorTextColor(247, 247, 247),
    m_currentThemeBackgroundColor(247, 247, 247),
    m_areNonEditorWidgetsVisible(true),
    m_isFrameRightTopWidgetsVisible(true)
{
    ui->setupUi(this);
    setupMainWindow();
    setupFonts();
    setupTrayIcon();
    setupKeyboardShortcuts();
    setupNewNoteButtonAndTrahButton();
    setupSplitter();
    setupLine();
    setupRightFrame();
    setupTitleBarButtons();
    setupSearchEdit();
    setupTextEdit();
    setupDatabases();
    setupModelView();
    restoreStates();
    setupSignalsSlots();
    autoCheckForUpdates();

    m_highlighter = new MarkdownHighlighter(m_textEdit->document());

    QTimer::singleShot(200,this, SLOT(InitData()));
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

    bool exist = (QFile::exists(oldNoteDBPath) || QFile::exists(oldTrashDBPath));

    if(exist){
        QProgressDialog* pd = new QProgressDialog(tr("Migrating database, please wait."), QString(), 0, 0, this);
        pd->setCancelButton(Q_NULLPTR);
        pd->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        pd->setMinimumDuration(0);
        pd->show();

        setButtonsAndFieldsEnabled(false);

        QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
        connect(watcher, &QFutureWatcher<void>::finished, this, [&, pd](){
            pd->deleteLater();
            setButtonsAndFieldsEnabled(true);
            emit requestNotesList();
        });

        QFuture<void> migration = QtConcurrent::run(this, &MainWindow::checkMigration);
        watcher->setFuture(migration);

    } else {
        emit requestNotesList();
    }

    /// Check if it is running with an argument (ex. hide)
    if (qApp->arguments().contains(QStringLiteral("--autostart"))) {
        setMainWindowVisibility(false);
    }
}

/*!
 * \brief MainWindow::setMainWindowVisibility
 * \param state
 */
void MainWindow::setMainWindowVisibility(bool state)
{
    if(state){
        show();
        qApp->processEvents();
        qApp->setActiveWindow(this);
        qApp->processEvents();
        m_restoreAction->setText(tr("&Hide Notes"));
    }else{
        m_restoreAction->setText(tr("&Show Notes"));
        hide();
    }
}

/*!
 * \brief MainWindow::paintEvent
 * \param event
 */
void MainWindow::paintEvent(QPaintEvent* event)
{
#if !defined(__APPLE__) && !defined(Q_OS_LINUX)
    if (!m_useNativeWindowFrame) {
        QPainter painter(this);
        painter.save();

        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);

        dropShadow(painter, ShadowType::Linear, ShadowSide::Left  );
        dropShadow(painter, ShadowType::Linear, ShadowSide::Top   );
        dropShadow(painter, ShadowType::Linear, ShadowSide::Right );
        dropShadow(painter, ShadowType::Linear, ShadowSide::Bottom);

        dropShadow(painter, ShadowType::Radial, ShadowSide::TopLeft    );
        dropShadow(painter, ShadowType::Radial, ShadowSide::TopRight   );
        dropShadow(painter, ShadowType::Radial, ShadowSide::BottomRight);
        dropShadow(painter, ShadowType::Radial, ShadowSide::BottomLeft );

        painter.restore();
    }
#endif

    QMainWindow::paintEvent(event);
}

/*!
 * \brief MainWindow::resizeEvent
 * \param event
 */
void MainWindow::resizeEvent(QResizeEvent* event)
{
    if(m_splitter != Q_NULLPTR){
        //restore note list width
        QList<int> sizes = m_splitter->sizes();
        if(sizes.at(0) != 0){
            sizes[0] = m_noteListWidth;
            sizes[1] = m_splitter->width() - m_noteListWidth;
            m_splitter->setSizes(sizes);
        }
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
#if defined(Q_OS_LINUX)
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
//    this->setAttribute(Qt::WA_TranslucentBackground);
#elif _WIN32
    this->setWindowFlags(Qt::CustomizeWindowHint);
#endif

    m_greenMaximizeButton = new QPushButton(this);
    m_redCloseButton = new QPushButton(this);
    m_yellowMinimizeButton = new QPushButton(this);
#ifndef __APPLE__
//    If we want to align window buttons with searchEdit and notesList
//    QSpacerItem *horizontialSpacer = new QSpacerItem(3, 0, QSizePolicy::Minimum, QSizePolicy::Minimum);
//    m_trafficLightLayout.addSpacerItem(horizontialSpacer);
    m_trafficLightLayout.addWidget(m_redCloseButton);
    m_trafficLightLayout.addWidget(m_yellowMinimizeButton);
    m_trafficLightLayout.addWidget(m_greenMaximizeButton);
#else
    this->setCloseBtnQuit(false);
    m_layoutMargin = 0;
    m_greenMaximizeButton->setVisible(false);
    m_redCloseButton->setVisible(false);
    m_yellowMinimizeButton->setVisible(false);
#endif

#ifdef _WIN32
    m_trafficLightLayout.setSpacing(0);
    m_trafficLightLayout.setMargin(0);
    m_trafficLightLayout.setGeometry(QRect(2,2,90,16));
#endif

    m_newNoteButton = ui->newNoteButton;
    m_trashButton = ui->trashButton;
    m_dotsButton = ui->dotsButton;
    m_styleEditorButton = ui->styleEditorButton;
    m_searchEdit = ui->searchEdit;
    m_textEdit = ui->textEdit;
    m_editorDateLabel = ui->editorDateLabel;
    m_splitter = ui->splitter;

#if defined(Q_OS_LINUX)
//    QMargins margins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin);
//    setMargins(margins);
    setMargins(QMargins(0,0,0,0));
#elif _WIN32
    ui->verticalSpacer->changeSize(0, 7, QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->verticalSpacer_upEditorDateLabel->changeSize(0, 27, QSizePolicy::Fixed, QSizePolicy::Fixed);
#endif
    ui->frame->installEventFilter(this);
    ui->frameRight->setMouseTracking(true);
    ui->centralWidget->setMouseTracking(true);
    ui->frame->setMouseTracking(true);
    ui->frameRightTop->setMouseTracking(true);
    this->setMouseTracking(true);

    QPalette pal(palette());
    pal.setColor(QPalette::Background, QColor(248, 248, 248));
    this->setAutoFillBackground(true);
    this->setPalette(pal);

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
}

/*!
 * \brief MainWindow::setupFonts
 */
void MainWindow::setupFonts()
{
#ifdef __APPLE__
    m_searchEdit->setFont(QFont(m_displayFont, 12));
    m_editorDateLabel->setFont(QFont(m_displayFont, 12, 65));
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
    m_trayIconMenu->addAction(m_restoreAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_quitAction);

    QIcon icon(QStringLiteral(":images/notes_system_tray_icon.png"));
    m_trayIcon->setIcon(icon);
    m_trayIcon->setContextMenu(m_trayIconMenu);
    m_trayIcon->show();
}

/*!
 * \brief MainWindow::setupKeyboardShortcuts
 * Setting up the keyboard shortcuts
 */
void MainWindow::setupKeyboardShortcuts()
{
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_N), this, SLOT(onNewNoteButtonClicked()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D), this, SLOT(deleteSelectedNote()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), m_searchEdit, SLOT(setFocus()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_E), m_searchEdit, SLOT(clear()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this, SLOT(setFocusOnCurrentNote()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down), this, SLOT(selectNoteDown()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Up), this, SLOT(selectNoteUp()));
    new QShortcut(QKeySequence(Qt::Key_Down), this, SLOT(selectNoteDown()));
    new QShortcut(QKeySequence(Qt::Key_Up), this, SLOT(selectNoteUp()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this, SLOT(setFocusOnText()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return), this, SLOT(setFocusOnText()));
    //new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(setFocusOnText()));
    //new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(setFocusOnText()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F), this, SLOT(fullscreenWindow()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L), this, SLOT(maximizeWindow()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_M), this, SLOT(minimizeWindow()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(QuitApplication()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_K), this, SLOT(toggleStayOnTop()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_J), this, SLOT(toggleNoteList()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S), this, SLOT(onStyleEditorButtonClicked()));

    QxtGlobalShortcut *shortcut = new QxtGlobalShortcut(this);
#if defined(Q_OS_LINUX)
    shortcut->setShortcut(QKeySequence(QStringLiteral("META+SHIFT+N")));
#else
    shortcut->setShortcut(QKeySequence(QStringLiteral("META+N")));
#endif
    connect(shortcut, &QxtGlobalShortcut::activated,[=]() {
        // workaround prevent textEdit and searchEdit
        // from taking 'N' from shortcut
        m_textEdit->setDisabled(true);
        m_searchEdit->setDisabled(true);
        setMainWindowVisibility(isHidden()
                                || windowState() == Qt::WindowMinimized
                                || qApp->applicationState() == Qt::ApplicationInactive);
        if(isHidden()
                || windowState() == Qt::WindowMinimized
                || qApp->applicationState() == Qt::ApplicationInactive)
#ifdef __APPLE__
            this->raise();
#else
            this->activateWindow();
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
}

/*!
 * \brief MainWindow::setupLine
 * Set up the vertical line that seperate between the scrollArea to the textEdit
 */
void MainWindow::setupLine()
{
#ifdef __APPLE__
    ui->line->setStyleSheet(QStringLiteral("border: 0px solid rgb(221, 221, 221)"));
#else
    ui->line->setStyleSheet(QStringLiteral("border: 1px solid rgb(221, 221, 221)"));
#endif
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
                 "}").arg(m_currentRightFrameColor.name());
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
    connect(&m_updater, &UpdaterWindow::dontShowUpdateWindowChanged, [=](bool state){m_dontShowUpdateWindow = state;});
    // Style Editor Window
    connect(&m_styleEditorWindow, &StyleEditorWindow::changeFontType, this, [=](FontTypeface fontType){changeEditorFontTypeFromStyleButtons(fontType);});
    connect(&m_styleEditorWindow, &StyleEditorWindow::changeFontSize, this, [=](FontSizeAction fontSizeAction){changeEditorFontSizeFromStyleButtons(fontSizeAction);});
    connect(&m_styleEditorWindow, &StyleEditorWindow::changeEditorTextWidth, this, [=](EditorTextWidth editorTextWidth){changeEditorTextWidthFromStyleButtons(editorTextWidth);});
    connect(&m_styleEditorWindow, &StyleEditorWindow::resetEditorToDefaultSettings, this, [=](){resetEditorToDefaultSettings();});
    connect(&m_styleEditorWindow, &StyleEditorWindow::changeTheme, this, [=](Theme theme){setTheme(theme);});
    // actions
    // connect(rightToLeftActionion, &QAction::triggered, this, );
    //connect(checkForUpdatesAction, &QAction::triggered, this, );
    // green button
    connect(m_greenMaximizeButton, &QPushButton::pressed, this, &MainWindow::onGreenMaximizeButtonPressed);
    connect(m_greenMaximizeButton, &QPushButton::clicked, this, &MainWindow::onGreenMaximizeButtonClicked);
    // red button
    connect(m_redCloseButton, &QPushButton::pressed, this, &MainWindow::onRedCloseButtonPressed);
    connect(m_redCloseButton, &QPushButton::clicked, this, &MainWindow::onRedCloseButtonClicked);
    // yellow button
    connect(m_yellowMinimizeButton, &QPushButton::pressed, this, &MainWindow::onYellowMinimizeButtonPressed);
    connect(m_yellowMinimizeButton, &QPushButton::clicked, this, &MainWindow::onYellowMinimizeButtonClicked);
    // new note button
    connect(m_newNoteButton, &QPushButton::pressed, this, &MainWindow::onNewNoteButtonPressed);
    connect(m_newNoteButton, &QPushButton::clicked, this, &MainWindow::onNewNoteButtonClicked);
    // delete note button
    connect(m_trashButton, &QPushButton::pressed, this, &MainWindow::onTrashButtonPressed);
    connect(m_trashButton, &QPushButton::clicked, this, &MainWindow::onTrashButtonClicked);
    connect(m_noteModel, &NoteModel::rowsRemoved, [this](){m_trashButton->setEnabled(true);});
    // 3 dots button
    connect(m_dotsButton, &QPushButton::pressed, this, &MainWindow::onDotsButtonPressed);
    connect(m_dotsButton, &QPushButton::clicked, this, &MainWindow::onDotsButtonClicked);
    // Style Editor Button
    connect(m_styleEditorButton, &QPushButton::pressed, this, &MainWindow::onStyleEditorButtonPressed);
    connect(m_styleEditorButton, &QPushButton::clicked, this, &MainWindow::onStyleEditorButtonClicked);
    // text edit text changed
    connect(m_textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextEditTextChanged);
    // textEdit scrollbar triggered
    connect(m_textEdit->verticalScrollBar(), &QAbstractSlider::actionTriggered, this, [=](){if(m_isFrameRightTopWidgetsVisible) setVisibilityOfFrameRightNonEditor(false);});
    // line edit text changed
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchEditTextChanged);
    // line edit enter key pressed
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &MainWindow::onSearchEditReturnPressed);
    // note pressed
    connect(m_noteView, &NoteView::pressed, this, &MainWindow::onNotePressed);
    // noteView viewport pressed
    connect(m_noteView, &NoteView::viewportPressed, this, [this](){
        if(m_isTemp && m_proxyModel->rowCount() > 1){
            QModelIndex indexInProxy = m_proxyModel->index(1, 0);
            selectNote(indexInProxy);
        }else if(m_isTemp && m_proxyModel->rowCount() == 1){
            QModelIndex indexInProxy = m_proxyModel->index(0, 0);
            m_editorDateLabel->clear();
            deleteNote(indexInProxy, false);
        }
    });
    // note model rows moved
    connect(m_noteModel, &NoteModel::rowsAboutToBeMoved, m_noteView, &NoteView::rowsAboutToBeMoved);
    connect(m_noteModel, &NoteModel::rowsMoved, m_noteView, &NoteView::rowsMoved);
    // auto save timer
    connect(m_autoSaveTimer, &QTimer::timeout, [this](){
        m_autoSaveTimer->stop();
        saveNoteToDB(m_currentSelectedNoteProxy);
    });
    // clear button
    connect(m_clearButton, &QToolButton::clicked, this, &MainWindow::onClearButtonClicked);
    // Restore Notes Action
    connect(m_restoreAction, &QAction::triggered, this, [this](){
        setMainWindowVisibility(isHidden()
                                || windowState() == Qt::WindowMinimized
                                || (qApp->applicationState() == Qt::ApplicationInactive));
    });
    // Quit Action
    connect(m_quitAction, &QAction::triggered, this, &MainWindow::QuitApplication);
    // Application state changed
    connect(qApp, &QApplication::applicationStateChanged, this,[this](){
        m_noteView->update(m_noteView->currentIndex());
    });

    // MainWindow <-> DBManager
    connect(this, &MainWindow::requestNotesList,
            m_dbManager,&DBManager::onNotesListRequested, Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestCreateUpdateNote,
            m_dbManager, &DBManager::onCreateUpdateRequested, Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestDeleteNote,
            m_dbManager, &DBManager::onDeleteNoteRequested);
    connect(this, &MainWindow::requestRestoreNotes,
            m_dbManager, &DBManager::onRestoreNotesRequested, Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestImportNotes,
            m_dbManager, &DBManager::onImportNotesRequested, Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestExportNotes,
            m_dbManager, &DBManager::onExportNotesRequested, Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestMigrateNotes,
            m_dbManager, &DBManager::onMigrateNotesRequested, Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestMigrateTrash,
            m_dbManager, &DBManager::onMigrateTrashRequested, Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestForceLastRowIndexValue,
            m_dbManager, &DBManager::onForceLastRowIndexValueRequested, Qt::BlockingQueuedConnection);

    connect(m_dbManager, &DBManager::notesReceived, this, &MainWindow::loadNotes);

#ifdef __APPLE__
    // Replace setUseNativeWindowFrame with just the part that handles pushing things up
    connect(this, &MainWindow::toggleFullScreen, this, [=](bool isFullScreen){adjustUpperWidgets(isFullScreen);});
#endif
}

/*!
 * \brief MainWindow::autoCheckForUpdates
 * Checks for updates, if an update is found, then the updater dialog will show
 * up, otherwise, no notification shall be showed
 */
void MainWindow::autoCheckForUpdates()
{
    m_updater.installEventFilter(this);
    m_updater.setShowWindowDisable(m_dontShowUpdateWindow);
    m_updater.checkForUpdates(false);
}

void MainWindow::setSearchEditStyleSheet(bool isFocused = false)
{
    QColor textColor = m_currentTheme == Theme::Dark ? m_currentEditorTextColor : QColor(26, 26, 26);

    m_searchEdit->setStyleSheet(QStringLiteral("QLineEdit{ "
                                             "  padding-left: 21px;"
                                             "  padding-right: 19px;"
                                             "  border: %2;"
                                             "  border-radius: 3px;"
                                             "  background: %1;"
                                             "  selection-background-color: rgb(61, 155, 218);"
                                             "  color: %3;"
                                             "} "
                                             "QLineEdit:focus { "
                                             "  border: 2px solid rgb(61, 155, 218);"
                                             "}"
                                             "QToolButton { "
                                             "  border: none; "
                                             "  padding: 0px;"
                                             "}").arg(m_currentThemeBackgroundColor.name(),
                                                      isFocused ? "2px solid rgb(61, 155, 218)" : "1px solid rgb(205, 205, 205)",                                                      textColor.name()));
}

/*!
 * \brief MainWindow::setupSearchEdit
 * Set the lineedit to start a bit to the right and end a bit to the left (pedding)
 */
void MainWindow::setupSearchEdit()
{
//    QLineEdit* searchEdit = m_searchEdit;

    m_searchEdit->setAttribute(Qt::WA_MacShowFocusRect, 0);

    setSearchEditStyleSheet(false);

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
    QSize searchSize(24, 25);
    searchButton->setIconSize(searchSize);
    searchButton->setCursor(Qt::ArrowCursor);

    // layout
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::RightToLeft, m_searchEdit);
    layout->setContentsMargins(0,0,3,0);
    layout->addWidget(m_clearButton);
    layout->addStretch();
    layout->addWidget(searchButton);
    m_searchEdit->setLayout(layout);

    m_searchEdit->installEventFilter(this);
}

/*!
 * \brief MainWindow::setCurrentFontBasedOnTypeface
 * Set the current font based on a given typeface
 */
void MainWindow::setCurrentFontBasedOnTypeface(FontTypeface selectedFontTypeFace)
{
    m_currentFontTypeface = selectedFontTypeFace;
    switch(selectedFontTypeFace) {
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

        if(m_textEdit->width() < m_smallEditorWidth) {
            m_currentFontPointSize = m_editorMediumFontSize - 2;
        } else if(m_textEdit->width() > m_smallEditorWidth && m_textEdit->width() < m_largeEditorWidth) {
            m_currentFontPointSize = m_editorMediumFontSize;
        } else if(m_textEdit->width() > m_largeEditorWidth) {
            m_currentFontPointSize = m_editorMediumFontSize + increaseSize;
        }

    m_currentSelectedFont = QFont(m_currentFontFamily, m_currentFontPointSize, QFont::Normal);
    m_textEdit->setFont(m_currentSelectedFont);

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

    m_styleEditorWindow.changeSelectedFont(FontTypeface::Mono, m_listOfMonoFonts.at(m_chosenMonoFontIndex));
    m_styleEditorWindow.changeSelectedFont(FontTypeface::Serif, m_listOfSerifFonts.at(m_chosenSerifFontIndex));
    m_styleEditorWindow.changeSelectedFont(FontTypeface::SansSerif, m_listOfSansSerifFonts.at(m_chosenSansSerifFontIndex));

    setCurrentFontBasedOnTypeface(m_currentFontTypeface);
    setTheme(m_currentTheme);
    m_styleEditorWindow.restoreSelectedOptions(false, m_currentFontTypeface, m_currentTheme);
}

void MainWindow::setupTextEditStyleSheet(int paddingLeft, int paddingRight)
{
    m_textEdit->setDocumentPadding(paddingLeft, 0, paddingRight, 2);

#if defined(Q_OS_LINUX)
    QString ss = QString("QTextEdit {background-color: %1;} "
                         "QTextEdit{selection-background-color: rgb(63, 99, 139);}"
                         "QScrollBar::handle:vertical:hover { background: rgb(170, 170, 171); } "
                         "QScrollBar::handle:vertical:pressed { background: rgb(149, 149, 149); } "
                         "QScrollBar::handle:vertical { border-radius: 4px; background: rgb(188, 188, 188); min-height: 20px; }  "
                         "QScrollBar::vertical {border-radius: 4px; width: 8px; color: rgba(255, 255, 255,0);} "
                         "QScrollBar {margin: 0; background: transparent;} "
                         "QScrollBar:hover { background-color: rgb(217, 217, 217);}"
                         "QScrollBar::add-line:vertical { width:0px; height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  "
                         "QScrollBar::sub-line:vertical { width:0px; height: 0px; subcontrol-position: top; subcontrol-origin: margin; }"
                         ).arg(m_currentEditorBackgroundColor.name());
#else
    QString ss = QString("QTextEdit {background-color: %1;} "
                         "QTextEdit{selection-background-color: rgb(63, 99, 139);}"
                         ).arg(m_currentEditorBackgroundColor.name());
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
    if(m_textEdit->lineWrapMode() == QTextEdit::WidgetWidth){
        setupTextEditStyleSheet(m_currentMinimumEditorPadding, m_currentMinimumEditorPadding);
        return;
    }

    QFontMetricsF fm(m_currentSelectedFont);
    QString limitingStringSample = QString("The quick brown fox jumps over the lazy dog the quick brown fox jumps over the lazy dog the quick brown fox jumps over the lazy dog");
    limitingStringSample.truncate(m_textEdit->lineWrapColumnOrWidth());
    qreal textSamplePixelsWidth = fm.width(limitingStringSample);
    m_currentAdaptableEditorPadding = (m_textEdit->width() - textSamplePixelsWidth) / 2 - 10;


    if(m_textEdit->width() - m_currentMinimumEditorPadding*2 > textSamplePixelsWidth &&
            m_currentAdaptableEditorPadding > 0 &&
            m_currentAdaptableEditorPadding > m_currentMinimumEditorPadding) {
        setupTextEditStyleSheet(m_currentAdaptableEditorPadding, m_currentAdaptableEditorPadding);
    } else {
        setupTextEditStyleSheet(m_currentMinimumEditorPadding, m_currentMinimumEditorPadding);
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
    m_textEdit->setTextColor(QColor(26, 26, 26));
    m_textEdit->setWordWrapMode(QTextOption::WordWrap);

#ifdef __APPLE__
    if(QFont("Avenir Next").exactMatch()) {
        m_listOfSansSerifFonts.push_front("Avenir Next");
    } else if(QFont("Avenir").exactMatch()) {
        m_listOfSansSerifFonts.push_front("Avenir");
    }

    if(QFont("SF Pro Text").exactMatch()) {
        m_listOfSansSerifFonts.push_front("SF Pro Text");
    } else if(QFont("Helvetica Neue").exactMatch()) {
        m_listOfSansSerifFonts.push_front("Helvetica Neue");
    } else if(QFont("Helvetica").exactMatch()) {
        m_listOfSansSerifFonts.push_front("Helvetica");
    }
#elif _WIN32
     if(QFont("Calibri").exactMatch())
         m_listOfSansSerifFonts.push_front("Calibri");

     if(QFont("Arial").exactMatch())
         m_listOfSansSerifFonts.push_front("Arial");

     if(QFont("Segoe UI").exactMatch())
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
    if(m_settingsDatabase->value(QStringLiteral("version"), "NULL") == "NULL")
        m_settingsDatabase->setValue(QStringLiteral("version"), qApp->applicationVersion());

    if(m_settingsDatabase->value(QStringLiteral("dontShowUpdateWindow"), "NULL") == "NULL")
        m_settingsDatabase->setValue(QStringLiteral("dontShowUpdateWindow"), m_dontShowUpdateWindow);

    if(m_settingsDatabase->value(QStringLiteral("windowGeometry"), "NULL") == "NULL"){
        int initWidth = 870;
        int initHeight = 630;
        QPoint center = qApp->desktop()->geometry().center();
        QRect rect(center.x() - initWidth/2, center.y() - initHeight/2, initWidth, initHeight);
        setGeometry(rect);
        m_settingsDatabase->setValue(QStringLiteral("windowGeometry"), saveGeometry());
    }

    if(m_settingsDatabase->value(QStringLiteral("splitterSizes"), "NULL") == "NULL"){
        m_splitter->resize(width()-2*m_layoutMargin, height()-2*m_layoutMargin);
        QList<int> sizes = m_splitter->sizes();
        m_noteListWidth = ui->frameLeft->minimumWidth() != 0 ? ui->frameLeft->minimumWidth() : m_noteListWidth;
        sizes[0] = m_noteListWidth;
        sizes[1] = m_splitter->width() - m_noteListWidth;
        m_splitter->setSizes(sizes);
        m_settingsDatabase->setValue(QStringLiteral("splitterSizes"), m_splitter->saveState());
    }
}

/*!
 * \brief MainWindow::setupDatabases
 * Setting up the database:
 */
void MainWindow::setupDatabases()
{
    m_settingsDatabase = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                                       QStringLiteral("Awesomeness"), QStringLiteral("Settings"), this);
    m_settingsDatabase->setFallbacksEnabled(false);
    initializeSettingsDatabase();

    bool doCreate = false;
    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());
    bool folderCreated = dir.mkpath(QStringLiteral("."));
    if(!folderCreated)
        qFatal("ERROR: Can't create settings folder : %s", dir.absolutePath().toStdString().c_str());

    QString noteDBFilePath(dir.path() + QDir::separator() + QStringLiteral("notes.db"));

    if(!QFile::exists(noteDBFilePath)){
        QFile noteDBFile(noteDBFilePath);
        if(!noteDBFile.open(QIODevice::WriteOnly))
            qFatal("ERROR : Can't create database file");

        noteDBFile.close();
        doCreate = true;
    }

    m_dbManager = new DBManager;
    m_dbThread = new QThread;
    m_dbThread->setObjectName(QStringLiteral("dbThread"));
    m_dbManager->moveToThread(m_dbThread);
    connect(m_dbThread, &QThread::started, [=](){emit requestOpenDBManager(noteDBFilePath, doCreate);});
    connect(this, &MainWindow::requestOpenDBManager, m_dbManager, &DBManager::onOpenDBManagerRequested);
    connect(m_dbThread, &QThread::finished, m_dbManager, &QObject::deleteLater);
    m_dbThread->start();
}

/*!
 * \brief MainWindow::setupModelView
 */
void MainWindow::setupModelView()
{
    m_noteView = static_cast<NoteView*>(ui->listView);
    m_proxyModel->setSourceModel(m_noteModel);
    m_proxyModel->setFilterKeyColumn(0);
    m_proxyModel->setFilterRole(NoteModel::NoteContent);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_noteView->setItemDelegate(new NoteWidgetDelegate(m_noteView));
    m_noteView->setModel(m_proxyModel);
}

/*!
 * \brief MainWindow::restoreStates
 * Restore the latest sates (if there are any) of the window and the splitter from
 * the settings database
 */
void MainWindow::restoreStates()
{
    setUseNativeWindowFrame(m_settingsDatabase->value(QStringLiteral("useNativeWindowFrame"), false).toBool());

    if(m_settingsDatabase->value(QStringLiteral("windowGeometry"), "NULL") != "NULL")
        this->restoreGeometry(m_settingsDatabase->value(QStringLiteral("windowGeometry")).toByteArray());

    if(m_settingsDatabase->value(QStringLiteral("editorSettingsWindowGeometry"), "NULL") != "NULL")
        m_styleEditorWindow.restoreGeometry(m_settingsDatabase->value(QStringLiteral("editorSettingsWindowGeometry")).toByteArray());

#if defined(Q_OS_LINUX)
    /// Set margin to zero if the window is maximized
//    if (isMaximized()) {
//        setMargins(QMargins(0,0,0,0));
//    }
#endif

    if(m_settingsDatabase->value(QStringLiteral("dontShowUpdateWindow"), "NULL") != "NULL")
        m_dontShowUpdateWindow = m_settingsDatabase->value(QStringLiteral("dontShowUpdateWindow")).toBool();

    m_splitter->setCollapsible(0, true);
    m_splitter->resize(width() - m_layoutMargin, height() - m_layoutMargin);
    if(m_settingsDatabase->value(QStringLiteral("splitterSizes"), "NULL") != "NULL")
        m_splitter->restoreState(m_settingsDatabase->value(QStringLiteral("splitterSizes")).toByteArray());
    m_noteListWidth = m_splitter->sizes().at(0);
    m_splitter->setCollapsible(0, false);

    QString selectedFontTypefaceFromDatabase = m_settingsDatabase->value(QStringLiteral("selectedFontTypeface"), "NULL").toString();
    if(selectedFontTypefaceFromDatabase != "NULL") {
        if(selectedFontTypefaceFromDatabase == "Mono") {
            m_currentFontTypeface = FontTypeface::Mono;
        } else if(selectedFontTypefaceFromDatabase == "Serif") {
            m_currentFontTypeface = FontTypeface::Serif;
        } else if(selectedFontTypefaceFromDatabase == "SansSerif") {
            m_currentFontTypeface = FontTypeface::SansSerif;
        }
    }

    if(m_settingsDatabase->value(QStringLiteral("editorMediumFontSize"), "NULL") != "NULL") {
        m_editorMediumFontSize = m_settingsDatabase->value(QStringLiteral("editorMediumFontSize")).toInt();
    } else {
#ifdef __APPLE__
        m_editorMediumFontSize = 17;
#else
        m_editorMediumFontSize = 13;
#endif
    }
    m_currentFontPointSize = m_editorMediumFontSize;

    bool isTextFullWidth = false;
    if(m_settingsDatabase->value(QStringLiteral("isTextFullWidth"), "NULL") != "NULL") {
        isTextFullWidth = m_settingsDatabase->value(QStringLiteral("isTextFullWidth")).toBool();
        if(isTextFullWidth) {
            m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
        } else {
            m_textEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
        }
    } else {
        m_textEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
    }

    if(m_settingsDatabase->value(QStringLiteral("charsLimitPerFontMono"), "NULL") != "NULL")
        m_currentCharsLimitPerFont.mono = m_settingsDatabase->value(QStringLiteral("charsLimitPerFontMono")).toInt();
    if(m_settingsDatabase->value(QStringLiteral("charsLimitPerFontSerif"), "NULL") != "NULL")
        m_currentCharsLimitPerFont.serif = m_settingsDatabase->value(QStringLiteral("charsLimitPerFontSerif")).toInt();
    if(m_settingsDatabase->value(QStringLiteral("charsLimitPerFontSansSerif"), "NULL") != "NULL")
        m_currentCharsLimitPerFont.sansSerif = m_settingsDatabase->value(QStringLiteral("charsLimitPerFontSansSerif")).toInt();

    if(m_settingsDatabase->value(QStringLiteral("chosenMonoFont"), "NULL") != "NULL") {
        QString fontName = m_settingsDatabase->value(QStringLiteral("chosenMonoFont")).toString();
        int fontIndex = m_listOfMonoFonts.indexOf(fontName);
        if(fontIndex != -1) {
            m_chosenMonoFontIndex = fontIndex;
        }
    }
    if(m_settingsDatabase->value(QStringLiteral("chosenSerifFont"), "NULL") != "NULL") {
        QString fontName = m_settingsDatabase->value(QStringLiteral("chosenSerifFont")).toString();
        int fontIndex = m_listOfSerifFonts.indexOf(fontName);
        if(fontIndex != -1) {
            m_chosenSerifFontIndex = fontIndex;
        }
    }
    if(m_settingsDatabase->value(QStringLiteral("chosenSansSerifFont"), "NULL") != "NULL") {
        QString fontName = m_settingsDatabase->value(QStringLiteral("chosenSansSerifFont")).toString();
        int fontIndex = m_listOfSansSerifFonts.indexOf(fontName);
        if(fontIndex != -1) {
            m_chosenSansSerifFontIndex = fontIndex;
        }
    }

    if(m_settingsDatabase->value(QStringLiteral("theme"), "NULL") != "NULL") {
        QString chosenTheme = m_settingsDatabase->value(QStringLiteral("theme")).toString();
        if (chosenTheme == "Light") {
            m_currentTheme = Theme::Light;
        } else if(chosenTheme == "Dark") {
            m_currentTheme = Theme::Dark;
        } else if(chosenTheme == "Sepia") {
            m_currentTheme = Theme::Sepia;
        }
    }

    m_styleEditorWindow.changeSelectedFont(FontTypeface::Mono, m_listOfMonoFonts.at(m_chosenMonoFontIndex));
    m_styleEditorWindow.changeSelectedFont(FontTypeface::Serif, m_listOfSerifFonts.at(m_chosenSerifFontIndex));
    m_styleEditorWindow.changeSelectedFont(FontTypeface::SansSerif, m_listOfSansSerifFonts.at(m_chosenSansSerifFontIndex));

    setCurrentFontBasedOnTypeface(m_currentFontTypeface);
    setTheme(m_currentTheme);
    m_styleEditorWindow.restoreSelectedOptions(isTextFullWidth, m_currentFontTypeface, m_currentTheme);
}

/*!
 * \brief MainWindow::getFirstLine
 * Get a string 'str' and return only the first line of it
 * If the string contain no text, return "New Note"
 * TODO: We might make it more efficient by not loading the entire string into the memory
 * \param str
 * \return
 */
QString MainWindow::getFirstLine(const QString& str)
{
    if(str.simplified().isEmpty())
        return "New Note";

    QString text = str.trimmed();
    QTextStream ts(&text);
    return ts.readLine(FIRST_LINE_MAX);
}

/*!
 * \brief MainWindow::getQDateTime
 * Get a date string of a note from database and put it's data into a QDateTime object
 * This function is not efficient
 * If QVariant would include toStdString() it might help, aka: notesDatabase->value().toStdString
 * \param date
 * \return
 */
QDateTime MainWindow::getQDateTime(QString date)
{
    QDateTime dateTime = QDateTime::fromString(date, Qt::ISODate);
    return dateTime;
}

/*!
 * \brief MainWindow::getNoteDateEditor
 * Get the full date of the selected note from the database and return
 * it as a string in form for editorDateLabel
 * \param dateEdited
 * \return
 */
QString MainWindow::getNoteDateEditor(QString dateEdited)
{
    QDateTime dateTimeEdited(getQDateTime(dateEdited));
    QLocale usLocale(QLocale(QStringLiteral("en_US")));

    return usLocale.toString(dateTimeEdited, QStringLiteral("MMMM d, yyyy, h:mm A"));
}

/*!
 * \brief MainWindow::generateNote
 * generate a new note
 * \param noteID
 * \return
 */
NoteData* MainWindow::generateNote(const int noteID)
{
    NoteData* newNote = new NoteData(this);
    newNote->setId(noteID);

    QDateTime noteDate = QDateTime::currentDateTime();
    newNote->setCreationDateTime(noteDate);
    newNote->setLastModificationDateTime(noteDate);
    newNote->setFullTitle(QStringLiteral("New Note"));

    return newNote;
}

/*!
 * \brief MainWindow::showNoteInEditor
 * show the specified note content text in the text editor
 * Set editorDateLabel text to the the selected note date
 * And restore the scrollBar position if it changed before.
 * \param noteIndex
 */
void MainWindow::showNoteInEditor(const QModelIndex &noteIndex)
{
    m_textEdit->blockSignals(true);

    /// fixing bug #202
    m_textEdit->setTextBackgroundColor(QColor(255,255,255, 0));


    QString content = noteIndex.data(NoteModel::NoteContent).toString();
    QDateTime dateTime = noteIndex.data(NoteModel::NoteLastModificationDateTime).toDateTime();
    int scrollbarPos = noteIndex.data(NoteModel::NoteScrollbarPos).toInt();

    // set text and date
    m_textEdit->setText(content);
    QString noteDate = dateTime.toString(Qt::ISODate);
    QString noteDateEditor = getNoteDateEditor(noteDate);
    m_editorDateLabel->setText(noteDateEditor);
    // set scrollbar position
    m_textEdit->verticalScrollBar()->setValue(scrollbarPos);
    m_textEdit->blockSignals(false);

    highlightSearch();
}

/*!
 * \brief MainWindow::createOrSelectFirstNote
 * Either create a new note if the user has no notes in the folder
 * or select the first note from the list
 */
void MainWindow::createOrSelectFirstNote() {
    createNewNoteIfEmpty();
    selectFirstNote();
}

/*!
 * \brief MainWindow::loadNotes
 * Load all the notes from database
 * add data to the models
 * sort them according to the date
 * update scrollbar stylesheet
 * \param noteList
 * \param noteCounter
 */
void MainWindow::loadNotes(QList<NoteData *> noteList, int noteCounter)
{
    if(!noteList.isEmpty()){
        m_noteModel->addListNote(noteList);
        m_noteModel->sort(0,Qt::AscendingOrder);
    }

    m_noteCounter = noteCounter;

    setTheme(m_currentTheme); // TODO: If we don't put this here, mainwindow will not update its background color, but this is not a proper place

    createOrSelectFirstNote();
}

/*!
 * \brief MainWindow::saveNoteToDB
 * save the current note to database
 * \param noteIndex
 */
void MainWindow::saveNoteToDB(const QModelIndex& noteIndex)
{
    if(noteIndex.isValid() && m_isContentModified){
        QModelIndex indexInSrc = m_proxyModel->mapToSource(noteIndex);
        NoteData* note = m_noteModel->getNote(indexInSrc);
        if(note != Q_NULLPTR)
            emit requestCreateUpdateNote(note);

        m_isContentModified = false;
    }
}

/*!
 * \brief MainWindow::removeNoteFromDB
 * \param noteIndex
 */
void MainWindow::removeNoteFromDB(const QModelIndex& noteIndex)
{
    if(noteIndex.isValid()){
        QModelIndex indexInSrc = m_proxyModel->mapToSource(noteIndex);
        NoteData* note = m_noteModel->getNote(indexInSrc);
        emit requestDeleteNote(note);
    }
}

/*!
 * \brief MainWindow::selectFirstNote
 * Select the first note in the notes list
 */
void MainWindow::selectFirstNote()
{
    if(m_proxyModel->rowCount() > 0){
        QModelIndex index = m_proxyModel->index(0,0);
        m_noteView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        m_noteView->setCurrentIndex(index);

        m_currentSelectedNoteProxy = index;
        showNoteInEditor(index);
    }
}

/*!
 * \brief MainWindow::createNewNoteIfEmpty
 * create a new note if there are no notes
 */
void MainWindow::createNewNoteIfEmpty()
{
    if(m_proxyModel->rowCount() == 0)
        createNewNote();
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
    m_newNoteButton->setIcon(QIcon(QStringLiteral(":/images/newNote_Regular.png")));

    if(!m_searchEdit->text().isEmpty()){
        clearSearch();
        m_selectedNoteBeforeSearchingInSource = QModelIndex();
    }

    // save the data of the previous selected
    if(m_currentSelectedNoteProxy.isValid()
            && m_isContentModified){

        saveNoteToDB(m_currentSelectedNoteProxy);
        m_isContentModified = false;
    }

    this->createNewNote();
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
    deleteSelectedNote();
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
    QMenu* viewMenu = mainMenu.addMenu(tr("View"));
    QMenu* importExportNotesMenu = mainMenu.addMenu(tr("Import/Export Notes"));
    importExportNotesMenu->setToolTipsVisible(true);
    viewMenu->setToolTipsVisible(true);
    mainMenu.setToolTipsVisible(true);

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
    bool isCollapsed = (m_splitter->sizes().at(0) == 0);
    QString actionLabel = isCollapsed? tr("Show notes list")
                                     : tr("Hide notes list");

    QAction* noteListVisbilityAction = viewMenu->addAction(actionLabel);
    noteListVisbilityAction->setShortcut(Qt::CTRL + Qt::Key_J);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    noteListVisbilityAction->setShortcutVisibleInContextMenu(true);
#endif
    if(isCollapsed){
        connect(noteListVisbilityAction, &QAction::triggered, this, &MainWindow::expandNoteList);
    }else{
        connect(noteListVisbilityAction, &QAction::triggered, this, &MainWindow::collapseNoteList);
    }

    // Enable or Disable markdown
    QString markDownLabel = is_markdown_enabled? tr("Disable Markdown")
                                               : tr("Enable Markdown");

    QAction* noteMarkdownVisibiltyAction = viewMenu->addAction(markDownLabel);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    noteMarkdownVisibiltyAction->setShortcutVisibleInContextMenu(true);
#endif

    if(is_markdown_enabled){
        connect(noteMarkdownVisibiltyAction, &QAction::triggered, this, &MainWindow::disableMarkdownHighlighter);
    }else{
        connect(noteMarkdownVisibiltyAction, &QAction::triggered, this, &MainWindow::enableMarkdownHighlighter);
    }

    // Check for update action
    QAction* checkForUpdatesAction = mainMenu.addAction(tr("Check For Updates"));
    connect (checkForUpdatesAction, &QAction::triggered, this, &MainWindow::checkForUpdates);

    // Autostart
    QAction* autostartAction = mainMenu.addAction(tr("Start automatically"));
    connect (autostartAction, &QAction::triggered, this, [&]() {
        m_autostart.setAutostart(autostartAction->isChecked());
    });
    autostartAction->setCheckable(true);
    autostartAction->setChecked(m_autostart.isAutostart());

    // About Notes
    QAction* aboutAction = mainMenu.addAction(tr("About Notes"));
    connect (aboutAction, &QAction::triggered, this, [&]() {

        m_aboutWindow.show();
    });

    mainMenu.addSeparator();

    // Close the app
    QAction* quitAppAction = mainMenu.addAction(tr("Quit"));
    connect (quitAppAction, &QAction::triggered, this, &MainWindow::QuitApplication);

    // Export notes action
    QAction* exportNotesFileAction = importExportNotesMenu->addAction (tr("Export"));
    exportNotesFileAction->setToolTip(tr("Save notes to a file"));
    connect (exportNotesFileAction, &QAction::triggered,
             this, &MainWindow::exportNotesFile);

    // Import notes action
    QAction* importNotesFileAction = importExportNotesMenu->addAction (tr("Import"));
    importNotesFileAction->setToolTip(tr("Add notes from a file"));
    connect (importNotesFileAction, &QAction::triggered,
             this, &MainWindow::importNotesFile);

    // Restore notes action
    QAction* restoreNotesFileAction = importExportNotesMenu->addAction (tr("Restore"));
    restoreNotesFileAction->setToolTip(tr("Replace all notes with notes from a file"));
    connect (restoreNotesFileAction, &QAction::triggered,
             this, &MainWindow::restoreNotesFile);

    // Export disabled if no notes exist
    if(m_noteModel->rowCount() < 1){
        exportNotesFileAction->setDisabled(true);
    }

#ifndef Q_OS_LINUX
    // Stay on top action
    QAction* stayOnTopAction = viewMenu->addAction(tr("Always stay on top"));
    stayOnTopAction->setToolTip(tr("Always keep the notes application on top of all windows"));
    stayOnTopAction->setCheckable(true);
    stayOnTopAction->setChecked(m_alwaysStayOnTop);
    connect(stayOnTopAction, &QAction::triggered, this, &MainWindow::toggleStayOnTop);
#endif

#ifndef __APPLE__
    // Use native frame action
    QAction* useNativeFrameAction = viewMenu->addAction(tr("Use native window frame"));
    useNativeFrameAction->setToolTip(tr("Use the window frame provided by the window manager"));
    useNativeFrameAction->setCheckable(true);
    useNativeFrameAction->setChecked(m_useNativeWindowFrame);
    connect(useNativeFrameAction, &QAction::triggered, this, &MainWindow::askBeforeSettingNativeWindowFrame);
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


    if(m_settingsDatabase->value(QStringLiteral("editorSettingsWindowGeometry"), "NULL") == "NULL")
        m_styleEditorWindow.move(m_newNoteButton->mapToGlobal(QPoint(-m_styleEditorWindow.width()-m_newNoteButton->width(),m_newNoteButton->height())));

    if(m_styleEditorWindow.isVisible()) {
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
    if(m_currentFontTypeface == fontTypeface) {
        switch(fontTypeface) {
        case FontTypeface::Mono:
            m_chosenMonoFontIndex = m_chosenMonoFontIndex < m_listOfMonoFonts.length()-1 ? m_chosenMonoFontIndex + 1 : 0;
            break;
        case FontTypeface::Serif:
            m_chosenSerifFontIndex = m_chosenSerifFontIndex < m_listOfSerifFonts.length()-1 ? m_chosenSerifFontIndex + 1 : 0;
            break;
        case FontTypeface::SansSerif:
            m_chosenSansSerifFontIndex = m_chosenSansSerifFontIndex < m_listOfSansSerifFonts.length()-1 ? m_chosenSansSerifFontIndex + 1 : 0;
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
    switch(fontSizeAction) {
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
    switch(editorTextWidth) {
    case EditorTextWidth::FullWidth:
        if(m_textEdit->lineWrapMode() != QTextEdit::WidgetWidth)
            m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
        else
            m_textEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
        break;
    case EditorTextWidth::Increase:
        m_textEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
        switch(m_currentFontTypeface){
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
        switch(m_currentFontTypeface){
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
    switch(theme) {
    case Theme::Light:
    {
        m_currentThemeBackgroundColor = QColor(247, 247, 247);
        m_currentEditorTextColor = QColor(26, 26, 26);
        m_currentEditorBackgroundColor = m_currentThemeBackgroundColor;
        m_currentRightFrameColor = m_currentThemeBackgroundColor;
        this->setStyleSheet(QStringLiteral("QMainWindow { background-color: rgb(247, 247, 247); }"));
        m_textEdit->setTextColor(m_currentEditorTextColor);
        m_noteView->setTheme(NoteView::Theme::Light);
        m_styleEditorWindow.setTheme(Theme::Light, m_currentThemeBackgroundColor, m_currentEditorTextColor);
        m_aboutWindow.setTheme(m_currentThemeBackgroundColor, m_currentEditorTextColor);
        break;
    }
    case Theme::Dark:
    {
        m_currentThemeBackgroundColor = QColor(26, 26, 26);
        m_currentEditorTextColor = QColor(204, 204, 204);
        m_currentEditorBackgroundColor = m_currentThemeBackgroundColor;
        m_currentRightFrameColor = m_currentThemeBackgroundColor;
        this->setStyleSheet(QStringLiteral("QMainWindow { background-color: rgb(26, 26, 26); }"));
        m_textEdit->setTextColor(m_currentEditorTextColor);
        m_noteView->setTheme(NoteView::Theme::Dark);
        m_styleEditorWindow.setTheme(Theme::Dark, m_currentThemeBackgroundColor, m_currentEditorTextColor);
        m_aboutWindow.setTheme(m_currentThemeBackgroundColor, m_currentEditorTextColor);
        break;
    }
    case Theme::Sepia:
    {
        m_currentThemeBackgroundColor = QColor(251, 240, 217);
        m_currentEditorTextColor = QColor(95, 74, 50);
        m_currentEditorBackgroundColor = m_currentThemeBackgroundColor;
        m_currentRightFrameColor = m_currentThemeBackgroundColor;
        this->setStyleSheet(QStringLiteral("QMainWindow { background-color: rgb(251, 240, 217); }"));
        m_textEdit->setTextColor(m_currentEditorTextColor);
        m_noteView->setTheme(NoteView::Theme::Sepia);
        m_styleEditorWindow.setTheme(Theme::Sepia, m_currentThemeBackgroundColor, QColor(26, 26, 26));
        m_aboutWindow.setTheme(m_currentThemeBackgroundColor, QColor(26, 26, 26));
        break;
    }
    }

    setSearchEditStyleSheet(false);

    int verticalScrollBarValueToRestore = m_textEdit->verticalScrollBar()->value();
    m_textEdit->setText(m_textEdit->toPlainText()); // TODO: Update the text color without setting the text
    m_textEdit->verticalScrollBar()->setValue(verticalScrollBarValueToRestore);
    alignTextEditText();
    setupRightFrame();
}

/*!
 * \brief MainWindow::onNotePressed
 * When clicking on a note in the scrollArea:
 * Unhighlight the previous selected note
 * If selecting a note when temporery note exist, delete the temp note
 * Highlight the selected note
 * Load the selected note content into textedit
 * \param index
 */
void MainWindow::onNotePressed(const QModelIndex& index)
{
    if(sender() != Q_NULLPTR){
        QModelIndex indexInProxy = m_proxyModel->index(index.row(), 0);
        selectNote(indexInProxy);
        m_noteView->setCurrentRowActive(false);
    }
}

/*!
 * \brief MainWindow::onTextEditTextChanged
 * When the text on textEdit change:
 * if the note edited is not on top of the list, we will make that happen
 * If the text changed is of a new (empty) note, reset temp values
 */
void MainWindow::onTextEditTextChanged()
{
    if(m_currentSelectedNoteProxy.isValid()){
        m_textEdit->blockSignals(true);
        QString content = m_currentSelectedNoteProxy.data(NoteModel::NoteContent).toString();
        if(m_textEdit->toPlainText() != content){

            // move note to the top of the list
            QModelIndex sourceIndex = m_proxyModel->mapToSource(m_currentSelectedNoteProxy);
            if(m_currentSelectedNoteProxy.row() != 0){
                moveNoteToTop();
            }else if(!m_searchEdit->text().isEmpty() && sourceIndex.row() != 0){
                m_noteView->setAnimationEnabled(false);
                moveNoteToTop();
                m_noteView->setAnimationEnabled(true);
            }

            // Get the new data
            QString firstline = getFirstLine(m_textEdit->toPlainText());
            QDateTime dateTime = QDateTime::currentDateTime();
            QString noteDate = dateTime.toString(Qt::ISODate);
            m_editorDateLabel->setText(getNoteDateEditor(noteDate));

            // update model
            QMap<int, QVariant> dataValue;
            dataValue[NoteModel::NoteContent] = QVariant::fromValue(m_textEdit->toPlainText());
            dataValue[NoteModel::NoteFullTitle] = QVariant::fromValue(firstline);
            dataValue[NoteModel::NoteLastModificationDateTime] = QVariant::fromValue(dateTime);

            QModelIndex index = m_proxyModel->mapToSource(m_currentSelectedNoteProxy);
            m_noteModel->setItemData(index, dataValue);

            m_isContentModified = true;

            m_autoSaveTimer->start(500);

            setVisibilityOfFrameRightNonEditor(false);
        }

        m_textEdit->blockSignals(false);

        m_isTemp = false;
    }else{
        qDebug() << "MainWindow::onTextEditTextChanged() : m_currentSelectedNoteProxy is not valid";
    }
}

/*!
 * \brief MainWindow::onSearchEditTextChanged
 * When text on searchEdit change:
 * If there is a temp note "New Note" while searching, we delete it
 * Saving the last selected note for recovery after searching
 * Clear all the notes from scrollArea and
 * If text is empty, reload all the notes from database
 * Else, load all the notes contain the string in searchEdit from database
 * \param keyword
 */
void MainWindow::onSearchEditTextChanged(const QString& keyword)
{
    m_textEdit->clearFocus();
    m_searchQueue.enqueue(keyword);

    if(!m_isOperationRunning){
        m_isOperationRunning = true;
        if(m_isTemp){
            m_isTemp = false;
            --m_noteCounter;
            // prevent the line edit from emitting signal
            // while animation for deleting the new note is running
            m_searchEdit->blockSignals(true);
            m_currentSelectedNoteProxy = QModelIndex();
            QModelIndex index = m_noteModel->index(0);
            m_noteModel->removeNote(index);
            m_searchEdit->blockSignals(false);

            if(m_noteModel->rowCount() > 0){
                m_selectedNoteBeforeSearchingInSource = m_noteModel->index(0);
            }else{
                m_selectedNoteBeforeSearchingInSource = QModelIndex();
            }

        }else if(!m_selectedNoteBeforeSearchingInSource.isValid()
                 && m_currentSelectedNoteProxy.isValid()){

            m_selectedNoteBeforeSearchingInSource = m_proxyModel->mapToSource(m_currentSelectedNoteProxy);
        }

        if(m_currentSelectedNoteProxy.isValid()
                && m_isContentModified){

            saveNoteToDB(m_currentSelectedNoteProxy);
        }

        // disable animation while searching
        m_noteView->setAnimationEnabled(false);

        while(!m_searchQueue.isEmpty()){
            qApp->processEvents();
            QString str = m_searchQueue.dequeue();
            if(str.isEmpty()){
                m_noteView->setFocusPolicy(Qt::StrongFocus);
                clearSearch();
                QModelIndex indexInProxy = m_proxyModel->mapFromSource(m_selectedNoteBeforeSearchingInSource);
                selectNote(indexInProxy);
                m_selectedNoteBeforeSearchingInSource = QModelIndex();
            }else{
                m_noteView->setFocusPolicy(Qt::NoFocus);
                findNotesContain(str);
            }
        }

        m_noteView->setAnimationEnabled(true);
        m_isOperationRunning = false;
    }

    highlightSearch();
}

/*!
 * \brief MainWindow::onClearButtonClicked
 * clears the search and
 * select the note that was selected before searching if it is still valid.
 */
void MainWindow::onClearButtonClicked()
{
    if(!m_isOperationRunning){

        clearSearch();

        if(m_noteModel->rowCount() > 0){
            QModelIndex indexInProxy = m_proxyModel->mapFromSource(m_selectedNoteBeforeSearchingInSource);
            int row = m_selectedNoteBeforeSearchingInSource.row();
            if(row == m_noteModel->rowCount())
                indexInProxy = m_proxyModel->index(m_proxyModel->rowCount()-1,0);

            selectNote(indexInProxy);
        }else{
            m_currentSelectedNoteProxy = QModelIndex();
        }

        m_selectedNoteBeforeSearchingInSource = QModelIndex();
    }
}

/*!
 * \brief MainWindow::createNewNote
 * create a new note
 * add it to the database
 * add it to the scrollArea
 */
void MainWindow::createNewNote()
{
    if(!m_isOperationRunning){
        m_isOperationRunning = true;

        m_noteView->scrollToTop();

        // clear the textEdit
        m_textEdit->blockSignals(true);
        m_textEdit->clear();
        m_textEdit->setFocus();
        m_textEdit->blockSignals(false);

        if(!m_isTemp){
            ++m_noteCounter;
            NoteData* tmpNote = generateNote(m_noteCounter);
            m_isTemp = true;

            // insert the new note to NoteModel
            QModelIndex indexSrc = m_noteModel->insertNote(tmpNote, 0);

            // update the editor header date label
            QString dateTimeFromDB = tmpNote->lastModificationdateTime().toString(Qt::ISODate);
            QString dateTimeForEditor = getNoteDateEditor(dateTimeFromDB);
            m_editorDateLabel->setText(dateTimeForEditor);

            // update the current selected index
            m_currentSelectedNoteProxy = m_proxyModel->mapFromSource(indexSrc);

        }else{
            int row = m_currentSelectedNoteProxy.row();
            m_noteView->animateAddedRow(QModelIndex(),row, row);
        }

        m_noteView->setCurrentIndex(m_currentSelectedNoteProxy);
        m_isOperationRunning = false;
    }
}

/*!
 * \brief MainWindow::deleteNote
 * deletes the specified note
 * \param noteIndex
 * note to delete
 * \param isFromUser
 * true if the user clicked on trash button
 */
void MainWindow::deleteNote(const QModelIndex &noteIndex, bool isFromUser)
{
    if(noteIndex.isValid()){
        // delete from model
        QModelIndex indexToBeRemoved = m_proxyModel->mapToSource(m_currentSelectedNoteProxy);
        NoteData* noteTobeRemoved = m_noteModel->removeNote(indexToBeRemoved);

        if(m_isTemp){
            m_isTemp = false;
            --m_noteCounter;
        }else{
            noteTobeRemoved->setDeletionDateTime(QDateTime::currentDateTime());
            emit requestDeleteNote(noteTobeRemoved);
        }

        if(isFromUser){
            // clear text edit and time date label
            m_editorDateLabel->clear();
            m_textEdit->blockSignals(true);
            m_textEdit->clear();
            m_textEdit->clearFocus();
            m_textEdit->blockSignals(false);

            if(m_noteModel->rowCount() > 0){
                QModelIndex index = m_noteView->currentIndex();
                m_currentSelectedNoteProxy = index;
            }else{
                m_currentSelectedNoteProxy = QModelIndex();
            }
        }
    }else{
        qDebug() << "MainWindow::deleteNote noteIndex is not valid";
    }

    m_noteView->setFocus();
}

/*!
 * \brief MainWindow::deleteSelectedNote
 * Delete the selected note
 */
void MainWindow::deleteSelectedNote()
{
    if(!m_isOperationRunning){
        m_isOperationRunning = true;
        if(m_currentSelectedNoteProxy.isValid()){

            // update the index of the selected note before searching
            if(!m_searchEdit->text().isEmpty()){
                QModelIndex currentIndexInSource = m_proxyModel->mapToSource(m_currentSelectedNoteProxy);
                int beforeSearchSelectedRow = m_selectedNoteBeforeSearchingInSource.row();
                if(currentIndexInSource.row() < beforeSearchSelectedRow){
                    m_selectedNoteBeforeSearchingInSource = m_noteModel->index(beforeSearchSelectedRow-1);
                }
            }

            deleteNote(m_currentSelectedNoteProxy, true);
            showNoteInEditor(m_currentSelectedNoteProxy);
        }
        m_isOperationRunning = false;
    }
}

/*!
 * \brief MainWindow::setFocusOnText
 * Set focus on textEdit
 */
void MainWindow::setFocusOnText()
{
    if(m_currentSelectedNoteProxy.isValid() && !m_textEdit->hasFocus()) {
        m_noteView->setCurrentRowActive(true);
        m_textEdit->setFocus();
    }
}

/*!
 * \brief MainWindow::setFocusOnCurrentNote
 * Set focus on current selected note
 */
void MainWindow::setFocusOnCurrentNote()
{
    if(m_currentSelectedNoteProxy.isValid()) {
        m_noteView->setCurrentRowActive(true);
        m_noteView->setFocus();
    }
}

/*!
 * \brief MainWindow::selectNoteUp
 * Select the note above the currentSelectedNote
 */
void MainWindow::selectNoteUp()
{
    if(m_currentSelectedNoteProxy.isValid()){
        int currentRow = m_noteView->currentIndex().row();
        QModelIndex aboveIndex = m_noteView->model()->index(currentRow - 1, 0);
        if(aboveIndex.isValid()){
            m_noteView->setCurrentIndex(aboveIndex);
            m_noteView->setCurrentRowActive(false);
            m_currentSelectedNoteProxy = aboveIndex;
            showNoteInEditor(m_currentSelectedNoteProxy);
        }
        if (!m_searchEdit->text().isEmpty()) {
            m_searchEdit->setFocus();
        } else {
            m_noteView->setFocus();
        }
    }
}

/*!
 * \brief MainWindow::selectNoteDown
 * Select the note below the currentSelectedNote
 */
void MainWindow::selectNoteDown()
{
    if(m_currentSelectedNoteProxy.isValid()){
        if(m_isTemp){
            deleteNote(m_currentSelectedNoteProxy, false);
            m_noteView->setCurrentIndex(m_currentSelectedNoteProxy);
            showNoteInEditor(m_currentSelectedNoteProxy);
        }else{
            int currentRow = m_noteView->currentIndex().row();
            QModelIndex belowIndex = m_noteView->model()->index(currentRow + 1, 0);
            if(belowIndex.isValid()){
                m_noteView->setCurrentIndex(belowIndex);
                m_noteView->setCurrentRowActive(false);
                m_currentSelectedNoteProxy = belowIndex;
                showNoteInEditor(m_currentSelectedNoteProxy);
            }
        }
        //if the searchEdit is not empty, set the focus to it
        if (!m_searchEdit->text().isEmpty()) {
            m_searchEdit->setFocus();
        } else {
            m_noteView->setFocus();
        }
    }
}

/*!
 * \brief MainWindow::fullscreenWindow
 * Switch to fullscreen mode
 */
void MainWindow::fullscreenWindow()
{
#if defined(Q_OS_LINUX)
    if(isFullScreen()){
        if(!isMaximized()) {
            m_noteListWidth = m_splitter->sizes().at(0) != 0 ? m_splitter->sizes().at(0) : m_noteListWidth;
//            QMargins margins(m_layoutMargin,m_layoutMargin,m_layoutMargin,m_layoutMargin);
//            setMargins(margins);
        }

        setWindowState(windowState() & ~Qt::WindowFullScreen);
    }else{
        setWindowState(windowState() | Qt::WindowFullScreen);
//        setMargins(QMargins(0,0,0,0));
    }

#elif _WIN32
    if(isFullScreen()){
        showNormal();
    }else{
        showFullScreen();
    }
#endif
}

/*!
 * \brief MainWindow::maximizeWindow
 * Maximize the window
 */
void MainWindow::maximizeWindow()
{
#if defined(Q_OS_LINUX)
    if(isMaximized()){
        if(!isFullScreen()){
            m_noteListWidth = m_splitter->sizes().at(0) != 0 ? m_splitter->sizes().at(0) : m_noteListWidth;
            QMargins margins(m_layoutMargin,m_layoutMargin,m_layoutMargin,m_layoutMargin);

            setMargins(margins);
            setWindowState(windowState() & ~Qt::WindowMaximized);
        }else{
            setWindowState(windowState() & ~Qt::WindowFullScreen);
        }

    }else{
        setWindowState(windowState() | Qt::WindowMaximized);
//        setMargins(QMargins(0,0,0,0));
    }
#elif _WIN32
    if(isMaximized()){
        setWindowState(windowState() & ~Qt::WindowMaximized);
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    }else if(isFullScreen()){
        setWindowState((windowState() | Qt::WindowMaximized) & ~Qt::WindowFullScreen);
        setGeometry(qApp->primaryScreen()->availableGeometry());
    }else{
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
#if defined(Q_OS_LINUX)
//    QMargins margins(m_layoutMargin,m_layoutMargin,m_layoutMargin,m_layoutMargin);
//    setMargins(margins);
#endif

    // BUG : QTBUG-57902 minimize doesn't store the window state before minimizing
    showMinimized();
}

/*!
 * \brief MainWindow::QuitApplication
 * Exit the application
 * If a new note created but wasn't edited, delete it from the database
 */
void MainWindow::QuitApplication()
{
    MainWindow::close();
}

/*!
 * \brief MainWindow::checkForUpdates
 * Called when the "Check for Updates" menu item is clicked, this function
 * instructs the updater window to check if there are any updates available
 * \param clicked
 */
void MainWindow::checkForUpdates(const bool clicked)
{
    Q_UNUSED (clicked)
    m_updater.checkForUpdates(true);
}

/*!
 * \brief MainWindow::importNotesFile
 * Called when the "Import Notes" menu button is clicked. this function will
 * prompt the user to select a file, attempt to load the file, and update the DB
 * if valid.
 * The user is presented with a dialog box if the upload/import fails for any reason.
 * \param clicked
 */
void MainWindow::importNotesFile(const bool clicked)
{
    Q_UNUSED (clicked)
    executeImport(false);
}

/*!
 * \brief MainWindow::restoreNotesFile
 * Called when the "Restore Notes" menu button is clicked. this function will
 * prompt the user to select a file, attempt to load the file, and update the DB
 * if valid.
 * The user is presented with a dialog box if the upload/import/restore fails for any reason.
 * \param clicked
 */
void MainWindow::restoreNotesFile(const bool clicked)
{
    Q_UNUSED (clicked)

    if (m_noteModel->rowCount() > 0) {
        QMessageBox msgBox;
        msgBox.setText(tr("Warning: All current notes will be lost. Make sure to create a backup copy before proceeding."));
        msgBox.setInformativeText(tr("Would you like to continue?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if  (msgBox.exec() != QMessageBox::Yes) {
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
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Notes Backup File"), "",
                                                    tr("Notes Backup File (*.nbk)"));

    if (fileName.isEmpty()) {
        return;
    } else {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            return;
        }
        QList<NoteData*> noteList;
        QDataStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        in.setVersion(QDataStream::Qt_5_6);
#elif QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
        in.setVersion(QDataStream::Qt_5_4);
#elif QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
        in.setVersion(QDataStream::Qt_5_2);
#endif

        try {
            in >> noteList;
        } catch (...) {
            // Any exception deserializing will result in an empty note list and  the user will be notified
        }
        file.close();

        if (noteList.isEmpty()) {
            QMessageBox::information(this, tr("Invalid file"), "Please select a valid notes export file");
            return;
        }

        QProgressDialog* pd = new QProgressDialog(replace ? "Restoring Notes..."
                                                          : "Importing Notes...", "", 0, 0, this);
        pd->deleteLater();
        pd->setCancelButton(Q_NULLPTR);
        pd->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        pd->setMinimumDuration(0);
        pd->show();
        pd->setValue(1);

        setButtonsAndFieldsEnabled(false);

        if(replace)
            emit requestRestoreNotes(noteList);
        else
            emit requestImportNotes(noteList);

        setButtonsAndFieldsEnabled(true);

        m_noteModel->clearNotes();
        emit requestNotesList();
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
void MainWindow::exportNotesFile(const bool clicked)
{
    Q_UNUSED (clicked)
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Notes"), "notes.nbk",
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

/*!
 * \brief MainWindow::toggleNoteList
 */
void MainWindow::toggleNoteList()
{
    bool isCollapsed = (m_splitter->sizes().at(0) == 0);
    if(isCollapsed) {
        expandNoteList();
    } else {
        collapseNoteList();
    }
}

/*!
 * \brief MainWindow::collapseNoteList
 */
void MainWindow::collapseNoteList()
{
    m_splitter->setCollapsible(0, true);
    QList<int> sizes = m_splitter->sizes();
    m_noteListWidth = sizes.at(0);
    sizes[0] = 0;
    m_splitter->setSizes(sizes);
    m_splitter->setCollapsible(0, false);

    if(!m_isFrameRightTopWidgetsVisible) {
#ifdef __APPLE__
        this->setStandardWindowButtonsMacVisibility(false);
#else
        m_redCloseButton->setVisible(false);
        m_yellowMinimizeButton->setVisible(false);
        m_greenMaximizeButton->setVisible(false);
#endif
    }
}

/*!
 * \brief MainWindow::expandNoteList
 */
void MainWindow::expandNoteList()
{
    int minWidth = ui->frameLeft->minimumWidth();
    int leftWidth = m_noteListWidth < minWidth ? minWidth : m_noteListWidth;

    QList<int> sizes = m_splitter->sizes();
    sizes[0] = leftWidth;
    sizes[1] = m_splitter->width() - leftWidth;
    m_splitter->setSizes(sizes);

#ifdef __APPLE__
    this->setStandardWindowButtonsMacVisibility(true);
#else
    m_redCloseButton->setVisible(true);
    m_yellowMinimizeButton->setVisible(true);
    m_greenMaximizeButton->setVisible(true);
#endif
}

/*!
 * \brief MainWindow::enableMarkdownHighlighter
 */
void MainWindow::enableMarkdownHighlighter()
{
    m_highlighter = new MarkdownHighlighter(m_textEdit->document());
    is_markdown_enabled = true;
}

/*!
 * \brief MainWindow::disableMarkdownHighlighter
 */
void MainWindow::disableMarkdownHighlighter()
{
    delete m_highlighter;
    is_markdown_enabled = false;
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
    if(this->windowState() == Qt::WindowFullScreen){
        m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/greenInPressed.png")));
    }else{
        m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/greenPressed.png")));
    }
#endif
}

/*!
 * \brief MainWindow::onYellowMinimizeButtonPressed
 * When the yellow button is pressed set it's icon accordingly
 */
void MainWindow::onYellowMinimizeButtonPressed()
{
#ifdef _WIN32
    if(this->windowState() == Qt::WindowFullScreen){
        m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/windows_de-maximize_pressed.png")));
    }else{
        m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/windows_maximize_pressed.png")));
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
    m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/windows_de-maximize_regular.png")));

    fullscreenWindow();
#else
    m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/yellow.png")));

    minimizeWindow();
    m_restoreAction->setText(tr("&Show Notes"));
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

    setMainWindowVisibility(false);
}

/*!
 * \brief MainWindow::closeEvent
 * Called when the app is about the close
 * save the geometry of the app to the settings
 * save the current note if it's note temporary one otherwise remove it from DB
 * \param event
 */
void MainWindow::closeEvent(QCloseEvent* event)
{
    if(windowState() != Qt::WindowFullScreen) {
        m_settingsDatabase->setValue(QStringLiteral("windowGeometry"), saveGeometry());
        if(m_styleEditorWindow.windowState() != Qt::WindowFullScreen)
            m_settingsDatabase->setValue(QStringLiteral("editorSettingsWindowGeometry"), m_styleEditorWindow.saveGeometry());
    }

    if(m_currentSelectedNoteProxy.isValid()
            &&  m_isContentModified
            && !m_isTemp){

        saveNoteToDB(m_currentSelectedNoteProxy);
    }

    m_settingsDatabase->setValue(QStringLiteral("dontShowUpdateWindow"), m_dontShowUpdateWindow);
    m_settingsDatabase->setValue(QStringLiteral("splitterSizes"), m_splitter->saveState());

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
    m_settingsDatabase->setValue(QStringLiteral("isTextFullWidth"), m_textEdit->lineWrapMode() == QTextEdit::WidgetWidth ? true : false);
    m_settingsDatabase->setValue(QStringLiteral("charsLimitPerFontMono"), m_currentCharsLimitPerFont.mono);
    m_settingsDatabase->setValue(QStringLiteral("charsLimitPerFontSerif"), m_currentCharsLimitPerFont.serif);
    m_settingsDatabase->setValue(QStringLiteral("charsLimitPerFontSansSerif"), m_currentCharsLimitPerFont.sansSerif);
    m_settingsDatabase->setValue(QStringLiteral("theme"), currentThemeString);
    m_settingsDatabase->setValue(QStringLiteral("chosenMonoFont"), m_listOfMonoFonts.at(m_chosenMonoFontIndex));
    m_settingsDatabase->setValue(QStringLiteral("chosenSerifFont"), m_listOfSerifFonts.at(m_chosenSerifFontIndex));
    m_settingsDatabase->setValue(QStringLiteral("chosenSansSerifFont"), m_listOfSansSerifFonts.at(m_chosenSansSerifFontIndex));

    m_settingsDatabase->sync();

    QWidget::closeEvent(event);
}

#ifndef _WIN32
/*!
 * \brief MainWindow::mousePressEvent
 * Set variables to the position of the window when the mouse is pressed
 * \param event
 */
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    m_mousePressX = event->x();
    m_mousePressY = event->y();

    if(event->buttons() == Qt::LeftButton){
        if(m_mousePressX < this->width() - m_layoutMargin
                && m_mousePressX >m_layoutMargin
                && m_mousePressY < this->height() - m_layoutMargin
                && m_mousePressY > m_layoutMargin){

            m_canMoveWindow = true;
//            QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));

#ifndef __APPLE__
        }else{
            m_canStretchWindow = true;

            int currentWidth = m_splitter->sizes().at(0);
            if(currentWidth != 0)
                m_noteListWidth = currentWidth;

            if((m_mousePressX < this->width() && m_mousePressX > this->width() - m_layoutMargin)
                    && (m_mousePressY < m_layoutMargin && m_mousePressY > 0)){
                m_stretchSide = StretchSide::TopRight;
            }else if((m_mousePressX < this->width() && m_mousePressX > this->width() - m_layoutMargin)
                     && (m_mousePressY < this->height() && m_mousePressY > this->height() - m_layoutMargin)){
                m_stretchSide = StretchSide::BottomRight;
            }else if((m_mousePressX < m_layoutMargin && m_mousePressX > 0)
                     && (m_mousePressY < m_layoutMargin && m_mousePressY > 0)){
                m_stretchSide = StretchSide::TopLeft;
            }else if((m_mousePressX < m_layoutMargin && m_mousePressX > 0)
                     && (m_mousePressY < this->height() && m_mousePressY > this->height() - m_layoutMargin)){
                m_stretchSide = StretchSide::BottomLeft;
            }else if(m_mousePressX < this->width() && m_mousePressX > this->width() - m_layoutMargin){
                m_stretchSide = StretchSide::Right;
            }else if(m_mousePressX < m_layoutMargin && m_mousePressX > 0){
                m_stretchSide = StretchSide::Left;
            }else if(m_mousePressY < this->height() && m_mousePressY > this->height() - m_layoutMargin){
                m_stretchSide = StretchSide::Bottom;
            }else if(m_mousePressY < m_layoutMargin && m_mousePressY > 0){
                m_stretchSide = StretchSide::Top;
            }else{
                m_stretchSide = StretchSide::None;
            }
#endif
        }

        event->accept();

    }else{
        QMainWindow::mousePressEvent(event);
    }
}

/*!
 * \brief MainWindow::mouseMoveEvent
 * Move the window according to the mouse positions
 * \param event
 */
void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
#ifndef __APPLE__
    if(!m_canStretchWindow && !m_canMoveWindow){
        m_mousePressX = event->x();
        m_mousePressY = event->y();

        if((m_mousePressX < this->width() && m_mousePressX > this->width() - m_layoutMargin)
                && (m_mousePressY < m_layoutMargin && m_mousePressY > 0)){
            m_stretchSide = StretchSide::TopRight;
        }else if((m_mousePressX < this->width() && m_mousePressX > this->width() - m_layoutMargin)
                 && (m_mousePressY < this->height() && m_mousePressY > this->height() - m_layoutMargin)){
            m_stretchSide = StretchSide::BottomRight;
        }else if((m_mousePressX < m_layoutMargin && m_mousePressX > 0)
                 && (m_mousePressY < m_layoutMargin && m_mousePressY > 0)){
            m_stretchSide = StretchSide::TopLeft;
        }else if((m_mousePressX < m_layoutMargin && m_mousePressX > 0)
                 && (m_mousePressY < this->height() && m_mousePressY > this->height() - m_layoutMargin)){
            m_stretchSide = StretchSide::BottomLeft;
        }else if(m_mousePressX < this->width() && m_mousePressX > this->width() - m_layoutMargin){
            m_stretchSide = StretchSide::Right;
        }else if(m_mousePressX < m_layoutMargin && m_mousePressX > 0){
            m_stretchSide = StretchSide::Left;
        }else if(m_mousePressY < this->height() && m_mousePressY > this->height() - m_layoutMargin){
            m_stretchSide = StretchSide::Bottom;
        }else if(m_mousePressY < m_layoutMargin && m_mousePressY > 0){
            m_stretchSide = StretchSide::Top;
        }else{
            m_stretchSide = StretchSide::None;
        }
    }

    if(!m_canMoveWindow){
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
            if(!m_canStretchWindow)
                ui->centralWidget->setCursor(Qt::ArrowCursor);
            break;
        }
    }
#endif

    if(m_canMoveWindow){
        int dx = event->globalX() - m_mousePressX;
        int dy = event->globalY() - m_mousePressY;
        move (dx, dy);

    }
#ifndef __APPLE__
    else if(m_canStretchWindow && !isMaximized() && !isFullScreen()){
        int newX = x();
        int newY = y();
        int newWidth = width();
        int newHeight = height();

        int minY =  QApplication::desktop()->availableGeometry().y();

        switch (m_stretchSide) {
        case StretchSide::Right:
            newWidth = abs(event->globalX()-this->x()+1);
            newWidth = newWidth < minimumWidth() ? minimumWidth() : newWidth;
            break;
        case StretchSide::Left:
            newX = event->globalX() - m_mousePressX;
            newX = newX > 0 ? newX : 0;
            newX = newX > geometry().bottomRight().x() - minimumWidth() ? geometry().bottomRight().x() - minimumWidth() : newX;
            newWidth = geometry().topRight().x() - newX + 1;
            newWidth = newWidth < minimumWidth() ? minimumWidth() : newWidth;
            break;
        case StretchSide::Top:
            newY = event->globalY() - m_mousePressY;
            newY = newY < minY ? minY : newY;
            newY = newY > geometry().bottomRight().y() - minimumHeight() ? geometry().bottomRight().y() - minimumHeight() : newY;
            newHeight = geometry().bottomLeft().y() - newY + 1;
            newHeight = newHeight < minimumHeight() ? minimumHeight() : newHeight ;

            break;
        case StretchSide::Bottom:
            newHeight = abs(event->globalY()-y()+1);
            newHeight = newHeight < minimumHeight() ? minimumHeight() : newHeight;

            break;
        case StretchSide::TopLeft:
            newX = event->globalX() - m_mousePressX;
            newX = newX < 0 ? 0: newX;
            newX = newX > geometry().bottomRight().x() - minimumWidth() ? geometry().bottomRight().x()-minimumWidth() : newX;

            newY = event->globalY() - m_mousePressY;
            newY = newY < minY ? minY : newY;
            newY = newY > geometry().bottomRight().y() - minimumHeight() ? geometry().bottomRight().y() - minimumHeight() : newY;

            newWidth = geometry().bottomRight().x() - newX + 1;
            newWidth = newWidth < minimumWidth() ? minimumWidth() : newWidth;

            newHeight = geometry().bottomRight().y() - newY + 1;
            newHeight = newHeight < minimumHeight() ? minimumHeight() : newHeight;

            break;
        case StretchSide::BottomLeft:
            newX = event->globalX() - m_mousePressX;
            newX = newX < 0 ? 0: newX;
            newX = newX > geometry().bottomRight().x() - minimumWidth() ? geometry().bottomRight().x()-minimumWidth() : newX;

            newWidth = geometry().bottomRight().x() - newX + 1;
            newWidth = newWidth < minimumWidth() ? minimumWidth() : newWidth;

            newHeight = event->globalY() - y() + 1;
            newHeight = newHeight < minimumHeight() ? minimumHeight() : newHeight;

            break;
        case StretchSide::TopRight:
            newY = event->globalY() - m_mousePressY;
            newY = newY > geometry().bottomRight().y() - minimumHeight() ? geometry().bottomRight().y() - minimumHeight() : newY;
            newY = newY < minY ? minY : newY;

            newWidth = event->globalX() - x() + 1;
            newWidth = newWidth < minimumWidth() ? minimumWidth() : newWidth;

            newHeight = geometry().bottomRight().y() - newY + 1;
            newHeight = newHeight < minimumHeight() ? minimumHeight() : newHeight;

            break;
        case StretchSide::BottomRight:
            newWidth = event->globalX() - x() + 1;
            newWidth = newWidth < minimumWidth() ? minimumWidth() : newWidth;

            newHeight = event->globalY() - y() + 1;
            newHeight = newHeight < minimumHeight() ? minimumHeight() : newHeight;

            break;
        default:
            break;
        }

        setGeometry(newX, newY, newWidth, newHeight);
        QList<int> sizes = m_splitter->sizes();
        if(sizes[0] != 0){
            sizes[0] = m_noteListWidth;
            sizes[1] = m_splitter->width() - m_noteListWidth;
            m_splitter->setSizes(sizes);
        }
    }
#endif
    event->accept();
}

/*!
 * \brief MainWindow::mouseReleaseEvent
 * Initialize flags
 * \param event
 */
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
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
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton){
        if(event->pos().x() < this->width() - 5
                && event->pos().x() >5
                && event->pos().y() < this->height()-5
                && event->pos().y() > 5){

            m_canMoveWindow = true;
            m_mousePressX = event->x();
            m_mousePressY = event->y();
        }
    }

    event->accept();
}

/*!
 * \brief MainWindow::mouseMoveEvent
 * Move the window according to the mouse positions
 * \param event
 */
void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if(m_canMoveWindow){
//        this->setCursor(Qt::ClosedHandCursor);
        int dx = event->globalX() - m_mousePressX;
        int dy = event->globalY() - m_mousePressY;
        move (dx, dy);

    }
}

/*!
 * \brief MainWindow::mouseReleaseEvent
 * Initialize flags
 * \param event
 */
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_canMoveWindow = false;
//    this->unsetCursor();
    event->accept();
}
#endif

/*!
 * \brief MainWindow::moveNoteToTop
 * moves the current note Widget to the top of the layout
 */
void MainWindow::moveNoteToTop()
{
    // check if the current note is note on the top of the list
    // if true move the note to the top
    if(m_currentSelectedNoteProxy.isValid()){

        m_noteView->scrollToTop();

        // move the current selected note to the top
        QModelIndex sourceIndex = m_proxyModel->mapToSource(m_currentSelectedNoteProxy);
        QModelIndex destinationIndex = m_noteModel->index(0);
        m_noteModel->moveRow(sourceIndex, sourceIndex.row(), destinationIndex, 0);

        // update the current item
        m_currentSelectedNoteProxy = m_proxyModel->mapFromSource(destinationIndex);
        m_noteView->setCurrentIndex(m_currentSelectedNoteProxy);
    }else{
        qDebug() << "MainWindow::moveNoteTop : m_currentSelectedNoteProxy not valid";
    }
}

/*!
 * \brief MainWindow::clearSearch
 */
void MainWindow::clearSearch()
{
    m_noteView->setFocusPolicy(Qt::StrongFocus);

    m_searchEdit->blockSignals(true);
    m_searchEdit->clear();
    m_searchEdit->blockSignals(false);

    m_textEdit->blockSignals(true);
    m_textEdit->clear();
    m_textEdit->clearFocus();
    m_editorDateLabel->clear();
    m_textEdit->blockSignals(false);

    m_proxyModel->setFilterFixedString(QString());

    m_clearButton->hide();
    m_searchEdit->setFocus();
}

/*!
 * \brief MainWindow::findNotesContain
 * \param keyword
 */
void MainWindow::findNotesContain(const QString& keyword)
{
    m_proxyModel->setFilterFixedString(keyword);
    m_clearButton->show();

    m_textEdit->blockSignals(true);
    m_textEdit->clear();
    m_editorDateLabel->clear();
    m_textEdit->blockSignals(false);

    if(m_proxyModel->rowCount() > 0){
        selectFirstNote();
    }else{
        m_currentSelectedNoteProxy = QModelIndex();
    }
}

/*!
 * \brief MainWindow::selectNote
 * \param noteIndex
 */
void MainWindow::selectNote(const QModelIndex &noteIndex)
{
    if(noteIndex.isValid()){
        // save the position of text edit scrollbar
        if(!m_isTemp && m_currentSelectedNoteProxy.isValid()){
            int pos = m_textEdit->verticalScrollBar()->value();
            QModelIndex indexSrc = m_proxyModel->mapToSource(m_currentSelectedNoteProxy);
            m_noteModel->setData(indexSrc, QVariant::fromValue(pos), NoteModel::NoteScrollbarPos);
        }

        // show the content of the pressed note in the text editor
        showNoteInEditor(noteIndex);

        if(m_isTemp && noteIndex.row() != 0){
            // delete the unmodified new note
            deleteNote(m_currentSelectedNoteProxy, false);
            m_currentSelectedNoteProxy = m_proxyModel->index(noteIndex.row()-1, 0);
        }else if(!m_isTemp
                 && m_currentSelectedNoteProxy.isValid()
                 && noteIndex != m_currentSelectedNoteProxy
                 && m_isContentModified){
            // save if the previous selected note was modified
            saveNoteToDB(m_currentSelectedNoteProxy);
            m_currentSelectedNoteProxy = noteIndex;
        }else{
            m_currentSelectedNoteProxy = noteIndex;
        }

        m_noteView->selectionModel()->select(m_currentSelectedNoteProxy, QItemSelectionModel::ClearAndSelect);
        m_noteView->setCurrentIndex(m_currentSelectedNoteProxy);
        m_noteView->scrollTo(m_currentSelectedNoteProxy);
    }else{
        qDebug() << "MainWindow::selectNote() : noteIndex is not valid";
    }
}

/*!
 * \brief MainWindow::checkMigration
 */
void MainWindow::checkMigration()
{
    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());

    QString oldNoteDBPath(dir.path() + QDir::separator() + "Notes.ini");
    if(QFile::exists(oldNoteDBPath))
        migrateNote(oldNoteDBPath);

    QString oldTrashDBPath(dir.path() + QDir::separator() + "Trash.ini");
    if(QFile::exists(oldTrashDBPath))
        migrateTrash(oldTrashDBPath);

    emit requestForceLastRowIndexValue(m_noteCounter);
}

/*!
 * \brief MainWindow::migrateNote
 * \param notePath
 */
void MainWindow::migrateNote(QString notePath)
{
    QSettings notesIni(notePath, QSettings::IniFormat);
    QStringList dbKeys = notesIni.allKeys();

    m_noteCounter = notesIni.value(QStringLiteral("notesCounter"), "0").toInt();
    QList<NoteData *> noteList;

    auto it = dbKeys.begin();
    for(; it < dbKeys.end()-1; it += 3){
        QString noteName = it->split(QStringLiteral("/"))[0];
        int id = noteName.split(QStringLiteral("_"))[1].toInt();

        // sync db index with biggest notes id
        m_noteCounter = m_noteCounter < id ? id : m_noteCounter;

        NoteData* newNote = new NoteData();
        newNote->setId(id);

        QString createdDateDB = notesIni.value(noteName + QStringLiteral("/dateCreated"), "Error").toString();
        newNote->setCreationDateTime(QDateTime::fromString(createdDateDB, Qt::ISODate));
        QString lastEditedDateDB = notesIni.value(noteName + QStringLiteral("/dateEdited"), "Error").toString();
        newNote->setLastModificationDateTime(QDateTime::fromString(lastEditedDateDB, Qt::ISODate));
        QString contentText = notesIni.value(noteName + QStringLiteral("/content"), "Error").toString();
        newNote->setContent(contentText);
        QString firstLine = getFirstLine(contentText);
        newNote->setFullTitle(firstLine);

        noteList.append(newNote);
    }

    if(!noteList.isEmpty())
        emit requestMigrateNotes(noteList);

    QFile oldNoteDBFile(notePath);
    oldNoteDBFile.rename(QFileInfo(notePath).dir().path() + QDir::separator() + QStringLiteral("oldNotes.ini"));
}

/*!
 * \brief MainWindow::migrateTrash
 * \param trashPath
 */
void MainWindow::migrateTrash(QString trashPath)
{
    QSettings trashIni(trashPath, QSettings::IniFormat);
    QStringList dbKeys = trashIni.allKeys();

    QList<NoteData *> noteList;

    auto it = dbKeys.begin();
    for(; it < dbKeys.end()-1; it += 3){
        QString noteName = it->split(QStringLiteral("/"))[0];
        int id = noteName.split(QStringLiteral("_"))[1].toInt();

        // sync db index with biggest notes id
        m_noteCounter = m_noteCounter < id ? id : m_noteCounter;

        NoteData* newNote = new NoteData();
        newNote->setId(id);

        QString createdDateDB = trashIni.value(noteName + QStringLiteral("/dateCreated"), "Error").toString();
        newNote->setCreationDateTime(QDateTime::fromString(createdDateDB, Qt::ISODate));
        QString lastEditedDateDB = trashIni.value(noteName + QStringLiteral("/dateEdited"), "Error").toString();
        newNote->setLastModificationDateTime(QDateTime::fromString(lastEditedDateDB, Qt::ISODate));
        QString contentText = trashIni.value(noteName + QStringLiteral("/content"), "Error").toString();
        newNote->setContent(contentText);
        QString firstLine = getFirstLine(contentText);
        newNote->setFullTitle(firstLine);

        noteList.append(newNote);
    }

    if(!noteList.isEmpty())
        emit requestMigrateTrash(noteList);

    QFile oldTrashDBFile(trashPath);
    oldTrashDBFile.rename(QFileInfo(trashPath).dir().path() + QDir::separator() + QStringLiteral("oldTrash.ini"));
}

/*!
 * \brief MainWindow::dropShadow
 * \param painter
 * \param type
 * \param side
 */
void MainWindow::dropShadow(QPainter& painter, ShadowType type, MainWindow::ShadowSide side)
{
    int resizedShadowWidth = m_shadowWidth > m_layoutMargin ? m_layoutMargin : m_shadowWidth;

    QRect mainRect   = rect();

    QRect innerRect(m_layoutMargin,
                    m_layoutMargin,
                    mainRect.width() - 2 * resizedShadowWidth + 1,
                    mainRect.height() - 2 * resizedShadowWidth + 1);
    QRect outerRect(innerRect.x() - resizedShadowWidth,
                    innerRect.y() - resizedShadowWidth,
                    innerRect.width() + 2* resizedShadowWidth,
                    innerRect.height() + 2* resizedShadowWidth);

    QPoint center;
    QPoint topLeft;
    QPoint bottomRight;
    QPoint shadowStart;
    QPoint shadowStop;
    QRadialGradient radialGradient;
    QLinearGradient linearGradient;

    switch (side) {
    case ShadowSide::Left :
        topLeft     = QPoint(outerRect.left(), innerRect.top() + 1);
        bottomRight = QPoint(innerRect.left(), innerRect.bottom() - 1);
        shadowStart = QPoint(innerRect.left(), innerRect.top() + 1);
        shadowStop  = QPoint(outerRect.left(), innerRect.top() + 1);
        break;
    case ShadowSide::Top :
        topLeft     = QPoint(innerRect.left() + 1, outerRect.top());
        bottomRight = QPoint(innerRect.right() - 1, innerRect.top());
        shadowStart = QPoint(innerRect.left() + 1, innerRect.top());
        shadowStop  = QPoint(innerRect.left() + 1, outerRect.top());
        break;
    case ShadowSide::Right :
        topLeft     = QPoint(innerRect.right(), innerRect.top() + 1);
        bottomRight = QPoint(outerRect.right(), innerRect.bottom() - 1);
        shadowStart = QPoint(innerRect.right(), innerRect.top() + 1);
        shadowStop  = QPoint(outerRect.right(), innerRect.top() + 1);
        break;
    case ShadowSide::Bottom :
        topLeft     = QPoint(innerRect.left() + 1, innerRect.bottom());
        bottomRight = QPoint(innerRect.right() - 1, outerRect.bottom());
        shadowStart = QPoint(innerRect.left() + 1, innerRect.bottom());
        shadowStop  = QPoint(innerRect.left() + 1, outerRect.bottom());
        break;
    case ShadowSide::TopLeft:
        topLeft     = outerRect.topLeft();
        bottomRight = innerRect.topLeft();
        center      = innerRect.topLeft();
        break;
    case ShadowSide::TopRight:
        topLeft     = QPoint(innerRect.right(), outerRect.top());
        bottomRight = QPoint(outerRect.right(), innerRect.top());
        center      = innerRect.topRight();
        break;
    case ShadowSide::BottomRight:
        topLeft     = innerRect.bottomRight();
        bottomRight = outerRect.bottomRight();
        center      = innerRect.bottomRight();
        break;
    case ShadowSide::BottomLeft:
        topLeft     = QPoint(outerRect.left(), innerRect.bottom());
        bottomRight = QPoint(innerRect.left(), outerRect.bottom());
        center      = innerRect.bottomLeft();
        break;
    }


    QRect zone(topLeft, bottomRight);
    radialGradient = QRadialGradient(center, resizedShadowWidth, center);

    linearGradient.setStart(shadowStart);
    linearGradient.setFinalStop(shadowStop);

    switch (type) {
    case ShadowType::Radial :
        fillRectWithGradient(painter, zone, radialGradient);
        break;
    case ShadowType::Linear :
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
void MainWindow::fillRectWithGradient(QPainter& painter, const QRect& rect, QGradient& gradient)
{
    double variance = 0.2;
    double xMax = 1.10;
    double q = 70/gaussianDist(0, 0, sqrt(variance));
    double nPt = 100.0;

    for(int i=0; i<=nPt; i++){
        double v = gaussianDist(i*xMax/nPt, 0, sqrt(variance));

        QColor c(168, 168, 168, int(q*v));
        gradient.setColorAt(i/nPt, c);
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
    return (1.0 / (2 * M_PI * pow(sigma, 2)) * exp( - pow(x - center, 2) / (2 * pow(sigma, 2))));
}

/*!
 * \brief MainWindow::mouseDoubleClickEvent
 * When the blank area at the top of window is double-clicked the window get maximized
 * \param event
 */
void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
#ifndef __APPLE__
    maximizeWindow();
#else
    this->maximizeWindowMac();
#endif
    event->accept();
}

/*!
 * \brief MainWindow::leaveEvent
 */
void MainWindow::leaveEvent(QEvent *)
{
    this->unsetCursor();
}

/*!
 * \brief MainWindow::setVisibilityOfFrameRightNonEditor
 * Either show or hide all widgets which are not m_textEdit
 */
void MainWindow::setVisibilityOfFrameRightNonEditor(bool isVisible)
{
    m_isFrameRightTopWidgetsVisible = isVisible;

    if(isVisible) {
        m_areNonEditorWidgetsVisible = true;
        m_editorDateLabel->setVisible(true);
        m_newNoteButton->setVisible(true);
        m_trashButton->setVisible(true);
        m_dotsButton->setVisible(true);
        m_styleEditorButton->setVisible(true);
    } else {
        m_areNonEditorWidgetsVisible = false;
        m_editorDateLabel->setVisible(false);
        m_newNoteButton->setVisible(false);
        m_trashButton->setVisible(false);
        m_dotsButton->setVisible(false);
        m_styleEditorButton->setVisible(false);
    }


    // If the notes list is collapsed, hide the window buttons
    if(m_splitter != Q_NULLPTR) {
        QList<int> sizes = m_splitter->sizes();
        if(sizes.at(0) == 0) {
#ifdef __APPLE__

            this->setStandardWindowButtonsMacVisibility(isVisible);
#else
            m_redCloseButton->setVisible(isVisible);
            m_yellowMinimizeButton->setVisible(isVisible);
            m_greenMaximizeButton->setVisible(isVisible);
#endif
        }

    }
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
    switch (event->type()){
    case QEvent::Enter:{
        if(qApp->applicationState() == Qt::ApplicationActive){
#ifdef _WIN32
            if(object == m_redCloseButton){
                m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/windows_close_hovered.png")));
            }

            if(object == m_yellowMinimizeButton){
                if(this->windowState() == Qt::WindowFullScreen){
                    m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/windows_de-maximize_hovered.png")));
                }else{
                    m_yellowMinimizeButton->setIcon(QIcon (QStringLiteral(":images/windows_maximize_hovered.png")));
                }
            }

            if(object == m_greenMaximizeButton){
                m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/windows_minimize_hovered.png")));
            }
#else
            // When hovering one of the traffic light buttons (red, yellow, green),
            // set new icons to show their function
            if(object == m_redCloseButton
                    || object == m_yellowMinimizeButton
                    || object == m_greenMaximizeButton){

                m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/redHovered.png")));
                m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/yellowHovered.png")));
                if(this->windowState() == Qt::WindowFullScreen){
                    m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/greenInHovered.png")));
                }else{
                    m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/greenHovered.png")));
                }
            }
#endif

            if(object == m_newNoteButton){
                this->setCursor(Qt::PointingHandCursor);
                m_newNoteButton->setIcon(QIcon(QStringLiteral(":/images/newNote_Hovered.png")));
            }

            if(object == m_trashButton){
                this->setCursor(Qt::PointingHandCursor);
                m_trashButton->setIcon(QIcon(QStringLiteral(":/images/trashCan_Hovered.png")));
            }

            if(object == m_dotsButton){
                this->setCursor(Qt::PointingHandCursor);
                m_dotsButton->setIcon(QIcon(QStringLiteral(":/images/3dots_Hovered.png")));
            }

            if(object == m_styleEditorButton){
                this->setCursor(Qt::PointingHandCursor);
                QString ss = QStringLiteral("QPushButton { "
                                 "  border: none; "
                                 "  padding: 0px; "
                                 "  color: rgb(51, 110, 162);"
                                 "}");
                m_styleEditorButton->setStyleSheet(ss);
            }

            if(object == ui->frameRightTop && !m_areNonEditorWidgetsVisible){
                setVisibilityOfFrameRightNonEditor(true);
            }
        }

        if(object == ui->frame){
            ui->centralWidget->setCursor(Qt::ArrowCursor);
        }

        break;
    }
    case QEvent::Leave:{
        if(qApp->applicationState() == Qt::ApplicationActive){
            // When not hovering, change back the icons of the traffic lights to their default icon
            if(object == m_redCloseButton
                    || object == m_yellowMinimizeButton
                    || object == m_greenMaximizeButton){

#ifdef _WIN32
                m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/windows_close_regular.png")));
                m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/windows_minimize_regular.png")));

                if(this->windowState() == Qt::WindowFullScreen){
                    m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/windows_de-maximize_regular.png")));
                }else{
                    m_yellowMinimizeButton->setIcon(QIcon (QStringLiteral(":images/windows_maximize_regular.png")));
                }
#else
                m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/red.png")));
                m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/yellow.png")));
                m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/green.png")));
#endif
            }

            if(object == m_newNoteButton){
                this->unsetCursor();
                m_newNoteButton->setIcon(QIcon(QStringLiteral(":/images/newNote_Regular.png")));
            }

            if(object == m_trashButton){
                this->unsetCursor();
                m_trashButton->setIcon(QIcon(QStringLiteral(":/images/trashCan_Regular.png")));
            }

            if(object == m_dotsButton){
                this->unsetCursor();
                m_dotsButton->setIcon(QIcon(QStringLiteral(":/images/3dots_Regular.png")));
            }

            if(object == m_styleEditorButton){
                this->unsetCursor();
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
    case QEvent::WindowDeactivate:{

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
    case QEvent::WindowActivate:{
#ifdef _WIN32
        m_redCloseButton->setIcon(QIcon(QStringLiteral(":images/windows_close_regular.png")));
        m_greenMaximizeButton->setIcon(QIcon(QStringLiteral(":images/windows_minimize_regular.png")));

        if(this->windowState() == Qt::WindowFullScreen){
            m_yellowMinimizeButton->setIcon(QIcon(QStringLiteral(":images/windows_de-maximize_regular.png")));
        }else{
            m_yellowMinimizeButton->setIcon(QIcon (QStringLiteral(":images/windows_maximize_regular.png")));
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
    case QEvent::HoverEnter:{
        if(object == m_textEdit->verticalScrollBar()){
            bool isSearching = !m_searchEdit->text().isEmpty();
            if(isSearching)
                m_textEdit->setFocusPolicy(Qt::NoFocus);
        }
        break;
    }
    case QEvent::HoverLeave:{
        bool isNoButtonClicked = qApp->mouseButtons() == Qt::NoButton;
        if(isNoButtonClicked){
            if(object == m_textEdit->verticalScrollBar()){
                m_textEdit->setFocusPolicy(Qt::StrongFocus);
            }
        }
        break;
    }
    case QEvent::MouseButtonRelease:{
        bool isMouseOnScrollBar = qApp->widgetAt(QCursor::pos()) != m_textEdit->verticalScrollBar();
        if(isMouseOnScrollBar){
            if(object == m_textEdit->verticalScrollBar()){
                m_textEdit->setFocusPolicy(Qt::StrongFocus);
            }
        }
        break;
    }
#endif
    case QEvent::FocusIn:{
        if(object == m_textEdit){
            if(!m_isOperationRunning){
                if(!m_searchEdit->text().isEmpty()){
                    if(!m_currentSelectedNoteProxy.isValid()){
                        clearSearch();
                        createNewNote();
                    }
                }else if(m_proxyModel->rowCount() == 0){
                    createNewNote();
                }
            }
            m_noteView->setCurrentRowActive(true);
            m_textEdit->setFocus();
        }

        if(object == m_searchEdit){
            setSearchEditStyleSheet(true);
        }
        break;
    }
    case QEvent::FocusOut:{
        if(object == m_searchEdit){
            setSearchEditStyleSheet(false);
        }
        break;
    }
    case QEvent::Resize:{
        if(m_textEdit->width() < m_smallEditorWidth) {
            m_currentMinimumEditorPadding = 15;
        } else if(m_textEdit->width() > m_smallEditorWidth && m_textEdit->width() < 515) {
            m_currentMinimumEditorPadding = 40;
        } else if(m_textEdit->width() > 515 && m_textEdit->width() < 755) {
            m_currentMinimumEditorPadding = 50;
        } else if(m_textEdit->width() > 755 && m_textEdit->width() < 775) {
            m_currentMinimumEditorPadding = 60;
        } else if(m_textEdit->width() > 775 && m_textEdit->width() < 800) {
            m_currentMinimumEditorPadding = 70;
        } else if(m_textEdit->width() > 800) {
            m_currentMinimumEditorPadding = 80;
        }

        setCurrentFontBasedOnTypeface(m_currentFontTypeface);

//        alignTextEditText();
    }
    case QEvent::Show:
        if(object == &m_updater){

            QRect rect = m_updater.geometry();
            QRect appRect = geometry();
            int titleBarHeight = 28 ;

            int x = int(appRect.x() + (appRect.width() - rect.width())/2.0);
            int y = int(appRect.y() + titleBarHeight  + (appRect.height() - rect.height())/2.0);

            m_updater.setGeometry(QRect(x, y, rect.width(), rect.height()));
        }
        break;
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
        // Only allow double click (maximise/minimise) or dragging (move)
        // from the top part of the window
        if(object == ui->frame){
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if(mouseEvent->y() >= ui->searchEdit->y()){
                return true;
            }
        }
        break;
    case QEvent::KeyPress:
    {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return && m_searchEdit->text().isEmpty()) {
            setFocusOnText();
        } else if (keyEvent->key() == Qt::Key_Return &&
                   keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            setFocusOnText();
            return true;
        }
        break;
    }
    case QEvent::MouseMove:
    {
        // Apperantly we need this if m_layoutMargin is set to 0
        if(object == ui->frame ||
                object == ui->frameLeft ||
                object == ui->frameRight ||
                object == ui->frameRightTop)
            mouseMoveEvent(static_cast<QMouseEvent*>(event));
    }
    default:
        break;
    }

    return QObject::eventFilter(object, event);
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
    this->setWindowAlwaysOnTopMac(checked);
#endif
}

/*!
 * \brief MainWindow::askBeforeSettingNativeWindowFrame
 */
void MainWindow::askBeforeSettingNativeWindowFrame()
{
#ifdef _WIN32
    if(!m_useNativeWindowFrame) {
        QMessageBox msgBox;
        msgBox.setText(tr("Warning: After performing this action, you will need to restart your system to revert to custom decoration."));
        msgBox.setInformativeText(tr("Would you like to continue?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() != QMessageBox::Yes) {
            return;
        }
    }
#endif

    setUseNativeWindowFrame(m_useNativeWindowFrame ? false : true);
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
    const int width = ui->verticalSpacer_upSearchEdit->sizeHint().width();
    ui->verticalSpacer_upSearchEdit->changeSize(width,
                                                shouldPushUp ? ui->verticalSpacer_upScrollArea->sizeHint().height() : 25,
                                                policy.horizontalPolicy(),
                                                policy.verticalPolicy());
    ui->verticalLayout_scrollArea->invalidate();

    // TODO: For some reason the layout update itself only when a button is being hovered upon
    // Adjust space above text editor
    ui->verticalSpacer_upEditorDateLabel->changeSize(width,
                                                     shouldPushUp ? ui->verticalSpacer_upScrollArea->sizeHint().height() : 25,
                                                     policy.horizontalPolicy(),
                                                     policy.verticalPolicy());
    ui->verticalLayout_textEdit->invalidate();
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

    auto flags = windowFlags();

#if defined(Q_OS_LINUX)
    if (useNativeWindowFrame)
        flags &= ~Qt::FramelessWindowHint;
    else
        flags |= Qt::FramelessWindowHint;
#elif _WIN32
    if (useNativeWindowFrame)
        flags &= ~Qt::CustomizeWindowHint;
    else
        flags |= Qt::CustomizeWindowHint;
#endif

    setWindowFlags(flags);
#endif


//#if defined(Q_OS_LINUX)
//    if (useNativeWindowFrame || isMaximized()) {
//        ui->centralWidget->layout()->setContentsMargins(QMargins());
//    } else {
//        QMargins margins(m_layoutMargin,m_layoutMargin,m_layoutMargin,m_layoutMargin);
//        ui->centralWidget->layout()->setContentsMargins(margins);
//    }
//#endif

//#ifndef _WIN32
//    if (useNativeWindowFrame || isMaximized()) {
//        ui->centralWidget->layout()->setContentsMargins(QMargins());
//    } else {
//        QMargins margins(m_layoutMargin,m_layoutMargin,m_layoutMargin,m_layoutMargin);
//        ui->centralWidget->layout()->setContentsMargins(margins);
//    }
//#endif

    adjustUpperWidgets(useNativeWindowFrame);

    setMainWindowVisibility(true);
}

/*!
 * \brief MainWindow::toggleStayOnTop
 */
void MainWindow::toggleStayOnTop()
{
    this->stayOnTop(!m_alwaysStayOnTop);
}

/*!
 * \brief MainWindow::onSearchEditReturnPressed
 */
void MainWindow::onSearchEditReturnPressed()
{
    if (m_searchEdit->text().isEmpty()) return;

    QString searchText = m_searchEdit->text();
    QTextDocument *doc = m_textEdit->document();
    //get current cursor
    QTextCursor from = m_textEdit->textCursor();
    //search
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
    m_trafficLightLayout.setGeometry(QRect(4+margins.left(),4+margins.top(),56,16));
}

/*!
 * \brief MainWindow::highlightSearch
 */
void MainWindow::highlightSearch() const
{
    QString searchString = m_searchEdit->text();

    if (searchString.isEmpty())
        return;

    m_textEdit->moveCursor(QTextCursor::Start);

    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextCharFormat highlightFormat;
    highlightFormat.setBackground(Qt::yellow);

    while (m_textEdit->find(searchString))
        extraSelections.append({ m_textEdit->textCursor(), highlightFormat});

    if (!extraSelections.isEmpty()) {
        m_textEdit->setTextCursor(extraSelections.first().cursor);
        m_textEdit->setExtraSelections(extraSelections);
    }
}

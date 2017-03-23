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
#define FIRST_LINE_MAX 80

/**
* Setting up the main window and it's content
*/
MainWindow::MainWindow (QWidget *parent) :
    QMainWindow (parent),
    ui (new Ui::MainWindow),
    m_autoSaveTimer(new QTimer(this)),
    m_settingsDatabase(Q_NULLPTR),
    m_noteWidgetsContainer(Q_NULLPTR),
    m_clearButton(Q_NULLPTR),
    m_greenMaximizeButton(Q_NULLPTR),
    m_redCloseButton(Q_NULLPTR),
    m_yellowMinimizeButton(Q_NULLPTR),
    m_newNoteButton(Q_NULLPTR),
    m_trashButton(Q_NULLPTR),
    m_dotsButton(Q_NULLPTR),
    m_textEdit(Q_NULLPTR),
    m_lineEdit(Q_NULLPTR),
    m_editorDateLabel(Q_NULLPTR),
    m_trayIcon(new QSystemTrayIcon(this)),
    m_restoreAction(new QAction(tr("&Hide Notes"), this)),
    m_quitAction(new QAction(tr("&Quit"), this)),
    m_trayIconMenu(new QMenu(this)),
    m_noteModel(new NoteModel(this)),
    m_deletedNotesModel(new NoteModel(this)),
    m_proxyModel(new QSortFilterProxyModel(this)),
    m_dbManager(Q_NULLPTR),
    m_noteCounter(0),
    m_trashCounter(0),
    m_layoutMargin(10),
    m_noteListWidth(200),
    m_canMoveWindow(false),
    m_canStretchWindow(false),
    m_isTemp(false),
    m_isListViewScrollBarHidden(true),
    m_isContentModified(false),
    m_isOperationRunning(false)
{
    ui->setupUi(this);
    setupMainWindow();
    setupFonts();
    setupTrayIcon();
    setupKeyboardShortcuts();
    setupNewNoteButtonAndTrahButton();
    setupSplitter();
    setupLine();
    setupRightFrame ();
    setupTitleBarButtons();
    setupLineEdit();
    setupTextEdit();
    setupDatabases();
    setupModelView();
    restoreStates();
    setupSignalsSlots();
    autoCheckForUpdates();

    QTimer::singleShot(200,this, SLOT(InitData()));
}

/**
 * @brief Init the data from database and select the first note if there is one
 */
void MainWindow::InitData()
{
    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());
    QString oldNoteDBPath(dir.path() + "/Notes.ini");
    QString oldTrashDBPath(dir.path() + "/Trash.ini");

    bool exist = (QFile::exists(oldNoteDBPath) || QFile::exists(oldTrashDBPath));

    if(exist){
        QProgressDialog* pd = new QProgressDialog("Migrating database, please wait.", "", 0, 0, this);
        pd->setCancelButton(0);
        pd->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        pd->setMinimumDuration(0);
        pd->show();

        setButtonsAndFieldsEnabled(false);

        QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
        connect(watcher, &QFutureWatcher<void>::finished, this, [&, pd](){
            pd->deleteLater();

            setButtonsAndFieldsEnabled(true);

            loadNotes();
            createNewNoteIfEmpty();
            selectFirstNote();
        });

        QFuture<void> migration = QtConcurrent::run(this, &MainWindow::checkMigration);
        watcher->setFuture(migration);

    }else{

        loadNotes();
        createNewNoteIfEmpty();
        selectFirstNote();
    }
}

void MainWindow::setMainWindowVisibility(bool state)
{
    if(state){
        showNormal();
        setWindowState(Qt::WindowNoState);
        qApp->processEvents();
        setWindowState(Qt::WindowActive);
        qApp->processEvents();
        qApp->setActiveWindow(this);
        qApp->processEvents();
        m_restoreAction->setText(tr("&Hide Notes"));
    }else{
        m_restoreAction->setText(tr("&Show Notes"));
        hide();
    }
}

/**
* @brief
* Deconstructor of the class
*/
MainWindow::~MainWindow ()
{
    delete ui;
}

/**
* @brief
* Setting up main window prefrences like frameless window and the minimum size of the window
* Setting the window background color to be white
*/
void MainWindow::setupMainWindow ()
{
#ifdef Q_OS_LINUX
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
#elif _WIN32
    this->setWindowFlags(Qt::CustomizeWindowHint);
#elif __APPLE__
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
#else
#error "We don't support that version yet..."
#endif

    m_greenMaximizeButton = ui->greenMaximizeButton;
    m_redCloseButton = ui->redCloseButton;
    m_yellowMinimizeButton = ui->yellowMinimizeButton;
    m_newNoteButton = ui->newNoteButton;
    m_trashButton = ui->trashButton;
    m_dotsButton = ui->dotsButton;
    m_lineEdit = ui->lineEdit;
    m_textEdit = ui->textEdit;
    m_editorDateLabel = ui->editorDateLabel;
    m_splitter = ui->splitter;

#ifndef _WIN32
    QMargins margins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin);
    ui->centralWidget->layout()->setContentsMargins(margins);
#endif
    ui->frame->installEventFilter(this);
    ui->centralWidget->setMouseTracking(true);
    this->setMouseTracking(true);

    QPalette pal(palette());
    pal.setColor(QPalette::Background, QColor(248, 248, 248));
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    m_newNoteButton->setToolTip("Create New Note");
    m_trashButton->setToolTip("Delete Selected Note");
    m_dotsButton->setToolTip("Open Menu");
}

void MainWindow::setupFonts()
{
#ifdef __APPLE__
    m_lineEdit->setFont(QFont("Helvetica Neue", 12));
    m_editorDateLabel->setFont(QFont("Helvetica Neue", 12, 65));
#else
    m_lineEdit->setFont(QFont(QStringLiteral("Roboto"), 10));
    m_editorDateLabel->setFont(QFont(QStringLiteral("Roboto"), 10, QFont::Bold));
#endif
}

void MainWindow::setupTrayIcon()
{
    m_trayIconMenu->addAction(m_restoreAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_quitAction);

    QIcon icon(":images/notes_system_tray_icon.png");
    m_trayIcon->setIcon(icon);
    m_trayIcon->setContextMenu(m_trayIconMenu);
    m_trayIcon->show();
}

/**
* @brief
* Setting up the keyboard shortcuts
*/
void MainWindow::setupKeyboardShortcuts ()
{
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_N), this, SLOT(onNewNoteButtonClicked()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Delete), this, SLOT(deleteSelectedNote()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), m_lineEdit, SLOT(setFocus()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_E), m_lineEdit, SLOT(clear()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this, SLOT(setFocusOnCurrentNote()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down), this, SLOT(selectNoteDown()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Up), this, SLOT(selectNoteUp()));
    new QShortcut(QKeySequence(Qt::Key_Down), this, SLOT(selectNoteDown()));
    new QShortcut(QKeySequence(Qt::Key_Up), this, SLOT(selectNoteUp()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this, SLOT(setFocusOnText()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return), this, SLOT(setFocusOnText()));
    new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(setFocusOnText()));
    new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(setFocusOnText()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F), this, SLOT(fullscreenWindow()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L), this, SLOT(maximizeWindow()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_M), this, SLOT(minimizeWindow()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(QuitApplication()));

    QxtGlobalShortcut *shortcut = new QxtGlobalShortcut(this);
    shortcut->setShortcut(QKeySequence("META+N"));
    connect(shortcut, &QxtGlobalShortcut::activated,[=]() {
        // workaround prevent textEdit and lineEdit
        // from taking 'N' from shortcut
        m_textEdit->setDisabled(true);
        m_lineEdit->setDisabled(true);
        setMainWindowVisibility(isHidden()
                                || windowState() == Qt::WindowMinimized
                                || qApp->applicationState() == Qt::ApplicationInactive);
        m_textEdit->setDisabled(false);
        m_lineEdit->setDisabled(false);
    });
}

/**
* @brief
* We need to set up some different values when using apple os x
* This is because if we want to get the native button look in os x,
* due to some bug in Qt, I think, the values of width and height of buttons
* needs to be more than 50 and less than 34 respectively.
* So some modifications needs to be done.
*/
void MainWindow::setupNewNoteButtonAndTrahButton ()
{
    QString ss = "QPushButton { "
                 "  border: none; "
                 "  padding: 0px; "
                 "}";

    m_newNoteButton->setStyleSheet(ss);
    m_trashButton->setStyleSheet(ss);
    m_dotsButton->setStyleSheet(ss);

    m_newNoteButton->installEventFilter(this);
    m_trashButton->installEventFilter(this);
    m_dotsButton->installEventFilter(this);
}

/**
* @brief
* Set up the splitter that control the size of the scrollArea and the textEdit
*/
void MainWindow::setupSplitter()
{
    m_splitter->setCollapsible(0, false);
    m_splitter->setCollapsible(1, false);
}

/**
* @brief
* Set up the vertical line that seperate between the scrollArea to the textEdit
*/
void MainWindow::setupLine ()
{
#ifdef __APPLE__
    ui->line->setStyleSheet("border: 0px solid rgb(221, 221, 221)");
#else
    ui->line->setStyleSheet("border: 1px solid rgb(221, 221, 221)");
#endif
}

/**
* @brief
* Set up a frame above textEdit and behind the other widgets for a unifed background in thet editor section
*/
void MainWindow::setupRightFrame ()
{
    QString ss = "QFrame{ "
                 "  background-image: url(:images/textEdit_background_pattern.png); "
                 "  border: none;"
                 "}";
    ui->frameRight->setStyleSheet(ss);
}

/**
* @brief
* Setting up the red (close), yellow (minimize), and green (maximize) buttons
* Make only the buttons icon visible
* And install this class event filter to them, to act when hovering on one of them
*/
void MainWindow::setupTitleBarButtons ()
{
    QString ss = "QPushButton { "
                 "  border: none; "
                 "  padding: 0px; "
                 "}";

    m_redCloseButton->setStyleSheet(ss);
    m_yellowMinimizeButton->setStyleSheet(ss);
    m_greenMaximizeButton->setStyleSheet(ss);

#ifdef _WIN32
    m_redCloseButton->setIcon(QIcon(":images/windows_close_regular.png"));
    m_yellowMinimizeButton->setIcon(QIcon(":images/windows_maximize_regular.png"));
    m_greenMaximizeButton->setIcon(QIcon(":images/windows_minimize_regular.png"));

    m_redCloseButton->setMinimumSize(34, 16);
    m_yellowMinimizeButton->setMinimumSize(28, 16);
    m_greenMaximizeButton->setMinimumSize(28, 16);

    ui->horizontalSpacer_leftTrafficLightButtons->changeSize(2, 20);
    ui->horizontalSpacer_rightRedTrafficLightButton->changeSize(0, 20);
    ui->horizontalSpacer_rightYellowTrafficLightButton->changeSize(0, 20);
    ui->verticalSpacer_upLineEdit->changeSize(20, 0);
#endif

    m_redCloseButton->installEventFilter(this);
    m_yellowMinimizeButton->installEventFilter(this);
    m_greenMaximizeButton->installEventFilter(this);
}

/**
 * @brief connect between signals and slots
 */
void MainWindow::setupSignalsSlots()
{
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
    // text edit text changed
    connect(m_textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextEditTextChanged);
    // line edit text changed
    connect(m_lineEdit, &QLineEdit::textChanged, this, &MainWindow::onLineEditTextChanged);
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
}

/**
 * Checks for updates, if an update is found, then the updater dialog will show
 * up, otherwise, no notification shall be showed
 */
void MainWindow::autoCheckForUpdates()
{
    m_updater.checkForUpdates (true);
}

/**
* @brief
* Set the lineedit to start a bit to the right and end a bit to the left (pedding)
*/
void MainWindow::setupLineEdit ()
{

    QLineEdit* lineEdit = m_lineEdit;

    int frameWidth = m_lineEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    QString ss = QString("QLineEdit{ "
                         "  padding-right: %1px; "
                         "  padding-left: 21px;"
                         "  padding-right: 19px;"
                         "  border: 1px solid rgb(205, 205, 205);"
                         "  border-radius: 3px;"
                         "  background: rgb(255, 255, 255);"
                         "  selection-background-color: rgb(61, 155, 218);"
                         "} "
                         "QToolButton { "
                         "  border: none; "
                         "  padding: 0px;"
                         "}"
                         ).arg(frameWidth + 1);

    lineEdit->setStyleSheet(ss);

    // clear button
    m_clearButton = new QToolButton(lineEdit);
    QPixmap pixmap(":images/closeButton.png");
    m_clearButton->setIcon(QIcon(pixmap));
    QSize clearSize(15, 15);
    m_clearButton->setIconSize(clearSize);
    m_clearButton->setCursor(Qt::ArrowCursor);
    m_clearButton->hide();

    // search button
    QToolButton *searchButton = new QToolButton(lineEdit);
    QPixmap newPixmap(":images/magnifyingGlass.png");
    searchButton->setIcon(QIcon(newPixmap));
    QSize searchSize(24, 25);
    searchButton->setIconSize(searchSize);
    searchButton->setCursor(Qt::ArrowCursor);

    // layout
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::RightToLeft, lineEdit);
    layout->setContentsMargins(0,0,3,0);
    layout->addWidget(m_clearButton);
    layout->addStretch();
    layout->addWidget(searchButton);
    lineEdit->setLayout(layout);

    lineEdit->installEventFilter(this);
}

/**
* @brief
* Setting up textEdit:
* Setup the style of the scrollBar and set textEdit background to an image
* Make the textEdit pedding few pixels right and left, to compel with a beautiful proportional grid
* And install this class event filter to catch when text edit is having focus
*/
void MainWindow::setupTextEdit ()
{
    QString ss = QString("QTextEdit {background-image: url(:images/textEdit_background_pattern.png); padding-left: %1px; padding-right: %2px; padding-bottom:2px;} "
                         "QScrollBar::handle:vertical:hover { background: rgb(170, 170, 171); } "
                         "QScrollBar::handle:vertical:pressed { background: rgb(149, 149, 149); } "
                         "QScrollBar::handle:vertical { border-radius: 4px; background: rgb(188, 188, 188); min-height: 20px; }  "
                         "QScrollBar::vertical {border-radius: 4px; width: 8px; color: rgba(255, 255, 255,0);} "
                         "QScrollBar {margin: 0; background: transparent;} "
                         "QScrollBar:hover { background-color: rgb(217, 217, 217);}"
                         "QScrollBar::add-line:vertical { width:0px; height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  "
                         "QScrollBar::sub-line:vertical { width:0px; height: 0px; subcontrol-position: top; subcontrol-origin: margin; }"
                         ).arg("27", "27");

    m_textEdit->setStyleSheet(ss);

    m_textEdit->installEventFilter(this);
    m_textEdit->verticalScrollBar()->installEventFilter(this);
    m_textEdit->setFont(QFont(QStringLiteral("Arimo"), 11, QFont::Normal));

    // This is done because for now where we're only handling plain text,
    // and we don't want people to past rich text and get something wrong.
    // In future versions, where we'll support rich text, we'll need to change that.
    m_textEdit->setAcceptRichText(false);

#ifdef __APPLE__
    m_textEdit->setFont(QFont("Helvetica Neue", 14));
#else
    m_textEdit->setTextColor(QColor(26, 26, 26));
#endif
}

void MainWindow::initializeSettingsDatabase()
{
    if(m_settingsDatabase->value("version", "NULL") == "NULL")
        m_settingsDatabase->setValue("version", "0.8.0");

    if(m_settingsDatabase->value("defaultWindowWidth", "NULL") == "NULL")
        m_settingsDatabase->setValue("defaultWindowWidth", 757);

    if(m_settingsDatabase->value("defaultWindowHeight", "NULL") == "NULL")
        m_settingsDatabase->setValue("defaultWindowHeight", 341);

    if(m_settingsDatabase->value("windowGeometry", "NULL") == "NULL")
        m_settingsDatabase->setValue("windowGeometry", saveGeometry());

    if(m_settingsDatabase->value("splitterSizes", "NULL") == "NULL")
        m_settingsDatabase->setValue("splitterSizes", m_splitter->saveState());
}

/**
* @brief
* Setting up the database:
* The "Company" name is: 'Awesomeness' (So you don't have to scroll when getting to the .config folder)
* The Application name is: 'Notes'
* If it's the first run (or the database is deleted) , create the database and the notesCounter key
* (notesCounter increases it's value everytime a new note is created)
* We chose the Ini format for all operating systems because of future reasons - it might be easier to
* sync databases between diffrent os's when you have one consistent file format. We also think that this
* format, in the way Qt is handling it, is very good for are needs.
* Also because the native format on windows - the registery is very limited.
* The databases are stored in the user scope of the computer. That's mostly in (Qt takes care of this automatically):
* Linux: /home/user/.config/Awesomeness/
* Windows: C:\Users\user\AppData\Roaming\Awesomeness
* Mac OS X:
*/
void MainWindow::setupDatabases ()
{
    m_settingsDatabase = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Awesomeness", "Settings", this);
    m_settingsDatabase->setFallbacksEnabled(false);
    initializeSettingsDatabase();

    bool doCreate = false;
    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());
    QString noteDBFilePath(dir.path() + "/notes.db");

    if(!QFile::exists(noteDBFilePath)){
        QFile noteDBFile(noteDBFilePath);
        if(!noteDBFile.open(QIODevice::WriteOnly)){
            qDebug() << "can't create db file";
            qApp->exit(-1);
        }
        noteDBFile.close();
        doCreate = true;
    }

    m_dbManager = new DBManager(noteDBFilePath, doCreate, this);
    m_noteCounter = m_dbManager->getLastRowID();
}

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

/**
* @brief
* Restore the latest sates (if there are any) of the window and the splitter from the settings database
*/
void MainWindow::restoreStates()
{
    this->restoreGeometry(m_settingsDatabase->value("windowGeometry").toByteArray());

    m_splitter->restoreState(m_settingsDatabase->value("splitterSizes").toByteArray());

    // If scrollArea is collapsed
    if(m_splitter->sizes().at(0) == 0){
        ui->verticalLayout_scrollArea->removeItem(ui->horizontalLayout_scrollArea_2);
        ui->verticalLayout_textEdit->insertLayout(0, ui->horizontalLayout_scrollArea_2, 0);

        ui->verticalLayout_scrollArea->removeItem(ui->verticalSpacer_upLineEdit);
        ui->verticalLayout_textEdit->insertItem(0, ui->verticalSpacer_upLineEdit);
        ui->verticalSpacer_upEditorDateLabel->changeSize(20, 5);
    }
}

/**
* @brief
* Get a string 'str' and return only the first line of it
* If the string contain no text, return "New Note"
* TODO: We might make it more efficient by not loading the entire string into the memory
*/
QString MainWindow::getFirstLine (const QString& str)
{
    if(str.simplified().isEmpty())
        return "New Note";

    QString text = str.trimmed();
    QTextStream ts(&text);
    return std::move(ts.readLine(FIRST_LINE_MAX));
}

/**
* @brief
* Get a date string of a note from database and put it's data into a QDateTime object
* This function is not efficient
* If QVariant would include toStdString() it might help, aka: notesDatabase->value().toStdString
*/
QDateTime MainWindow::getQDateTime (QString date)
{
    QDateTime dateTime = QDateTime::fromString(date, Qt::ISODate);
    return dateTime;
}

/**
* @brief
* Get the full date of the selected note from the database and return it as a string in form for editorDateLabel
*/
QString MainWindow::getNoteDateEditor (QString dateEdited)
{
    QDateTime dateTimeEdited(getQDateTime(dateEdited));
    QLocale usLocale(QLocale("en_US"));

    return usLocale.toString(dateTimeEdited, "MMMM d, yyyy, h:mm A");
}

/**
* @brief
* @brief generate a new note
*/
NoteData *MainWindow::generateNote(QString noteName)
{
    NoteData* newNote = new NoteData(this);
    newNote->setId(noteName);

    QDateTime noteDate = QDateTime::currentDateTime();
    newNote->setCreationDateTime(noteDate);
    newNote->setLastModificationDateTime(noteDate);
    newNote->setFullTitle(QStringLiteral("New Note"));

    return newNote;
}

/**
 * @brief show the specified note content text in the text editor
 * Set editorDateLabel text to the the selected note date
 * And restore the scrollBar position if it changed before.
 */
void MainWindow::showNoteInEditor(const QModelIndex &noteIndex)
{
    m_textEdit->blockSignals(true);
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
}

/**
* @brief
* Load all the notes from database
* add data to the models
* sort them according to the date
* update scrollbar stylesheet
*/
void MainWindow::loadNotes ()
{
    QList<NoteData*> noteList = m_dbManager->getAllNotes();

    if(!noteList.isEmpty()){
        m_noteModel->addListNote(noteList);
        m_noteModel->sort(0,Qt::AscendingOrder);
    }
}

/**
* @brief
* save the current note to database
*/
void MainWindow::saveNoteToDB(const QModelIndex &noteIndex)
{
    if(noteIndex.isValid() && m_isContentModified){
        QModelIndex indexInSrc = m_proxyModel->mapToSource(noteIndex);
        NoteData* note = m_noteModel->getNote(indexInSrc);
        if(note != Q_NULLPTR){
            bool doExist = m_dbManager->isNoteExist(note);
            if(doExist){
                QtConcurrent::run(m_dbManager, &DBManager::modifyNote, note);
            }else{
                QtConcurrent::run(m_dbManager, &DBManager::addNote, note);
            }
        }

        m_isContentModified = false;
    }
}

void MainWindow::removeNoteFromDB(const QModelIndex& noteIndex)
{
    if(noteIndex.isValid()){
        QModelIndex indexInSrc = m_proxyModel->mapToSource(noteIndex);
        NoteData* note = m_noteModel->getNote(indexInSrc);
        m_dbManager->removeNote(note);
    }
}

/**
* @brief
* Select the first note in the notes list
*/
void MainWindow::selectFirstNote ()
{
    if(m_proxyModel->rowCount() > 0){
        QModelIndex index = m_proxyModel->index(0,0);
        m_noteView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        m_noteView->setCurrentIndex(index);

        m_currentSelectedNoteProxy = index;
        showNoteInEditor(index);
    }
}

/**
* @brief
* create a new note if there are no notes
*/
void MainWindow::createNewNoteIfEmpty ()
{
    if(m_proxyModel->rowCount() == 0)
        createNewNote();
}

void MainWindow::setButtonsAndFieldsEnabled(bool doEnable)
{
    m_greenMaximizeButton->setEnabled(doEnable);
    m_redCloseButton->setEnabled(doEnable);
    m_yellowMinimizeButton->setEnabled(doEnable);
    m_newNoteButton->setEnabled(doEnable);
    m_trashButton->setEnabled(doEnable);
    m_lineEdit->setEnabled(doEnable);
    m_textEdit->setEnabled(doEnable);
}

/**
* @brief
* When the new-note button is pressed, set it's icon accordingly
*/
void MainWindow::onNewNoteButtonPressed()
{
    m_newNoteButton->setIcon(QIcon(":/images/newNote_Pressed.png"));
}

/**
* @brief
* Create a new note when clicking the 'new note' button
*/
void MainWindow::onNewNoteButtonClicked()
{
    m_newNoteButton->setIcon(QIcon(":/images/newNote_Regular.png"));

    if(!m_lineEdit->text().isEmpty()){
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

/**
* @brief
* When the trash button is pressed, set it's icon accordingly
*/
void MainWindow::onTrashButtonPressed()
{
    m_trashButton->setIcon(QIcon(":/images/trashCan_Pressed.png"));
}

/**
* @brief
* Delete selected note when clicking the 'delete note' button
*/
void MainWindow::onTrashButtonClicked()
{
    m_trashButton->setIcon(QIcon(":/images/trashCan_Regular.png"));

    m_trashButton->blockSignals(true);
    this->deleteSelectedNote();
    m_trashButton->blockSignals(false);
}

/**
* @brief
* When the 3 dots button is pressed, set it's icon accordingly
*/
void MainWindow::onDotsButtonPressed()
{
    m_dotsButton->setIcon(QIcon(":/images/3dots_Pressed.png"));
}

/**
* @brief
* Open up the menu when clicking the 3 dots button
*/
void MainWindow::onDotsButtonClicked()
{
    m_dotsButton->setIcon(QIcon(":/images/3dots_Regular.png"));

    QMenu mainMenu;
    QMenu* viewMenu = mainMenu.addMenu("View");
    QMenu* importExportNotesMenu = mainMenu.addMenu("Import/Export Notes");

    mainMenu.setStyleSheet("QMenu { "
                              "  background-color: rgb(247, 247, 247); "
                              "  border: 1px solid #308CC6; "
                              "  }"
                              "QMenu::item:selected { "
                              "  background: 1px solid #308CC6; "
                              "  }");

#ifdef __APPLE__
    mainMenu.setFont(QFont("Helvetica Neue", 13));
    viewMenu->setFont(QFont("Helvetica Neue", 13));
    importExportNotesMenu->setFont(QFont("Helvetica Neue", 13));
#else
    mainMenu.setFont(QFont(QStringLiteral("Roboto"), 10, QFont::Normal));
    viewMenu->setFont(QFont(QStringLiteral("Roboto"), 10, QFont::Normal));
    importExportNotesMenu->setFont(QFont(QStringLiteral("Roboto"), 10, QFont::Normal));
#endif

    // note list visiblity action
    bool isCollapsed = (m_splitter->sizes().at(0) == 0);
    QString actionLabel = isCollapsed? tr("Show notes list")
                                     : tr("Hide notes list");

    QAction* noteListVisbilityAction = viewMenu->addAction(actionLabel);
    if(isCollapsed){
        connect(noteListVisbilityAction, SIGNAL(triggered(bool)), this, SLOT(expandNoteList()));
    }else{
        connect(noteListVisbilityAction, SIGNAL(triggered(bool)), this, SLOT(collapseNoteList()));
    }

    // Check for update action
    QAction* checkForUpdatesAction = mainMenu.addAction (tr("Check For Updates"));
    connect (checkForUpdatesAction, SIGNAL (triggered (bool)),
             this, SLOT (checkForUpdates (bool)));

    // Import notes action
    QAction* importNotesFileAction = importExportNotesMenu->addAction (tr("Import"));
    connect (importNotesFileAction, SIGNAL (triggered (bool)),
             this, SLOT (importNotesFile (bool)));

    // Export notes action
    QAction* exportNotesFileAction = importExportNotesMenu->addAction (tr("Export"));
    connect (exportNotesFileAction, SIGNAL (triggered (bool)),
             this, SLOT (exportNotesFile (bool)));

    // Export disabled if no notes exist
    if(m_noteModel->rowCount() < 1){
        exportNotesFileAction->setDisabled(true);
    }

    mainMenu.exec(m_dotsButton->mapToGlobal(QPoint(0, m_dotsButton->height())));
}

/**
* @brief
* When clicking on a note in the scrollArea:
* Unhighlight the previous selected note
* If selecting a note when temporery note exist, delete the temp note
* Highlight the selected note
* Load the selected note content into textedit
*/
void MainWindow::onNotePressed (const QModelIndex& index)
{
    if(sender() != Q_NULLPTR){
        QModelIndex indexInProxy = m_proxyModel->index(index.row(), 0);
        selectNote(indexInProxy);
    }
}

/**
* @brief
* When the text on textEdit change:
* if the note edited is not on top of the list, we will make that happen
* If the text changed is of a new (empty) note, reset temp values
*/
void MainWindow::onTextEditTextChanged ()
{
    if(m_currentSelectedNoteProxy.isValid()){
        m_textEdit->blockSignals(true);
        QString content = m_currentSelectedNoteProxy.data(NoteModel::NoteContent).toString();
        if(m_textEdit->toPlainText() != content){

            m_autoSaveTimer->start(500);

            // move note to the top of the list
            if(m_currentSelectedNoteProxy.row() != 0)
                moveNoteToTop();

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
        }

        m_textEdit->blockSignals(false);

        m_isTemp = false;
    }else{
        qDebug() << "MainWindow::onTextEditTextChanged() : m_currentSelectedNoteProxy is not valid";
    }
}

/**
* @brief
* When text on lineEdit change:
* If there is a temp note "New Note" while searching, we delete it
* Saving the last selected note for recovery after searching
* Clear all the notes from scrollArea and
* If text is empty, reload all the notes from database
* Else, load all the notes contain the string in lineEdit from database
*/
void MainWindow::onLineEditTextChanged (const QString &keyword)
{
    m_textEdit->clearFocus();
    m_searchQueue.enqueue(keyword);

    if(!m_isOperationRunning){
        m_isOperationRunning = true;
        if(m_isTemp){
            m_isTemp = false;
            // prevent the line edit from emitting signal
            // while animation for deleting the new note is running
            m_lineEdit->blockSignals(true);
            m_currentSelectedNoteProxy = QModelIndex();
            QModelIndex index = m_noteModel->index(0);
            m_noteModel->removeNote(index);
            m_lineEdit->blockSignals(false);

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

        // tell the noteView that we are searching to disable the animation
        m_noteView->setSearching(true);

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

        m_isOperationRunning = false;
    }
}
/**
 * @brief MainWindow::onClearButtonClicked clears the search and
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

/**
 * @brief create a new note
 * add it to the database
 * add it to the scrollArea
 */
void MainWindow::createNewNote ()
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
            QString noteID = QString("noteID_%1").arg(m_noteCounter);
            NoteData* tmpNote = generateNote(noteID);
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

/**
 * @brief MainWindow::deleteNote deletes the specified note
 * @param note  : note to delete
 * @param isFromUser :  true if the user clicked on trash button
 */
void MainWindow::deleteNote(const QModelIndex &noteIndex, bool isFromUser)
{
    if(noteIndex.isValid()){
        // delete from model
        QModelIndex indexToBeRemoved = m_proxyModel->mapToSource(m_currentSelectedNoteProxy);
        NoteData* noteTobeRemoved = m_noteModel->removeNote(indexToBeRemoved);

        if(m_isTemp){
            m_isTemp = false;
        }else{
            noteTobeRemoved->setDeletionDateTime(QDateTime::currentDateTime());
            QtConcurrent::run(m_dbManager, &DBManager::removeNote, noteTobeRemoved);
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

/**
* @brief
* Delete the selected note
*/
void MainWindow::deleteSelectedNote ()
{
    if(!m_isOperationRunning){
        m_isOperationRunning = true;
        if(m_currentSelectedNoteProxy.isValid()){

            // update the index of the selected note before searching
            if(!m_lineEdit->text().isEmpty()){
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

/**
* @brief
* Set focus on textEdit
*/
void MainWindow::setFocusOnText ()
{
    if(m_currentSelectedNoteProxy.isValid() && !m_textEdit->hasFocus())
        m_textEdit->setFocus();
}

/**
* @brief
* Set focus on current selected note
*/
void MainWindow::setFocusOnCurrentNote ()
{
    if(m_currentSelectedNoteProxy.isValid())
        m_noteView->setFocus();
}

/**
* @brief
* Select the note above the currentSelectedNote
*/
void MainWindow::selectNoteUp ()
{
    if(m_currentSelectedNoteProxy.isValid()){
        int currentRow = m_noteView->currentIndex().row();
        QModelIndex aboveIndex = m_noteView->model()->index(currentRow - 1, 0);
        if(aboveIndex.isValid()){
            m_noteView->setCurrentIndex(aboveIndex);
            m_currentSelectedNoteProxy = aboveIndex;
            showNoteInEditor(m_currentSelectedNoteProxy);
        }
        m_noteView->setFocus();
    }
}

/**
* @brief
* Select the note below the currentSelectedNote
*/
void MainWindow::selectNoteDown ()
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
                m_currentSelectedNoteProxy = belowIndex;
                showNoteInEditor(m_currentSelectedNoteProxy);
            }
        }
        m_noteView->setFocus();
    }
}

/**
* @brief
* Switch to fullscreen mode
*/
void MainWindow::fullscreenWindow ()
{
#ifndef _WIN32
    QMargins margins(m_layoutMargin,m_layoutMargin,m_layoutMargin,m_layoutMargin);

    if(isFullScreen()){
        if(!isMaximized())
            ui->centralWidget->layout()->setContentsMargins(margins);

        setWindowState(windowState() & ~Qt::WindowFullScreen);
    }else{
        setWindowState(windowState() | Qt::WindowFullScreen);
        ui->centralWidget->layout()->setContentsMargins(0,0,0,0);
    }

#else
    if(isFullScreen()){
        showNormal();
    }else{
        showFullScreen();
    }
#endif
}

/**
* @brief
* Maximize the window
*/
void MainWindow::maximizeWindow ()
{
#ifndef _WIN32
    QMargins margins(m_layoutMargin,m_layoutMargin,m_layoutMargin,m_layoutMargin);

    if(isMaximized()){
        if(!isFullScreen()){
            ui->centralWidget->layout()->setContentsMargins(margins);
            setWindowState(windowState() & ~Qt::WindowMaximized);
        }else{
            setWindowState(windowState() & ~Qt::WindowFullScreen);
        }

    }else{
        setWindowState(windowState() | Qt::WindowMaximized);
        ui->centralWidget->layout()->setContentsMargins(0,0,0,0);
    }
#else
    if(isMaximized()){
        setWindowState(windowState() & ~Qt::WindowMaximized);
    }else if(isFullScreen()){
        setWindowState((windowState() | Qt::WindowMaximized) & ~Qt::WindowFullScreen);
        setGeometry(qApp->primaryScreen()->availableGeometry());
    }else{
        setWindowState(windowState() | Qt::WindowMaximized);
        setGeometry(qApp->primaryScreen()->availableGeometry());
    }
#endif
}

/**
* @brief
* Minimize the window
*/
void MainWindow::minimizeWindow ()
{
#ifndef _WIN32
    QMargins margins(m_layoutMargin,m_layoutMargin,m_layoutMargin,m_layoutMargin);
    ui->centralWidget->layout()->setContentsMargins(margins);
#endif

    // BUG : QTBUG-57902 minimize doesn't store the window state before minimizing
    showMinimized();
}

/**
* @brief
* Exit the application
* If a new note created but wasn't edited, delete it from the database
*/
void MainWindow::QuitApplication ()
{
    MainWindow::close();
}


/**
 * Called when the "Check for Updates" menu item is clicked, this function
 * instructs the updater window to check if there are any updates available
 *
 * \note This code won't be executed under Linux builds
 * \param clicked required by the signal/slot connection, the value is ignored
 */
void MainWindow::checkForUpdates (const bool clicked) {
    Q_UNUSED (clicked);
    m_updater.checkForUpdates (false);
}

/**
 * Called when the "Import Notes" menu button is clicked. this function will
 * prompt the user to select a file, attempt to load the file, and update the DB
 * if valid.
 * The user is presented with a dialog box if the upload/import fails for any reason.
 *
 * @brief MainWindow::importNotesFile
 * @param clicked
 */
void MainWindow::importNotesFile (const bool clicked) {
    Q_UNUSED (clicked);
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
        in.setVersion(QDataStream::Qt_5_6);
        qInfo() << "XXXXXXXXXXXXX 222";
        try {
            in >> noteList;
        } catch (...) {
            // Any exception deserializing will result in an empty note list and  the user will be notified
        }

        if (noteList.isEmpty()) {
            QMessageBox::information(this, tr("Invalid file"), "Please select a valid notes export file");
            return;
        }
        qInfo() << "XXXXXXXXXXXXX 333";

        QProgressDialog* pd = new QProgressDialog("Importing Notes...", "", 0, 0, this);
        pd->setCancelButton(0);
        pd->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        pd->setMinimumDuration(0);
        pd->show();
        pd->setValue(1);

        setButtonsAndFieldsEnabled(false);

        QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
        connect(watcher, &QFutureWatcher<void>::finished, this, [&, pd](){
            pd->deleteLater();

            setButtonsAndFieldsEnabled(true);

            m_noteModel->clearNotes();
            loadNotes();
            createNewNoteIfEmpty();
            selectFirstNote();
        });

        QFuture<void> migration = QtConcurrent::run(this, &MainWindow::importNotes, noteList);
        watcher->setFuture(migration);
    }
}

void MainWindow::importNotes(QList<NoteData*> noteList) {
    qInfo() << "XXXXXXXXXXXXX 111";
    QtConcurrent::blockingMap(noteList, [this] (NoteData* note) { m_dbManager->importNote(note); });
}

/**
 * Called when the "Export Notes" menu button is clicked. this function will
 * prompt the user to select a location for the export file, and then builds
 * the file.
 * The user is presented with a dialog box if the file cannot be opened for any reason.
 *
 * @brief MainWindow::exportNotesFile
 * @param clicked
 */
void MainWindow::exportNotesFile (const bool clicked) {
    Q_UNUSED (clicked);
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
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_6);
        out << m_dbManager->getAllNotes();
    }
}

void MainWindow::collapseNoteList()
{
    m_splitter->setCollapsible(0, true);
    QList<int> sizes = m_splitter->sizes();
    m_noteListWidth = sizes.at(0);
    sizes[0] = 0;
    m_splitter->setSizes(sizes);
    m_splitter->setCollapsible(0, false);
}

void MainWindow::expandNoteList()
{
    QList<int> sizes = m_splitter->sizes();
    sizes[0] = m_noteListWidth;
    sizes[1] = m_splitter->width() - m_noteListWidth;
    m_splitter->setSizes(sizes);
}

/**
* @brief
* When the green button is pressed set it's icon accordingly
*/
void MainWindow::onGreenMaximizeButtonPressed ()
{
#ifdef _WIN32
    m_greenMaximizeButton->setIcon(QIcon(":images/windows_minimize_pressed.png"));
#else
    if(this->windowState() == Qt::WindowFullScreen){
        m_greenMaximizeButton->setIcon(QIcon(":images/greenInPressed.png"));
    }else{
        m_greenMaximizeButton->setIcon(QIcon(":images/greenPressed.png"));
    }
#endif
}

/**
* @brief
* When the yellow button is pressed set it's icon accordingly
*/
void MainWindow::onYellowMinimizeButtonPressed ()
{
#ifdef _WIN32
    if(this->windowState() == Qt::WindowFullScreen){
        m_yellowMinimizeButton->setIcon(QIcon(":images/windows_de-maximize_pressed.png"));
    }else{
        m_yellowMinimizeButton->setIcon(QIcon(":images/windows_maximize_pressed.png"));
    }
#else
    m_yellowMinimizeButton->setIcon(QIcon(":images/yellowPressed.png"));
#endif
}

/**
* @brief
* When the red button is pressed set it's icon accordingly
*/
void MainWindow::onRedCloseButtonPressed ()
{
#ifdef _WIN32
    m_redCloseButton->setIcon(QIcon(":images/windows_close_pressed.png"));
#else
    m_redCloseButton->setIcon(QIcon(":images/redPressed.png"));
#endif
}

/**
* @brief
* When the green button is released the window goes fullscrren
*/
void MainWindow::onGreenMaximizeButtonClicked()
{
#ifdef _WIN32
    m_greenMaximizeButton->setIcon(QIcon(":images/windows_minimize_regular.png"));

    minimizeWindow();
    m_restoreAction->setText(tr("&Show Notes"));
#else
    m_greenMaximizeButton->setIcon(QIcon(":images/green.png"));

    fullscreenWindow();
#endif
}

/**
* @brief
* When yellow button is released the window is minimized
*/
void MainWindow::onYellowMinimizeButtonClicked()
{
#ifdef _WIN32
    m_yellowMinimizeButton->setIcon(QIcon(":images/windows_de-maximize_regular.png"));

    fullscreenWindow();
#else
    m_yellowMinimizeButton->setIcon(QIcon(":images/yellow.png"));

    minimizeWindow();
    m_restoreAction->setText(tr("&Show Notes"));
#endif
}

/**
* @brief
* When red button is released the window get closed
* If a new note created but wasn't edited, delete it from the database
*/
void MainWindow::onRedCloseButtonClicked()
{
#ifdef _WIN32
    m_redCloseButton->setIcon(QIcon(":images/windows_close_regular.png"));
#else
    m_redCloseButton->setIcon(QIcon(":images/red.png"));
#endif

    setMainWindowVisibility(false);
}

/**
 * @brief Called when the app is about the close
 * save the geometry of the app to the settings
 * save the current note if it's note temporary one otherwise remove it from DB
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    if(windowState() != Qt::WindowFullScreen && windowState() != Qt::WindowMaximized)
        m_settingsDatabase->setValue("windowGeometry", saveGeometry());

    if(m_currentSelectedNoteProxy.isValid()
            &&  m_isContentModified
            && !m_isTemp){

        saveNoteToDB(m_currentSelectedNoteProxy);
    }

    m_settingsDatabase->setValue("splitterSizes", m_splitter->saveState());
    m_settingsDatabase->sync();

    QWidget::closeEvent(event);
}
#ifndef _WIN32
/**
* @brief
* Set variables to the position of the window when the mouse is pressed
*/
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    m_mousePressX = event->x();
    m_mousePressY = event->y();

    if(m_mousePressX < this->width() - m_layoutMargin
            && m_mousePressX >m_layoutMargin
            && m_mousePressY < this->height() - m_layoutMargin
            && m_mousePressY > m_layoutMargin){

        m_canMoveWindow = true;
    }else{
        m_canStretchWindow = true;
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

    event->accept();
}

/**
* @brief
* Move the window according to the mouse positions
*/
void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
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

    if(m_canMoveWindow){
        int dx = event->globalX() - m_mousePressX;
        int dy = event->globalY() - m_mousePressY;
        move (dx, dy);

    }else if(m_canStretchWindow && !isMaximized() && !isFullScreen()){
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
    }
    event->accept();
}

/**
* @brief
  * Initialize flags
 */
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_canMoveWindow = false;
    m_canStretchWindow = false;
    event->accept();
}
#else
/**
* @brief
* Set variables to the position of the window when the mouse is pressed
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

/**
* @brief
* Move the window according to the mouse positions
*/
void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if(m_canMoveWindow){
        this->setCursor(Qt::ClosedHandCursor);
        int dx = event->globalX() - m_mousePressX;
        int dy = event->globalY() - m_mousePressY;
        move (dx, dy);

    }
}

/**
* @brief
  * Initialize flags
 */
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_canMoveWindow = false;
    this->unsetCursor();
    event->accept();
}
#endif

/**
 * @brief MainWindow::moveNoteToTop : moves the current note Widget
 * to the top of the layout
 */
void MainWindow::moveNoteToTop()
{
    // check if the current note is note on the top of the list
    // if true move the note to the top
    if(m_currentSelectedNoteProxy.isValid()
            && m_noteView->currentIndex().row() != 0){

        m_noteView->scrollToTop();

        // move the current selected note to the top
        QModelIndex sourceIndex = m_proxyModel->mapToSource(m_currentSelectedNoteProxy);
        QModelIndex destinationIndex = m_noteModel->index(0);
        m_noteModel->moveRow(sourceIndex, sourceIndex.row(), destinationIndex, 0);

        // update the current item
        m_currentSelectedNoteProxy = m_proxyModel->mapFromSource(destinationIndex);
        m_noteView->setCurrentIndex(m_currentSelectedNoteProxy);
    }
}

void MainWindow::clearSearch()
{
    m_noteView->setFocusPolicy(Qt::StrongFocus);

    m_lineEdit->blockSignals(true);
    m_lineEdit->clear();
    m_lineEdit->blockSignals(false);

    m_textEdit->blockSignals(true);
    m_textEdit->clear();
    m_textEdit->clearFocus();
    m_editorDateLabel->clear();
    m_textEdit->blockSignals(false);

    m_proxyModel->setFilterFixedString(QStringLiteral(""));

    m_clearButton->hide();
    m_lineEdit->setFocus();

    m_noteView->setSearching(false);
}

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

void MainWindow::checkMigration()
{
    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());

    QString oldNoteDBPath(dir.path() + "/Notes.ini");
    if(QFile::exists(oldNoteDBPath))
        migrateNote(oldNoteDBPath);

    QString oldTrashDBPath(dir.path() + "/Trash.ini");
    if(QFile::exists(oldTrashDBPath))
        migrateTrash(oldTrashDBPath);
}

void MainWindow::migrateNote(QString notePath)
{
    QSettings notesIni(QSettings::IniFormat, QSettings::UserScope, "Awesomeness", "Notes");
    QStringList dbKeys = notesIni.allKeys();

    auto it = dbKeys.begin();
    for(; it < dbKeys.end()-1; it += 3){
        QString noteName = it->split("/")[0];

        NoteData* newNote = new NoteData();
        newNote->setId(noteName);

        QString cntStr = notesIni.value("notesCounter", "NULL").toString();
        if(cntStr == "NULL"){
            m_noteCounter = 0;
        }else{
            m_noteCounter = cntStr.toInt();
        }

        QString createdDateDB = notesIni.value(noteName + "/dateCreated", "Error").toString();
        newNote->setCreationDateTime(QDateTime::fromString(createdDateDB, Qt::ISODate));
        QString lastEditedDateDB = notesIni.value(noteName + "/dateEdited", "Error").toString();
        newNote->setLastModificationDateTime(QDateTime::fromString(lastEditedDateDB, Qt::ISODate));
        QString contentText = notesIni.value(noteName + "/content", "Error").toString();
        newNote->setContent(contentText);
        QString firstLine = getFirstLine(contentText);
        newNote->setFullTitle(firstLine);

        m_dbManager->migrateNote(newNote);
        delete newNote;
    }

    QFile oldNoteDBFile(notePath);
    oldNoteDBFile.rename(QFileInfo(notePath).dir().path() + "/oldNotes.ini");
}

void MainWindow::migrateTrash(QString trashPath)
{
    QSettings trashIni(QSettings::IniFormat, QSettings::UserScope, "Awesomeness", "Trash");

    QStringList dbKeys = trashIni.allKeys();

    auto it = dbKeys.begin();
    for(; it < dbKeys.end()-1; it += 3){
        QString noteName = it->split("/")[0];

        NoteData* newNote = new NoteData();
        newNote->setId(noteName);

        QString createdDateDB = trashIni.value(noteName + "/dateCreated", "Error").toString();
        newNote->setCreationDateTime(QDateTime::fromString(createdDateDB, Qt::ISODate));
        QString lastEditedDateDB = trashIni.value(noteName + "/dateEdited", "Error").toString();
        newNote->setLastModificationDateTime(QDateTime::fromString(lastEditedDateDB, Qt::ISODate));
        QString contentText = trashIni.value(noteName + "/content", "Error").toString();
        newNote->setContent(contentText);
        QString firstLine = getFirstLine(contentText);
        newNote->setFullTitle(firstLine);

        m_dbManager->migrateTrash(newNote);
        delete newNote;
    }

    QFile oldTrashDBFile(trashPath);
    oldTrashDBFile.rename(QFileInfo(trashPath).dir().path() + "/oldTrash.ini");
}

/**
* @brief
* When the blank area at the top of window is double-clicked the window get maximized
*/
void MainWindow::mouseDoubleClickEvent (QMouseEvent *event)
{
    maximizeWindow();
    event->accept();
}

void MainWindow::leaveEvent(QEvent *)
{
    this->unsetCursor();
}

/**
* @brief
* Mostly take care on the event happened on widget whose filter installed to tht mainwindow
*/
bool MainWindow::eventFilter (QObject *object, QEvent *event)
{
    switch (event->type()){
    case QEvent::Enter:{
        if(qApp->applicationState() == Qt::ApplicationActive){
#ifdef _WIN32
            if(object == m_redCloseButton){
                m_redCloseButton->setIcon(QIcon(":images/windows_close_hovered.png"));
            }

            if(object == m_yellowMinimizeButton){
                if(this->windowState() == Qt::WindowFullScreen){
                    m_yellowMinimizeButton->setIcon(QIcon(":images/windows_de-maximize_hovered.png"));
                }else{
                    m_yellowMinimizeButton->setIcon(QIcon (":images/windows_maximize_hovered.png"));
                }
            }

            if(object == m_greenMaximizeButton){
                m_greenMaximizeButton->setIcon(QIcon(":images/windows_minimize_hovered.png"));
            }
#else
            // When hovering one of the traffic light buttons (red, yellow, green),
            // set new icons to show their function
            if(object == m_redCloseButton
                    || object == m_yellowMinimizeButton
                    || object == m_greenMaximizeButton){

                m_redCloseButton->setIcon(QIcon(":images/redHovered.png"));
                m_yellowMinimizeButton->setIcon(QIcon(":images/yellowHovered.png"));
                if(this->windowState() == Qt::WindowFullScreen){
                    m_greenMaximizeButton->setIcon(QIcon(":images/greenInHovered.png"));
                }else{
                    m_greenMaximizeButton->setIcon(QIcon(":images/greenHovered.png"));
                }
            }
#endif

            if(object == m_newNoteButton){
                this->setCursor(Qt::PointingHandCursor);
                m_newNoteButton->setIcon(QIcon(":/images/newNote_Hovered.png"));
            }

            if(object == m_trashButton){
                this->setCursor(Qt::PointingHandCursor);
                m_trashButton->setIcon(QIcon(":/images/trashCan_Hovered.png"));
            }

            if(object == m_dotsButton){
                this->setCursor(Qt::PointingHandCursor);
                m_dotsButton->setIcon(QIcon(":/images/3dots_Hovered.png"));
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
                m_redCloseButton->setIcon(QIcon(":images/windows_close_regular.png"));
                m_greenMaximizeButton->setIcon(QIcon(":images/windows_minimize_regular.png"));

                if(this->windowState() == Qt::WindowFullScreen){
                    m_yellowMinimizeButton->setIcon(QIcon(":images/windows_de-maximize_regular.png"));
                }else{
                    m_yellowMinimizeButton->setIcon(QIcon (":images/windows_maximize_regular.png"));
                }
#else
                m_redCloseButton->setIcon(QIcon(":images/red.png"));
                m_yellowMinimizeButton->setIcon(QIcon(":images/yellow.png"));
                m_greenMaximizeButton->setIcon(QIcon(":images/green.png"));
#endif
            }

            if(object == m_newNoteButton){
                this->unsetCursor();
                m_newNoteButton->setIcon(QIcon(":/images/newNote_Regular.png"));
            }

            if(object == m_trashButton){
                this->unsetCursor();
                m_trashButton->setIcon(QIcon(":/images/trashCan_Regular.png"));
            }

            if(object == m_dotsButton){
                this->unsetCursor();
                m_dotsButton->setIcon(QIcon(":/images/3dots_Regular.png"));
            }
        }
        break;
    }
    case QEvent::WindowDeactivate:{
#ifndef _WIN32
        m_redCloseButton->setIcon(QIcon(":images/unfocusedButton"));
        m_yellowMinimizeButton->setIcon(QIcon(":images/unfocusedButton"));
        m_greenMaximizeButton->setIcon(QIcon(":images/unfocusedButton"));
#endif
        m_newNoteButton->setIcon(QIcon(":/images/newNote_Regular.png"));
        m_trashButton->setIcon(QIcon(":/images/trashCan_Regular.png"));
        m_dotsButton->setIcon(QIcon(":/images/3dots_Regular.png"));
        break;
    }
    case QEvent::WindowActivate:{
#ifdef _WIN32
        m_redCloseButton->setIcon(QIcon(":images/windows_close_regular.png"));
        m_greenMaximizeButton->setIcon(QIcon(":images/windows_minimize_regular.png"));

        if(this->windowState() == Qt::WindowFullScreen){
            m_yellowMinimizeButton->setIcon(QIcon(":images/windows_de-maximize_regular.png"));
        }else{
            m_yellowMinimizeButton->setIcon(QIcon (":images/windows_maximize_regular.png"));
        }
#else
        m_redCloseButton->setIcon(QIcon(":images/red.png"));
        m_yellowMinimizeButton->setIcon(QIcon(":images/yellow.png"));
        m_greenMaximizeButton->setIcon(QIcon(":images/green.png"));
#endif
        m_newNoteButton->setIcon(QIcon(":/images/newNote_Regular.png"));
        m_trashButton->setIcon(QIcon(":/images/trashCan_Regular.png"));
        m_dotsButton->setIcon(QIcon(":/images/3dots_Regular.png"));
        break;
    }
    case QEvent::HoverEnter:{
        if(object == m_textEdit->verticalScrollBar()){
            bool isSearching = !m_lineEdit->text().isEmpty();
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
    case QEvent::FocusIn:{
        if(object == m_textEdit){

            m_noteView->setCurrentRowActive(true);

            if(!m_isOperationRunning){
                // When clicking in a note's content while searching,
                // reload all the notes and go and select that note
                if(!m_lineEdit->text().isEmpty()){
                    m_selectedNoteBeforeSearchingInSource = QModelIndex();

                    if(m_currentSelectedNoteProxy.isValid()){
                        QModelIndex indexInSource = m_proxyModel->mapToSource(m_currentSelectedNoteProxy);
                        clearSearch();
                        m_currentSelectedNoteProxy = m_proxyModel->mapFromSource(indexInSource);
                        selectNote(m_currentSelectedNoteProxy);

                    }else{
                        clearSearch();
                        createNewNote();
                    }

                    m_textEdit->setFocus();

                }else if(m_proxyModel->rowCount() == 0){
                    createNewNote();
                }
            }
        }

        if(object == m_lineEdit){
            int frameWidth = m_lineEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
            QString ss = QString("QLineEdit{ "
                                 "  padding-right: %1px; "
                                 "  padding-left: 21px;"
                                 "  padding-right: 19px;"
                                 "  border: 1px solid rgb(61, 155, 218);"
                                 "  border-radius: 3px;"
                                 "  background: rgb(255, 255, 255);"
                                 "  selection-background-color: rgb(61, 155, 218);"
                                 "} "
                                 "QToolButton { "
                                 "  border: none; "
                                 "  padding: 0px;"
                                 "}"
                                 ).arg(frameWidth + 1);

            m_lineEdit->setStyleSheet(ss);
        }
        break;
    }
    case QEvent::FocusOut:{
        if(object == m_textEdit){
            m_noteView->setCurrentRowActive(false);
        }

        if(object == m_lineEdit){
            int frameWidth = m_lineEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
            QString ss = QString("QLineEdit{ "
                                 "  padding-right: %1px; "
                                 "  padding-left: 21px;"
                                 "  padding-right: 19px;"
                                 "  border: 1px solid rgb(205, 205, 205);"
                                 "  border-radius: 3px;"
                                 "  background: rgb(255, 255, 255);"
                                 "  selection-background-color: rgb(61, 155, 218);"
                                 "} "
                                 "QToolButton { "
                                 "  border: none; "
                                 "  padding: 0px;"
                                 "}"
                                 ).arg(frameWidth + 1);

            m_lineEdit->setStyleSheet(ss);
        }
        break;
    }
    default:
        break;
    }

    return QObject::eventFilter(object, event);
}

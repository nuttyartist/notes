/**************************************************************************************
* We believe in the power of notes to help us record ideas and thoughts.
* We want people to have an easy, beautiful and simple way of doing that.
* And so we have Notes.
***************************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include <QShortcut>
#include <QTextStream>
#include <QScrollArea>
#include "notewidgetdelegate.h"
#include "qxtglobalshortcut.h"
#define FIRST_LINE_MAX 80

/**
* Setting up the main window and it's content
*/
MainWindow::MainWindow (QWidget *parent) :
    QMainWindow (parent),
    ui (new Ui::MainWindow),
    m_autoSaveTimer(new QTimer(this)),
    m_notesDatabase(Q_NULLPTR),
    m_trashDatabase(Q_NULLPTR),
    m_settingsDatabase(Q_NULLPTR),
    m_noteWidgetsContainer(Q_NULLPTR),
    m_clearButton(Q_NULLPTR),
    m_greenMaximizeButton(Q_NULLPTR),
    m_redCloseButton(Q_NULLPTR),
    m_yellowMinimizeButton(Q_NULLPTR),
    m_newNoteButton(Q_NULLPTR),
    m_trashButton(Q_NULLPTR),
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
    m_noteCounter(0),
    m_trashCounter(0),
    m_canMoveWindow(false),
    m_isTemp(false),
    m_isListViewScrollBarHidden(true),
    m_isContentModified(false),
    m_isOperationRunning(false)
{
    ui->setupUi(this);
    setupMainWindow();
    setupTrayIcon();
    setupKeyboardShortcuts();
    setupNewNoteButtonAndTrahButton();
    setupEditorDateLabel();
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

    QTimer::singleShot(200,this, SLOT(InitData()));
}

/**
 * @brief Init the data from database and select the first note if there is one
 */
void MainWindow::InitData()
{
    loadNotes();
    createNewNoteIfEmpty();
    selectFirstNote();
}

void MainWindow::setMainWindowVisibility(bool state)
{
    if(state){
        m_restoreAction->setText(tr("&Hide Notes"));
        show();
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
#elif _WIN32
    this->setWindowFlags(Qt::CustomizeWindowHint);
#elif __APPLE__
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
#else
#error "We don't support that version yet..."
#endif

    m_greenMaximizeButton = ui->greenMaximizeButton;
    m_redCloseButton = ui->redCloseButton;
    m_yellowMinimizeButton = ui->yellowMinimizeButton;
    m_newNoteButton = ui->newNoteButton;
    m_trashButton = ui->trashButton;
    m_lineEdit = ui->lineEdit;
    m_textEdit = ui->textEdit;
    m_editorDateLabel = ui->editorDateLabel;
    m_splitter = ui->splitter;

    QPalette pal(palette());
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    m_newNoteButton->setToolTip("Create New Note");
    m_trashButton->setToolTip("Delete Selected Note");
}

void MainWindow::setupTrayIcon()
{
    m_trayIconMenu->addAction(m_restoreAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_quitAction);

    QIcon icon(":images/notes_icon.png");
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
        this->setMainWindowVisibility(isHidden());
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
#ifdef __APPLE__
    m_newNoteButton->setMinimumSize(QSize(50, 32));
    m_trashButton->setMinimumSize(QSize(50, 32));
#endif
}
/**
* @brief
* This is what happens when you build cross-platform apps,
* some problems just occures that needs specific and special care.
* When we face these kind of problems it helps to remember that when
* we put the effort to linger on these tiny bits and bytes it creates
* a flawless and a delightful experience for are users,
* and that's what matters the most.
*/
void MainWindow::setupEditorDateLabel()
{
    // There is a problem with Helvetica here so we usa Arial, someone sguggestion?
#ifdef __APPLE__
    QFont editorDateLabelFont(QFont("Arial", 12));
    editorDateLabelFont.setBold(true);
    m_editorDateLabel->setFont(editorDateLabelFont);
#endif
}

/**
* @brief
* Set up the splitter that control the size of the scrollArea and the textEdit
*/
void MainWindow::setupSplitter()
{
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setStretchFactor(2, 0);
}

/**
* @brief
* Set up the vertical line that seperate between the scrollArea to the textEdit
*/
void MainWindow::setupLine ()
{
    ui->line->setStyleSheet("border: 1px solid rgb(221, 221, 221)");
}

/**
* @brief
* Set up a frame above textEdit and behind the other widgets for a unifed background in thet editor section
*/
void MainWindow::setupRightFrame ()
{
    QString ss = "QFrame{ "
                 "  background-image: url(:images/textSideBackground.png); "
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

    m_redCloseButton->installEventFilter(this);
    m_yellowMinimizeButton->installEventFilter(this);
    m_greenMaximizeButton->installEventFilter(this);
}

/**
 * @brief connect between signals and slots
 */
void MainWindow::setupSignalsSlots()
{
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
    connect(m_newNoteButton, &QPushButton::clicked, this, &MainWindow::onNewNoteButtonClicked);
    // delete note button
    connect(m_trashButton, &QPushButton::clicked, this, &MainWindow::onTrashButtonClicked);
    connect(m_noteModel, &NoteModel::rowsRemoved, [this](){m_trashButton->setEnabled(true);});
    // text edit text changed
    connect(m_textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextEditTextChanged);
    // line edit text changed
    connect(m_lineEdit, &QLineEdit::textChanged, this, &MainWindow::onLineEditTextChanged);
    // note pressed
    connect(m_noteView, &NoteView::pressed, this, &MainWindow::onNotePressed);
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
        setMainWindowVisibility(isHidden());
    });
    // Quit Action
    connect(m_quitAction, &QAction::triggered, this, &MainWindow::QuitApplication);

}

/**
* @brief
* Set the lineedit to start a bit to the right and end a bit to the left (pedding)
*/
void MainWindow::setupLineEdit ()
{
    // There is a problem with Helvetica here so we usa Arial, someone sguggestion?
#ifdef __APPLE__
    m_lineEdit->setFont(QFont("Arial", 12));
#endif

    QLineEdit* lineEdit = m_lineEdit;

    int frameWidth = m_lineEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    QString ss = QString("QLineEdit{ "
                         "  padding-right: %1px; "
                         "  padding-left: 20px;"
                         "  padding-right: 19px;"
                         "} "
                         "QToolButton { "
                         "  border: none; "
                         "  padding: 0px;"
                         "}"
                         ).arg(frameWidth + 1);

    lineEdit->setStyleSheet(ss);

    // clear button
    m_clearButton = new QToolButton(lineEdit);
    QPixmap pixmap(":images/closeButton.gif");
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

#ifdef Q_OS_LINUX
    m_textEditLeftPadding = 5;
#elif _WIN32
    m_textEditLeftPadding = 5;
#elif __APPLE__
    m_textEditLeftPadding = 22;
#else
#error "We don't support that version yet..."
#endif

    QString ss = QString("QTextEdit {background-image: url(:images/textSideBackground.png); padding-left: %1px; padding-right: %2px; padding-bottom:2px;} "
                         "QScrollBar::handle:vertical:hover { background: rgb(170, 170, 171); } "
                         "QScrollBar::handle:vertical:pressed { background: rgb(149, 149, 149); } "
                         "QScrollBar::handle:vertical { border-radius: 4px; background: rgb(188, 188, 188); min-height: 20px; }  "
                         "QScrollBar::vertical {border-radius: 4px; width: 8px; color: rgba(255, 255, 255,0);} "
                         "QScrollBar {margin: 0; background: transparent;} "
                         "QScrollBar:hover { background-color: rgb(217, 217, 217);}"
                         "QScrollBar::add-line:vertical { width:0px; height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  "
                         "QScrollBar::sub-line:vertical { width:0px; height: 0px; subcontrol-position: top; subcontrol-origin: margin; }"
                         ).arg(QString::number(m_newNoteButton->width() - m_textEditLeftPadding), "27");

    m_textEdit->setStyleSheet(ss);

    m_textEdit->installEventFilter(this);
    m_textEdit->verticalScrollBar()->installEventFilter(this);

#ifdef Q_OS_LINUX
    m_textEdit->setFont(QFont("Liberation Sans", 11));
#elif _WIN32
    m_textEdit->setFont(QFont("Arial", 11));
#elif __APPLE__
    m_textEdit->setFont(QFont("Helvetica", 15));
#else
#error "We don't support that version yet..."
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
    m_notesDatabase = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Awesomeness", "Notes", this);
    m_notesDatabase->setFallbacksEnabled(false);

    QString cntStr = m_notesDatabase->value("notesCounter", "NULL").toString();
    if(cntStr == "NULL"){
        m_notesDatabase->setValue("notesCounter", 0);
    }else{
        m_noteCounter = cntStr.toInt();
    }

    m_trashDatabase = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Awesomeness", "Trash", this);
    m_trashDatabase->setFallbacksEnabled(false);

    QString trashCntrStr = m_trashDatabase->value("notesCounter", "NULL").toString();
    if(trashCntrStr == "NULL"){
        m_trashDatabase->setValue("notesCounter", 0);
    }else{
        m_trashCounter = trashCntrStr.toInt();
    }

    m_settingsDatabase = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Awesomeness", "Settings", this);
    m_settingsDatabase->setFallbacksEnabled(false);

    initializeSettingsDatabase();

    m_notesDatabase->sync();
    m_trashDatabase->sync();
    m_settingsDatabase->sync();
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
NoteData *MainWindow::generateNote(QString noteName, bool isLoadingOrNew)
{
    NoteData* newNote = new NoteData(this);
    newNote->setId(noteName);

    if(isLoadingOrNew){
        // created date time
        QString createdDateDB = m_notesDatabase->value(noteName + "/dateCreated", "Error").toString();
        newNote->setCreationDateTime(QDateTime::fromString(createdDateDB, Qt::ISODate));
        // last edited date time
        QString lastEditedDateDB = m_notesDatabase->value(noteName + "/dateEdited", "Error").toString();
        newNote->setLastModificationDateTime(QDateTime::fromString(lastEditedDateDB, Qt::ISODate));
        // content
        QString contentText = m_notesDatabase->value(noteName + "/content", "Error").toString();
        newNote->setContent(contentText);
        // full title
        QString firstLine = getFirstLine(contentText);
        newNote->setFullTitle(firstLine);

    }else{
        QDateTime noteDate = QDateTime::currentDateTime();
        newNote->setCreationDateTime(noteDate);
        newNote->setLastModificationDateTime(noteDate);
        newNote->setFullTitle(QStringLiteral("New Note"));
    }

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
    QStringList dbKeys = m_notesDatabase->allKeys();
    QList<NoteData*> noteList;
    auto it = dbKeys.begin();
    for(; it < dbKeys.end()-1; it += 3){
        QString noteName = it->split("/")[0];
        NoteData* newNote = generateNote(noteName, true);
        noteList << newNote;
    }

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

        QModelIndex proxyIndex = m_proxyModel->index(noteIndex.row(),0);
        QModelIndex index = m_proxyModel->mapToSource(proxyIndex);

        QString id = index.data(NoteModel::NoteID).toString();
        QString content = index.data(NoteModel::NoteContent).toString();
        QDateTime createdDateTime = index.data(NoteModel::NoteCreationDateTime).toDateTime();
        QString createdNoteDateStr = createdDateTime.toString(Qt::ISODate);
        QDateTime editedDateTime = index.data(NoteModel::NoteLastModificationDateTime).toDateTime();
        QString editedNoteDateStr = editedDateTime.toString(Qt::ISODate);

        m_notesDatabase->setValue("notesCounter", m_noteCounter);
        m_notesDatabase->setValue(id + "/content", content);
        m_notesDatabase->setValue(id + "/dateCreated", createdNoteDateStr);
        m_notesDatabase->setValue(id + "/dateEdited", editedNoteDateStr);

        m_isContentModified = false;
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

/**
* @brief
* Create a new note when clicking the 'new note' button
*/
void MainWindow::onNewNoteButtonClicked()
{
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
* Delete selected note when clicking the 'delete note' button
*/
void MainWindow::onTrashButtonClicked()
{
    m_trashButton->blockSignals(true);
    this->deleteSelectedNote();
    m_trashButton->blockSignals(false);
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
* Delete a given note from the database and put it in the trash DB
*/
void MainWindow::deleteNoteFromDataBase (const QModelIndex &noteIndex)
{
    if(noteIndex.isValid()){
        QString id = noteIndex.data(NoteModel::NoteID).toString();
        QString content = noteIndex.data(NoteModel::NoteContent).toString();
        QDateTime createdDateTime = noteIndex.data(NoteModel::NoteCreationDateTime).toDateTime();
        QDateTime lastEditedDateTime = noteIndex.data(NoteModel::NoteLastModificationDateTime).toDateTime();
        QDateTime deletedNoteDateTime =  noteIndex.data(NoteModel::NoteDeletionDateTime).toDateTime();

        // Putting the deleted note in trash
        ++m_trashCounter;
        QString noteName = QString("noteID_%1").arg(m_trashCounter);
        QString dateDBCreated = createdDateTime.toString(Qt::ISODate);
        QString dateDBEdited = lastEditedDateTime.toString(Qt::ISODate);
        QString dateNoteDeleted = deletedNoteDateTime.toString(Qt::ISODate);

        m_trashDatabase->setValue("notesCounter", m_trashCounter);
        m_trashDatabase->setValue(noteName + "/content",  content);
        m_trashDatabase->setValue(noteName + "/dateCreated", dateDBCreated);
        m_trashDatabase->setValue(noteName + "/dateEdited", dateDBEdited);
        m_trashDatabase->setValue(noteName + "/dateTrashed", dateNoteDeleted);

        m_notesDatabase->remove(id);

    }else{
        qDebug() << "MainWindow::deleteNoteFromDataBase() : noteIndex is not valid";
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
            NoteData* tmpNote = generateNote(noteID, false);
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
 * @brief MainWindow::deleteNote delete the specified note
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
            m_deletedNotesModel->addNote(noteTobeRemoved);
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
    if(this->windowState() == Qt::WindowFullScreen){
        this->setWindowState(Qt::WindowNoState);
    }else{
        this->setWindowState(Qt::WindowFullScreen);
    }
}

/**
* @brief
* Maximize the window
*/
void MainWindow::maximizeWindow ()
{
    if(this->windowState() == Qt::WindowMaximized || this->windowState() == Qt::WindowFullScreen){
        this->setWindowState(Qt::WindowNoState);
    }else{
        this->setWindowState(Qt::WindowMaximized);
    }
}

/**
* @brief
* Minimize the window
*/
void MainWindow::minimizeWindow ()
{
    this->setWindowState(Qt::WindowMinimized);
}

/**
* @brief
* Exit the application
* If a new note created but wasn't edited, delete it from the database
*/
void MainWindow::QuitApplication ()
{
    //QApplication::quit();
    for (int i=0; i<m_deletedNotesModel->rowCount(); i++) {
        QModelIndex index = m_deletedNotesModel->index(i);
        deleteNoteFromDataBase(index);
    }

    MainWindow::close();
}

/**
* @brief
* When the green button is pressed set it's icon accordingly
*/
void MainWindow::onGreenMaximizeButtonPressed ()
{
    if(this->windowState() == Qt::WindowFullScreen){
        m_greenMaximizeButton->setIcon(QIcon(":images/greenInPressed.png"));
    }else{
        m_greenMaximizeButton->setIcon(QIcon(":images/greenPressed.png"));
    }
}

/**
* @brief
* When the yellow button is pressed set it's icon accordingly
*/
void MainWindow::onYellowMinimizeButtonPressed ()
{
    m_yellowMinimizeButton->setIcon(QIcon(":images/yellowPressed.png"));
}

/**
* @brief
* When the red button is pressed set it's icon accordingly
*/
void MainWindow::onRedCloseButtonPressed ()
{
    m_redCloseButton->setIcon(QIcon(":images/redPressed.png"));
}

/**
* @brief
* When the green button is released the window goes fullscrren
*/
void MainWindow::onGreenMaximizeButtonClicked()
{
    m_greenMaximizeButton->setIcon(QIcon(":images/green.png"));

    fullscreenWindow();
}

/**
* @brief
* When yellow button is released the window is minimized
*/
void MainWindow::onYellowMinimizeButtonClicked()
{
    m_yellowMinimizeButton->setIcon(QIcon(":images/yellow.png"));

    minimizeWindow();
}

/**
* @brief
* When red button is released the window get closed
* If a new note created but wasn't edited, delete it from the database
*/
void MainWindow::onRedCloseButtonClicked()
{
    m_redCloseButton->setIcon(QIcon(":images/red.png"));

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

    if(m_isTemp){
        QString id = m_currentSelectedNoteProxy.data(NoteModel::NoteID).toString();
        m_notesDatabase->remove(id);
    }

    m_settingsDatabase->setValue("splitterSizes", m_splitter->saveState());

    m_settingsDatabase->sync();
    m_notesDatabase->sync();

    QWidget::closeEvent(event);
}

/**
* @brief
* Set variables to the position of the window when the mouse is pressed
*/
void MainWindow::mousePressEvent (QMouseEvent* event)
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
void MainWindow::mouseMoveEvent (QMouseEvent* event)
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
void MainWindow::mouseReleaseEvent (QMouseEvent *event)
{
    m_canMoveWindow = false;
    this->unsetCursor();
    event->accept();
}

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
        // show the content of the pressed note in the text editor
        showNoteInEditor(noteIndex);

        // save the position of text edit scrollbar
        if(!m_isTemp && m_currentSelectedNoteProxy.isValid()){
            int pos = m_textEdit->verticalScrollBar()->value();
            QModelIndex indexSrc = m_proxyModel->mapToSource(noteIndex);
            m_noteModel->setData(indexSrc, QVariant::fromValue(pos), NoteModel::NoteScrollbarPos);
        }

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
        }
        break;
    }
    case QEvent::Leave:{
        if(qApp->applicationState() == Qt::ApplicationActive){
            // When not hovering, change back the icons of the traffic lights to their default icon
            if(object == m_redCloseButton
                    || object == m_yellowMinimizeButton
                    || object == m_greenMaximizeButton){

                m_redCloseButton->setIcon(QIcon(":images/red.png"));
                m_yellowMinimizeButton->setIcon(QIcon(":images/yellow.png"));
                m_greenMaximizeButton->setIcon(QIcon(":images/green.png"));
            }
        }
        break;
    }
    case QEvent::WindowDeactivate:{
        m_redCloseButton->setIcon(QIcon(":images/unfocusedButton"));
        m_yellowMinimizeButton->setIcon(QIcon(":images/unfocusedButton"));
        m_greenMaximizeButton->setIcon(QIcon(":images/unfocusedButton"));
        break;
    }
    case QEvent::WindowActivate:{
        m_redCloseButton->setIcon(QIcon(":images/red.png"));
        m_yellowMinimizeButton->setIcon(QIcon(":images/yellow.png"));
        m_greenMaximizeButton->setIcon(QIcon(":images/green.png"));
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
        break;
    }
    case QEvent::FocusOut:{
        if(object == m_textEdit){
            m_noteView->setCurrentRowActive(false);
        }
        break;
    }
    default:
        break;
    }

    return QObject::eventFilter(object, event);
}

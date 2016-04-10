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

#define FIRST_LINE_MAX 80

/**
* Setting up the main window and it's content
*/
MainWindow::MainWindow (QWidget *parent) :
    QMainWindow (parent),
    ui (new Ui::MainWindow),
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
    m_noteModel(new NoteModel(this)),
    m_proxyModel(new QSortFilterProxyModel(this)),
    m_canBeResized(false),
    m_resizeHorzTop(false),
    m_resizeHorzBottom(false),
    m_resizeVertRight(false),
    m_resizeVertLeft(false),
    m_canMoveWindow(false),
    m_focusBreaker(false),
    m_isTemp(false),
    m_isListViewScrollBarHidden(true)
{
    ui->setupUi(this);
    setupMainWindow();
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


    QPalette pal(palette());
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    m_newNoteButton->setToolTip("Create New Note");
    m_trashButton->setToolTip("Delete Selected Note");
}

/**
* @brief
* Setting up the keyboard shortcuts
*/
void MainWindow::setupKeyboardShortcuts ()
{
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_N), this, SLOT(createNewNote()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Delete), this, SLOT(deleteSelectedNote()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), m_lineEdit, SLOT(setFocus()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_E), m_lineEdit, SLOT(clear()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this, SLOT(setFocusOnCurrentNote()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down), this, SLOT(selectNoteDown()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Up), this, SLOT(selectNoteUp()));
    new QShortcut(QKeySequence(Qt::Key_Down), this, SLOT(selectNoteDown()));
    new QShortcut(QKeySequence(Qt::Key_Up), this, SLOT(selectNoteUp()));
    new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(setFocusOnText()));
    new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(setFocusOnText()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F), this, SLOT(fullscreenWindow()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L), this, SLOT(maximizeWindow()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_M), this, SLOT(minimizeWindow()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(QuitApplication()));
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
    m_newNoteButton->setGeometry(m_newNoteButton->x(), m_newNoteButton->y(), 50, 32);
    m_newNoteButton->setIconSize(QSize(16, 16));
    m_trashButton->setGeometry(676, m_trashButton->y(), 50, 32);
    m_trashButton->setIconSize(QSize(14, 18));
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
    ui->editorDateLabel->setFont(editorDateLabelFont);
    ui->editorDateLabel->setGeometry(ui->editorDateLabel->x(), ui->editorDateLabel->y() + 4, ui->editorDateLabel->width(), ui->editorDateLabel->height());
#endif
}

/**
* @brief
* Set up the splitter that control the size of the scrollArea and the textEdit
*/
void MainWindow::setupSplitter()
{
    ui->splitter->setStretchFactor(1, 1);
    ui->splitter->setStretchFactor(2, 0);
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

    connect(m_clearButton, &QToolButton::clicked, this, [&, lineEdit](){
        m_clearButton->hide();
        lineEdit->clear();
        ui->textEdit->setFocus();
    });

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
    textEditLeftPadding = 5;
#elif __APPLE__
    textEditLeftPadding = 18;
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

    ui->textEdit->setStyleSheet(ss);

    ui->textEdit->installEventFilter(this);

#ifdef Q_OS_LINUX
    ui->textEdit->setFont(QFont("Liberation Sans", 11));
#elif _WIN32
    ui->textEdit->setFont(QFont("Arial", 11));
#elif __APPLE__
    ui->textEdit->setFont(QFont("Helvetica", 15));
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
        m_settingsDatabase->setValue("splitterSizes", ui->splitter->saveState());
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

    if(m_notesDatabase->value("notesCounter", "NULL") == "NULL")
        m_notesDatabase->setValue("notesCounter", 0);

    m_trashDatabase = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Awesomeness", "Trash", this);
    m_trashDatabase->setFallbacksEnabled(false);

    if(m_trashDatabase->value("notesCounter", "NULL") == "NULL")
        m_trashDatabase->setValue("notesCounter", 0);

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

    ui->splitter->restoreState(m_settingsDatabase->value("splitterSizes").toByteArray());

    // If scrollArea is collapsed
    if(ui->splitter->sizes().at(0) == 0){
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

    QString dateDB = m_notesDatabase->value(noteName + "/dateEdited", "Error").toString();
    newNote->setDateTime(QDateTime::fromString(dateDB, Qt::ISODate));
    QString contentText = m_notesDatabase->value(noteName + "/content", "Error").toString();
    newNote->setContent(contentText);
    QString firstLine = isLoadingOrNew ? getFirstLine(contentText) : "New Note";
    newNote->setFullTitle(firstLine);

    return newNote;
}

/**
 * @brief show the specified note content text in the text editor
 * Set editorDateLabel text to the the selected note date
 * And restore the scrollBar position if it changed before.
 */
void MainWindow::showNoteInEditor(const QModelIndex &noteIndex)
{
    ui->textEdit->blockSignals(true);
    QString content = noteIndex.data(NoteModel::NoteContent).toString();
    QDateTime dateTime = noteIndex.data(NoteModel::NoteDateTime).toDateTime();
    int scrollbarPos = noteIndex.data(NoteModel::NoteScrollbarPos).toInt();

    // set text and date
    ui->textEdit->setText(content);
    QString noteDate = dateTime.toString(Qt::ISODate);
    QString noteDateEditor = getNoteDateEditor(noteDate);
    ui->editorDateLabel->setText(noteDateEditor);
    // set scrollbar position
    ui->textEdit->verticalScrollBar()->setValue(scrollbarPos);
    ui->textEdit->blockSignals(false);
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
    m_noteModel->addListNote(noteList);

    m_noteModel->sort(0,Qt::AscendingOrder);
}

/**
* @brief
* save the current note to database
*/
void MainWindow::saveNoteToDB(const QModelIndex &noteIndex)
{
    if(noteIndex.row() != -1){

        QModelIndex proxyIndex = m_proxyModel->index(noteIndex.row(),0);
        QModelIndex index = m_proxyModel->mapToSource(proxyIndex);

        QString id = index.data(NoteModel::NoteID).toString();
        QString content = index.data(NoteModel::NoteContent).toString();
        QDateTime dateTime = index.data(NoteModel::NoteDateTime).toDateTime();
        QString noteDateStr = dateTime.toString(Qt::ISODate);

        m_notesDatabase->setValue(id + "/content", content);
        m_notesDatabase->setValue(id + "/dateEdited", noteDateStr);

        m_notesDatabase->sync();

        // TODO : find a way to mark a note was modified
        // noteIndex->setModified(false);
    }
}

/**
* @brief
* Select the first note in the notes list
*/
void MainWindow::selectFirstNote ()
{
    if(m_proxyModel->rowCount() > 0){
        QModelIndex index = m_proxyModel->mapToSource(m_proxyModel->index(0,0));
        QItemSelection itemSelection(index, index);
        m_noteView->selectionModel()->select(itemSelection,
                                             QItemSelectionModel::ClearAndSelect);
        m_noteView->setCurrentIndex(m_noteView->model()->index(0,0));

        m_currentSelectedNoteProxy = index;
        showNoteInEditor(m_currentSelectedNoteProxy);
    }
}

/**
* @brief
* create a new note if there are no notes
*/
void MainWindow::createNewNoteIfEmpty ()
{
    if(m_noteModel->rowCount() == 0)
        createNewNote();
}

/**
* @brief
* Create a new note when clicking the 'new note' button
*/
void MainWindow::onNewNoteButtonClicked()
{
    this->createNewNote();
}

/**
* @brief
* Delete selected note when clicking the 'delete note' button
*/
void MainWindow::onTrashButtonClicked()
{
    m_trashButton->setEnabled(false);
    this->deleteSelectedNote();
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
        m_noteView->setCurrentIndex(index);
        selectNote(index);
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
        QDateTime dateTime = noteIndex.data(NoteModel::NoteDateTime).toDateTime();

        // Putting the deleted note in trash
        int counter = m_trashDatabase->value("notesCounter").toInt() + 1;
        m_trashDatabase->setValue("notesCounter", counter);

        QString noteName = QString("noteID_%1").arg(counter);
        m_trashDatabase->setValue(noteName + "/content",  content);

        QString dateDBCreated = m_notesDatabase->value(id + "/dateCreated", "Error").toString();
        QString dateDBEdited = dateTime.toString(Qt::ISODate);
        QString dateNoteDeleted = QDateTime::currentDateTime().toString(Qt::ISODate);

        m_trashDatabase->setValue(noteName + "/dateCreated", dateDBCreated);
        m_trashDatabase->setValue(noteName + "/dateEdited", dateDBEdited);
        m_trashDatabase->setValue(noteName + "/dateTrashed", dateNoteDeleted);

        m_notesDatabase->remove(id);

        m_notesDatabase->sync();
        m_trashDatabase->sync();

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
        if(ui->textEdit->toPlainText() != content){

            // move note to the top of the list
            moveNoteToTop();

            // Get the new data
            QString firstline = getFirstLine(ui->textEdit->toPlainText());
            QDateTime dateTime = QDateTime::currentDateTime();
            QString noteDate = dateTime.toString(Qt::ISODate);
            ui->editorDateLabel->setText(getNoteDateEditor(noteDate));

            // update model
            QMap<int, QVariant> dataValue;
            dataValue[NoteModel::NoteContent] = QVariant::fromValue(ui->textEdit->toPlainText());
            dataValue[NoteModel::NoteFullTitle] = QVariant::fromValue(firstline);
            dataValue[NoteModel::NoteDateTime] = QVariant::fromValue(dateTime);

            QModelIndex index = m_noteModel->index(0,0);
            m_noteModel->setItemData(index, dataValue);

            // update modification flag
            // TODO : find a way to set modify flag
            //m_currentSelectedNote->setModified(true);
        }

        m_textEdit->blockSignals(false);

        m_isTemp = false;
    }else{
        qDebug() << "MainWindow::onTextEditTextChanged() : m_currentSelectedNoteProxy is not valid";
    }
}

/**
* @brief
* Given a note name name, select the given note
* and set the scrollArea's scrollBar position to it
* return true if the given note was found, else return false
*/
bool MainWindow::goToAndSelectNote (const QModelIndex& noteIndex)
{
    bool found = false;

    if(noteIndex.isValid()){
        found = true;
        QModelIndex currentIndex = m_noteView->model()->index(noteIndex.row(), 0);
        m_noteView->setCurrentIndex(currentIndex);
        m_noteView->scrollTo(currentIndex);
        m_currentSelectedNoteProxy = noteIndex;
        showNoteInEditor(noteIndex);
    }else{
        qDebug() << "MainWindow::goToAndSelectNote() : noteIndex is not valid";
    }

    return found;
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
    if(m_isTemp){
        QModelIndex index = m_noteModel->index(0);
        m_noteModel->removeNote(index);

    }else if(!m_selectedNoteBeforeSearching.isValid()
             && m_currentSelectedNoteProxy.isValid()){

        m_selectedNoteBeforeSearching = m_currentSelectedNoteProxy;
    }

    if(keyword.isEmpty()){
        clearSearch(m_selectedNoteBeforeSearching);

    }else{
        findNotesContain(keyword);
    }
}

/**
* @brief
* Create a new note in database
*/
QString MainWindow::createNewNoteInDatabase ()
{
    int counter = m_notesDatabase->value("notesCounter").toInt() + 1;
    m_notesDatabase->setValue("notesCounter", counter);

    QString noteName = QString("noteID_%1").arg(counter);
    m_notesDatabase->setValue(noteName + "/content",  "");

    QString noteDate = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_notesDatabase->setValue(noteName + "/dateCreated", noteDate);
    m_notesDatabase->setValue(noteName + "/dateEdited", noteDate);

    m_notesDatabase->sync();

    return noteName;
}
/**
 * @brief create a new note
 * add it to the database
 * add it to the scrollArea
 */
void MainWindow::createNewNote ()
{
    if(!m_isTemp){
        if(!m_lineEdit->text().isEmpty())
            m_lineEdit->clear();

        QString noteID = createNewNoteInDatabase();
        NoteData* tmpNote = generateNote(noteID, false);
        m_isTemp = true;

        // insert the new note to NoteModel
        QModelIndex indexSrc = m_noteModel->insertNote(tmpNote, 0);

        // increment the index of the previous selected row and save to DB
        m_currentSelectedNoteProxy = m_proxyModel->index(m_currentSelectedNoteProxy.row() + 1, 0);
        if(m_currentSelectedNoteProxy.isValid()){
            m_proxyModel->setData(m_currentSelectedNoteProxy,
                                  QVariant::fromValue(ui->textEdit->toPlainText()),
                                  NoteModel::NoteContent);

            saveNoteToDB(m_currentSelectedNoteProxy);
        }

        // update the editor header date label
        QString dateTimeFromDB = tmpNote->dateTime().toString(Qt::ISODate);
        QString dateTimeForEditor = getNoteDateEditor(dateTimeFromDB);
        ui->editorDateLabel->setText(dateTimeForEditor);

        // clear the textEdit
        ui->textEdit->blockSignals(true);
        ui->textEdit->clear();
        ui->textEdit->setFocus();
        ui->textEdit->blockSignals(false);

        // update the current selected index
        m_currentSelectedNoteProxy = m_proxyModel->mapFromSource(indexSrc);

    }else{
        ui->textEdit->blockSignals(true);
        ui->textEdit->setFocus();
        ui->textEdit->blockSignals(false);
    }

    m_noteView->selectionModel()->select(m_noteModel->index(0,0),
                                         QItemSelectionModel::ClearAndSelect);
    m_noteView->setCurrentIndex(m_noteView->model()->index(0, 0));
    m_noteView->scrollToTop();
}

/**
 * @brief MainWindow::deleteNote delete the specified note
 * @param note  : note to delete
 * @param isFromUser :  true if the user clicked on trash button
 */
void MainWindow::deleteNote(const QModelIndex &noteIndex, bool isFromUser)
{
    if(noteIndex.isValid()){
        if(m_isTemp)
            m_isTemp = false;

        // delete from database
        deleteNoteFromDataBase(noteIndex);

        // delete from models
        m_noteModel->removeNote(noteIndex);

        if(isFromUser){
            // clear text edit
            ui->textEdit->blockSignals(true);
            ui->editorDateLabel->clear();
            ui->textEdit->clear();
            ui->textEdit->clearFocus();
            ui->textEdit->blockSignals(false);

            if(m_noteModel->rowCount() > 0){
                QModelIndex index = m_noteView->currentIndex();
                m_currentSelectedNoteProxy = index;
            }else{
                m_currentSelectedNoteProxy = QModelIndex();
            }
        }
    }else{

    }
}

/**
* @brief
* Delete the selected note
*/
void MainWindow::deleteSelectedNote ()
{
    if(m_noteView->currentIndex().row() != -1){
        QModelIndex index = m_noteView->currentIndex();
        deleteNote(index, true);
        showNoteInEditor(m_currentSelectedNoteProxy);
    }
}

/**
* @brief
* Set focus on textEdit
*/
void MainWindow::setFocusOnText ()
{
    if(m_currentSelectedNoteProxy.isValid() && !ui->textEdit->hasFocus())
        ui->textEdit->setFocus();
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
            m_currentSelectedNoteProxy = m_noteModel->index(aboveIndex.row());
            showNoteInEditor(m_currentSelectedNoteProxy);
        }
    }
}

/**
* @brief
* Select the note below the currentSelectedNote
*/
void MainWindow::selectNoteDown ()
{
    if(m_currentSelectedNoteProxy.isValid()){
        int currentRow = m_noteView->currentIndex().row();
        QModelIndex belowIndex = m_noteView->model()->index(currentRow + 1, 0);
        if(belowIndex.isValid()){
            m_noteView->setCurrentIndex(belowIndex);
            m_currentSelectedNoteProxy = m_noteModel->index(belowIndex.row());
            showNoteInEditor(m_currentSelectedNoteProxy);
        }
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
    this->showNormal(); // I don't know why, but it's need to be here
}

/**
* @brief
* Exit the application
* If a new note created but wasn't edited, delete it from the database
*/
void MainWindow::QuitApplication ()
{
    if(m_isTemp){
        ui->textEdit->blockSignals(true);
        QModelIndex index = m_noteModel->index(0,0);
        deleteNoteFromDataBase(index);
        ui->textEdit->blockSignals(false);
    }

    //QApplication::quit();
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

    QuitApplication();
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
            // && m_currentSelectedNote->isModified() // TODO find a way to fetch the modification value
            && !m_isTemp){

        saveNoteToDB(m_currentSelectedNoteProxy);
    }

    if(m_isTemp){
        QString id = m_currentSelectedNoteProxy.data(NoteModel::NoteID).toString();
        m_notesDatabase->remove(id);
    }

    m_settingsDatabase->setValue("splitterSizes", ui->splitter->saveState());

    m_settingsDatabase->sync();
    m_notesDatabase->sync();

    QWidget::closeEvent(event);
}

/**
* @brief
* Set variables to the position of the window when the mouse is pressed
* And set variables for resizing
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
        }else{
            m_canBeResized = true;
        }
    }

    event->accept();
}

/**
* @brief
* Move the window according to the mouse positions
* And resizing
*/
void MainWindow::mouseMoveEvent (QMouseEvent* event)
{
    if(m_canMoveWindow){
        this->setCursor(Qt::ClosedHandCursor);
        int dx = event->globalX() - m_mousePressX;
        int dy = event->globalY() - m_mousePressY;
        move (dx, dy);

    }else if(m_canBeResized
             && (m_resizeVertLeft
                 || m_resizeVertRight
                 || m_resizeHorzTop
                 || m_resizeHorzBottom)
             ){

        resizeWindow(event);

    }else{
        m_resizeVertLeft = false;
        m_resizeVertRight = false;
        m_resizeHorzTop = false;
        m_resizeHorzBottom = false;

        if(event->pos().x() <4){
            m_resizeVertLeft = true;
            this->setCursor(Qt::SizeHorCursor);
            event->accept();
        }else if(event->pos().x() > this->width() - 4){
            m_resizeVertRight = true;
            this->setCursor(Qt::SizeHorCursor);
            event->accept();
        }else if(event->pos().y() < 4){
            m_resizeHorzTop = true;
            this->setCursor(Qt::SizeVerCursor);
            event->accept();
        }else if(event->pos().y() > this->height() - 4){
            m_resizeHorzBottom = true;
            this->setCursor(Qt::SizeVerCursor);
            event->accept();
        }else{
            this->unsetCursor();
        }
    }
}

/**
* @brief
  * Initialize flags
 */
void MainWindow::mouseReleaseEvent (QMouseEvent *event)
{
    m_canBeResized = false;
    m_canMoveWindow = false;
    m_resizeVertLeft = false;
    m_resizeVertRight = false;
    m_resizeHorzTop = false;
    m_resizeHorzBottom = false;
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

void MainWindow::clearSearch(const QModelIndex& previousNote)
{
    m_proxyModel->setFilterRegExp(QStringLiteral(""));
    bool found = goToAndSelectNote(previousNote);
    if(!found)
        selectFirstNote();
    m_lineEdit->setFocus();
    m_selectedNoteBeforeSearching = QModelIndex();

    m_clearButton->hide();
}

void MainWindow::findNotesContain(const QString& keyword)
{

    m_proxyModel->setFilterRegExp(keyword);
    m_clearButton->show();

    ui->textEdit->blockSignals(true);
    ui->textEdit->clear();
    ui->editorDateLabel->clear();
    ui->textEdit->blockSignals(false);

    if(m_proxyModel->rowCount() > 0){
        selectFirstNote();
    }else{
        m_currentSelectedNoteProxy = QModelIndex();
    }
}

void MainWindow::selectNote(const QModelIndex &noteIndex)
{
    if(noteIndex.isValid()){
        // save the text edit scrollbar position
        if(!m_isTemp && m_currentSelectedNoteProxy.isValid()){
            int pos = ui->textEdit->verticalScrollBar()->value();
            m_noteModel->setData(m_currentSelectedNoteProxy, QVariant::fromValue(pos), NoteModel::NoteScrollbarPos);
        }

        if(m_isTemp && noteIndex != m_currentSelectedNoteProxy){
            deleteNote(m_currentSelectedNoteProxy, false);

        }else if(!m_isTemp
                 && m_currentSelectedNoteProxy.isValid()
                 && noteIndex != m_currentSelectedNoteProxy
                 /*& m_currentSelectedNote->isModified()*/){ // TODO: Find a way for modified

            saveNoteToDB(m_currentSelectedNoteProxy);
        }

        m_currentSelectedNoteProxy = noteIndex;
        showNoteInEditor(m_currentSelectedNoteProxy);

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
 * resize the mainwindow depending on
 * the side from where the mouse used to resize the window
 *
 */
void MainWindow::resizeWindow(QMouseEvent* event)
{
    int newPosX = this->x();
    int newPosY = this->y();
    int newWidth = this->width();
    int newHeight = this->height();

    if(m_resizeVertLeft){
        if(this->width() + this->x() - event->globalX() > minimumWidth()){
            newPosX = event->globalX();
            newPosY = this->y();
            newWidth = this-> width() + x() - event->globalX();
            newHeight = height();
        }

    }else if(m_resizeVertRight){
        if(event->globalX() - this->x() > minimumWidth()){
            newPosX = this->x();
            newPosY = this->y();
            newWidth = event->globalX() - this->x();
            newHeight = this->height();
        }

    }else if(m_resizeHorzTop){
        if(this->height() + this->y() - event->globalY() > minimumHeight()){
            newPosX = this->x();
            newPosY = event->globalY();
            newWidth = this->width();
            newHeight = this->height() + this->y() - event->globalY();
        }

    }else if(m_resizeHorzBottom){
        if(event->globalY() - this->y() > minimumHeight()){
            newPosX = this->x();
            newPosY = this->y();
            newWidth = this->width();
            newHeight = event->globalY() - this->y();
        }
    }

    this->setGeometry(newPosX, newPosY, newWidth, newHeight);
}

/**
* @brief
* Mostly take care on the event happened on widget whose filter installed to tht mainwindow
*/
bool MainWindow::eventFilter (QObject *object, QEvent *event)
{
    if(event->type() == QEvent::Enter){
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

    if(event->type() == QEvent::Leave){
        // When not hovering, change back the icons of the traffic lights to their default icon
        if(object == m_redCloseButton
                || object == m_yellowMinimizeButton
                || object == m_greenMaximizeButton){

            m_redCloseButton->setIcon(QIcon(":images/red.png"));
            m_yellowMinimizeButton->setIcon(QIcon(":images/yellow.png"));
            m_greenMaximizeButton->setIcon(QIcon(":images/green.png"));
        }
    }

    if(event->type() == QEvent::FocusIn){
        if(object == ui->textEdit){

            // When clicking in a note's content while searching,
            // reload all the notes and go and select that note
            if(!m_focusBreaker
                    && !m_lineEdit->text().isEmpty()){

                m_lineEdit->blockSignals(true);
                m_lineEdit->clear();
                m_lineEdit->blockSignals(false);

                m_proxyModel->setFilterRegExp(QStringLiteral(""));
                m_clearButton->hide();

                if(m_currentSelectedNoteProxy.isValid()){
                    bool found = goToAndSelectNote(m_currentSelectedNoteProxy);
                    if(!found)
                        selectFirstNote();
                }else{
                    createNewNote();
                }
            }else if(m_proxyModel->rowCount() == 0){
                createNewNote();
            }
        }
    }

    return QObject::eventFilter(object, event);
}

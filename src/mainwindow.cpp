/**************************************************************************************
* We believe in the power of notes to help us record ideas and thoughts.
* We want people to have an easy, beautiful and simple way of doing that.
* And so we have Notes.
***************************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include <stdio.h>
#include <QShortcut>
#include <QTextStream>

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
    m_tempNote(Q_NULLPTR),
    m_currentSelectedNote(Q_NULLPTR),
    m_currentHoveredNote(Q_NULLPTR),
    m_tempSelectedNoteBeforeSearching(Q_NULLPTR),
    m_noteOnTopInTheLayout(Q_NULLPTR),
    m_canBeResized(false),
    m_resizeHorzTop(false),
    m_resizeHorzBottom(false),
    m_resizeVertRight(false),
    m_resizeVertLeft(false),
    m_canMoveWindow(false),
    m_focusBreaker(false),
    m_isTemp(false)
{
    ui->setupUi(this);
    setupMainWindow();
    setupSignalsSlots();
    setupKeyboardShortcuts();
    setupNewNoteButtonAndTrahButton();
    setupEditorDateLabel();
    setupSplitter();
    setupLine();
    setupRightFrame ();
    setupTitleBarButtons();
    setupLineEdit();
    setupScrollArea();
    setupTextEdit();
    setupDatabases();
    restoreStates();
    setLayoutForScrollArea();

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

void MainWindow::paintEvent(QPaintEvent *e)
{
    /*QPainter painter(this);

    painter.setPen(QPen(QColor::fromRgb(254, 206, 9), 2));
    painter.drawRoundedRect(1,1,871, 394,0,0);*/

    e->accept();
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

    QPalette pal(palette());
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    ui->newNoteButton->setToolTip("Create New Note");
    ui->trashButton->setToolTip("Delete Selected Note");
}

/**
* @brief
* Setting up the keyboard shortcuts
*/
void MainWindow::setupKeyboardShortcuts ()
{
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_N), this, SLOT(createNewNoteWithAnimation()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Delete), this, SLOT(deleteSelectedNote()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), ui->lineEdit, SLOT(setFocus()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_E), ui->lineEdit, SLOT(clear()));
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
    ui->newNoteButton->setGeometry(ui->newNoteButton->x(), ui->newNoteButton->y(), 50, 32);
    ui->newNoteButton->setIconSize(QSize(16, 16));
    ui->trashButton->setGeometry(676, ui->trashButton->y(), 50, 32);
    ui->trashButton->setIconSize(QSize(14, 18));
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

    ui->redCloseButton->setStyleSheet(ss);
    ui->yellowMinimizeButton->setStyleSheet(ss);
    ui->greenMaximizeButton->setStyleSheet(ss);

    ui->redCloseButton->installEventFilter(this);
    ui->yellowMinimizeButton->installEventFilter(this);
    ui->greenMaximizeButton->installEventFilter(this);

}

/**
 * @brief connect between signals and slots
 */
void MainWindow::setupSignalsSlots()
{
    // green button
    connect(ui->greenMaximizeButton, SIGNAL(pressed()), this, SLOT(onGreenMaximizeButtonPressed()));
    connect(ui->greenMaximizeButton, SIGNAL(clicked()), this, SLOT(onGreenMaximizeButtonClicked()));
    // red button
    connect(ui->redCloseButton, SIGNAL(pressed()), this, SLOT(onRedCloseButtonPressed()));
    connect(ui->redCloseButton, SIGNAL(clicked()), this, SLOT(onRedCloseButtonClicked()));
    // yellow button
    connect(ui->yellowMinimizeButton, SIGNAL(pressed()), this, SLOT(onYellowMinimizeButtonPressed()));
    connect(ui->yellowMinimizeButton, SIGNAL(clicked()), this, SLOT(onYellowMinimizeButtonClicked()));
    // new note button
    connect(ui->newNoteButton, SIGNAL(clicked(bool)), this, SLOT(onNewNoteButtonClicked()));
    // delete note button
    connect(ui->trashButton, SIGNAL(clicked(bool)), this, SLOT(onTrashButtonClicked()));
    // text edit text changed
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(onTextEditTextChanged()));
    // line edit text changed
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(onLineEditTextChanged(QString)));
}

/**
* @brief
* Set the lineedit to start a bit to the right and end a bit to the left (pedding)
*/
void MainWindow::setupLineEdit ()
{
    // There is a problem with Helvetica here so we usa Arial, someone sguggestion?
#ifdef __APPLE__
    ui->lineEdit->setFont(QFont("Arial", 12));
#endif

    QLineEdit* lineEdit = ui->lineEdit;

    int frameWidth = ui->lineEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
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
* Setting up the scrollArea widget:
*/
void MainWindow::setupScrollArea ()
{
#ifdef __APPLE__
    ui->scrollArea->setGeometry(ui->scrollArea->x() + 1, ui->scrollArea->y(), ui->scrollArea->width() - 1, ui->scrollArea->height());
#endif

    QString ss = QString("QScrollArea QWidget{background-color:white;} "
                         "QScrollBar {margin-right: 2px; background: transparent;} "
                         "QScrollBar:hover { background-color: rgb(217, 217, 217);}"
                         "QScrollBar:handle:vertical:hover { background: rgb(170, 170, 171); } "
                         "QScrollBar:handle:vertical:pressed { background: rgb(149, 149, 149);}"
                         "QScrollBar:vertical { border: none; width: 10px; border-radius: 4px;} "
                         "QScrollBar::handle:vertical { border-radius: 4px; background: rgb(188, 188, 188); min-height: 20px; }  "
                         "QScrollBar::add-line:vertical { height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  "
                         "QScrollBar::sub-line:vertical { height: 0px; subcontrol-position: top; subcontrol-origin: margin; }");

    ui->scrollArea->setStyleSheet(ss);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
                         ).arg(QString::number(ui->newNoteButton->width() - m_textEditLeftPadding), "27");

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
* @brief configure the layout contained in scrollArea
*/
void MainWindow::setLayoutForScrollArea ()
{
    m_noteWidgetsContainer = ui->verticalLayout_scrollArea_2;

    m_noteWidgetsContainer->addStretch();
    m_noteWidgetsContainer->setSpacing(0);
    m_noteWidgetsContainer->setContentsMargins(0,0,0,0);
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
    NoteData* newNote = new NoteData(noteName, this);

    QString dateDB = m_notesDatabase->value(noteName + "/dateEdited", "Error").toString();
    newNote->setDateTime(QDateTime::fromString(dateDB, Qt::ISODate));
    QString contentText = m_notesDatabase->value(noteName + "/content", "Error").toString();
    newNote->setText(contentText);
    QString firstLine = isLoadingOrNew ? getFirstLine(contentText) : "New Note";
    newNote->setTitle(firstLine);

    connect(newNote, SIGNAL(pressed()), this, SLOT(onNotePressed()));

    return newNote;
}

/**
 * @brief show the specified note content text in the text editor
 * Set editorDateLabel text to the the selected note date
 * And restore the scrollBar position if it changed before.
 */
void MainWindow::showNoteInEditor(NoteData *note)
{
    ui->textEdit->blockSignals(true);
    // set text and date
    ui->textEdit->setText(note->text());
    QString noteDate = note->dateTime().toString(Qt::ISODate);
    QString noteDateEditor = getNoteDateEditor(noteDate);
    ui->editorDateLabel->setText(noteDateEditor);
    // set scrollbar position
    int tempScrollBarPosition = note->scrollBarPosition();
    ui->textEdit->verticalScrollBar()->setValue(tempScrollBarPosition);
    ui->textEdit->blockSignals(false);
}

/**
* @brief
* Load all the notes from database into the notes list (in scrollArea)
*/
void MainWindow::loadNotes ()
{
    QStringList dbKeys = m_notesDatabase->allKeys();
    QList<NoteData *> sortedNotesList;

    auto it = dbKeys.begin();
    for(; it < dbKeys.end()-1; it += 3){
        QString noteName = it->split("/")[0];
        NoteData* newNote = generateNote(noteName, true);
        sortedNotesList.push_back(newNote);
    }

    std::stable_sort(std::begin(sortedNotesList), std::end(sortedNotesList),[&](NoteData *lhs, NoteData *rhs){
        return lhs->dateTime() < rhs->dateTime();
    });

    foreach (NoteData *note, sortedNotesList) {
        m_allNotesList.push_back(note);
        m_visibleNotesList.push_back(note);
        m_noteWidgetsContainer->insertWidget(0, note, 0, Qt::AlignTop);
    }

    if(!m_allNotesList.isEmpty())
        m_noteOnTopInTheLayout = m_allNotesList.back();
}

/**
* @brief
* save the current note to database
*/
void MainWindow::saveCurrentNoteToDB()
{
    if(m_currentSelectedNote != Q_NULLPTR){
        m_notesDatabase->setValue(m_currentSelectedNote->noteName() + "/content", m_currentSelectedNote->text());

        QString noteDate = m_currentSelectedNote->dateTime().toString(Qt::ISODate);
        m_notesDatabase->setValue(m_currentSelectedNote->noteName() + "/dateEdited", noteDate);

        m_notesDatabase->sync();

        m_currentSelectedNote->setModified(false);
    }
}

/**
* @brief
* Select the first note in the notes list
*/
void MainWindow::selectFirstNote ()
{
    if(!m_visibleNotesList.empty()){
        m_currentSelectedNote != Q_NULLPTR ? m_currentSelectedNote->setSelected(false) : void();
        m_currentSelectedNote = m_visibleNotesList.back();
        m_currentSelectedNote->setSelected(true);
        this->showNoteInEditor(m_currentSelectedNote);
    }
}

/**
* @brief
* create a new note if there are no notes
*/
void MainWindow::createNewNoteIfEmpty ()
{
    if(m_allNotesList.empty())
        this->createNewNoteWithAnimation();
}

/**
* @brief
* Create a new note when clicking the 'new note' button
*/
void MainWindow::onNewNoteButtonClicked()
{
    this->createNewNoteWithAnimation();
}

/**
* @brief
* Delete selected note when clicking the 'delete note' button
*/
void MainWindow::onTrashButtonClicked()
{
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
void MainWindow::onNotePressed ()
{
    if(sender() != Q_NULLPTR){
        NoteData* pressedNote = qobject_cast<NoteData *>(sender());

        if(m_currentSelectedNote != Q_NULLPTR && pressedNote != m_currentSelectedNote)
            m_currentSelectedNote->setSelected(false);

        if(m_tempNote == Q_NULLPTR && m_currentSelectedNote != Q_NULLPTR)
            m_currentSelectedNote->setScrollBarPosition(ui->textEdit->verticalScrollBar()->value());

        if(m_tempNote != Q_NULLPTR && pressedNote != m_tempNote){
            deleteNoteWithAnimation(m_tempNote,false);
        }else if(m_tempNote == Q_NULLPTR
                 && m_currentSelectedNote != Q_NULLPTR
                 && pressedNote != m_currentSelectedNote
                 && m_currentSelectedNote->isModified()){
            saveCurrentNoteToDB();
        }

        m_currentSelectedNote = pressedNote;

        m_currentSelectedNote->setSelectedWithFocus(true,true);
        showNoteInEditor(m_currentSelectedNote);
    }
}

/**
* @brief
* Delete a given note from the database
*/
void MainWindow::deleteNoteFromDataBase (NoteData *note)
{
    m_allNotesList.removeOne(note);

    // Putting the deleted note in trash
    int counter = m_trashDatabase->value("notesCounter").toInt() + 1;
    m_trashDatabase->setValue("notesCounter", counter);

    QString noteName = QString("noteID_%1").arg(counter);
    m_trashDatabase->setValue(noteName + "/content",  m_notesDatabase->value(note->noteName() + "/content", "Error"));

    QString dateDBCreated = m_notesDatabase->value(note->noteName() + "/dateCreated", "Error").toString();
    QString dateDBEdited = m_notesDatabase->value(note->noteName() + "/dateEdited", "Error").toString();
    QString noteDate = QDateTime::currentDateTime().toString(Qt::ISODate);

    m_trashDatabase->setValue(noteName + "/dateCreated", dateDBCreated);
    m_trashDatabase->setValue(noteName + "/dateEdited", dateDBEdited);
    m_trashDatabase->setValue(noteName + "/dateTrashed", noteDate);

    m_notesDatabase->remove(note->noteName());

    m_notesDatabase->sync();
    m_trashDatabase->sync();
}

/**
* @brief
* When the text on textEdit change:
* if the note edited is not on top of the list, we will make that happen
* If the text changed is of a new (empty) note, reset temp values
*/
void MainWindow::onTextEditTextChanged ()
{
    //if(text has really changed)
    if(m_currentSelectedNote != Q_NULLPTR
            && ui->textEdit->toPlainText() != m_currentSelectedNote->text()){

        // update modification flag
        m_currentSelectedNote->setModified(true);
        // update text
        m_currentSelectedNote->setText(ui->textEdit->toPlainText());
        // update title
        QString firstline = getFirstLine(m_currentSelectedNote->text());
        m_currentSelectedNote->setTitle(firstline);
        // update time
        m_currentSelectedNote->setDateTime(QDateTime::currentDateTime());
        QString noteDate = m_currentSelectedNote->dateTime().toString(Qt::ISODate);
        // m_currentSelectedNote->m_dateLabel->setText(getNoteDate(m_currentSelectedNote->m_dateTime));
        ui->editorDateLabel->setText(getNoteDateEditor(noteDate));

        if(m_currentSelectedNote != m_noteOnTopInTheLayout)
            moveNoteToTopWithAnimation();
    }

    if(m_isTemp && !m_tempNote->text().isEmpty()){
        m_tempNote = Q_NULLPTR;
        m_isTemp = false;
    }
}

/**
* @brief
* Remove all the notes from the scrollArea
*/
void MainWindow::clearAllNotesFromVisual ()
{
    foreach (NoteData *note, m_visibleNotesList) {
        note->setSelected(false);
        note->hide();
    }

    m_visibleNotesList.clear();
    m_currentSelectedNote = Q_NULLPTR;
}

/**
* @brief
* Given a note name name, select the given note
* and set the scrollArea's scrollBar position to it
* return true if the given note was found, else return false
*/
bool MainWindow::goToAndSelectNote (NoteData* note)
{
    bool found = false;

    int noteIndex = m_visibleNotesList.indexOf(note);
    if(noteIndex != -1){
        found = true;
        note->pressed();
        int noteSz = note->height();
        int sbVal = (m_visibleNotesList.size()-1 - noteIndex) * noteSz;
        ui->scrollArea->verticalScrollBar()->setValue(sbVal);
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

    if(m_tempNote != Q_NULLPTR){
        deleteNoteWithAnimation(m_tempNote, false);
    }else if(m_currentSelectedNote != Q_NULLPTR){
        m_tempSelectedNoteBeforeSearching = m_currentSelectedNote;
    }

    clearAllNotesFromVisual();

    if(keyword.isEmpty()){

        foreach(NoteData* note, m_allNotesList){
            note->show();
            m_visibleNotesList.push_back(note);
        }

        bool found = goToAndSelectNote(m_tempSelectedNoteBeforeSearching);
        if(!found)
            selectFirstNote();
        ui->lineEdit->setFocus();
        m_tempSelectedNoteBeforeSearching = Q_NULLPTR;
        m_clearButton->hide();

    }else{
        m_clearButton->show();

        int scVal = ui->scrollArea->verticalScrollBar()->minimum();
        ui->scrollArea->verticalScrollBar()->setValue(scVal);

        if(m_visibleNotesList.empty()){
            ui->textEdit->blockSignals(true);
            ui->textEdit->clear();
            ui->editorDateLabel->clear();
            ui->textEdit->blockSignals(false);
        }

        foreach (NoteData *note, m_allNotesList) {
            bool isFound = note->text().contains(keyword, Qt::CaseInsensitive);
            if(isFound){
                note->show();
                m_visibleNotesList.push_back(note);
            }
        }

        selectFirstNote();
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
        if(!ui->lineEdit->text().isEmpty())
            ui->lineEdit->clear();

        QString noteName = createNewNoteInDatabase();
        m_tempNote = generateNote(noteName, false);
        m_noteOnTopInTheLayout = m_tempNote;
        m_isTemp = true;

        m_visibleNotesList.push_back(m_tempNote);
        m_allNotesList.push_back(m_tempNote);

        m_noteWidgetsContainer->insertWidget(0, m_tempNote, 0, Qt::AlignTop);

        if(m_currentSelectedNote != Q_NULLPTR){
            saveCurrentNoteToDB();
            m_currentSelectedNote->setSelected(false);
        }

        m_currentSelectedNote = m_tempNote;
        m_currentSelectedNote->setSelectedWithFocus(true, false);

        QString dateTimeFromDB = m_tempNote->dateTime().toString(Qt::ISODate);
        QString dateTimeForEditor = getNoteDateEditor(dateTimeFromDB);
        ui->editorDateLabel->setText(dateTimeForEditor);

        ui->textEdit->blockSignals(true);
        ui->textEdit->clear();
        ui->textEdit->setFocus();
        ui->textEdit->blockSignals(false);

    }else{
        ui->textEdit->blockSignals(true);
        ui->textEdit->setFocus();
        ui->textEdit->blockSignals(false);
    }

    int scVal = ui->scrollArea->verticalScrollBar()->minimum();
    ui->scrollArea->verticalScrollBar()->setValue(scVal);
}

void MainWindow::createNewNoteWithAnimation()
{
    this->createNewNote();

    int noteHeight = m_tempNote->height();
    m_tempNote->setFixedHeight(0);

    QPair<int, int> start = QPair<int,int>(0,0);
    QPair<int, int> end = QPair<int,int>(0,noteHeight);
    QPropertyAnimation *animation = createAnimation(m_tempNote,start,end,190);

    connect(animation, &QPropertyAnimation::valueChanged, this, [&,noteHeight](QVariant v){
        m_tempNote->update();
        m_tempNote->setFixedHeight(v.toRect().height());
    });
    animation->start();
}

/**
 * @brief MainWindow::deleteNote delete the specified note
 * @param note  : note to delete
 * @param isFromUser :  true if the user clicked on trash button
 */
void MainWindow::deleteNote(NoteData *note, bool isFromUser)
{
    if(note == m_tempNote){
        m_tempNote = Q_NULLPTR;
        m_isTemp = false;
    }

    // update m_noteOnTopInTheLayout
    int noteDbIndex = m_allNotesList.indexOf(note);
    if(noteDbIndex == m_allNotesList.size() - 1 && m_allNotesList.size() > 1){
        m_noteOnTopInTheLayout = m_allNotesList.at( m_allNotesList.size() - 2);
    }else if(m_allNotesList.size() == 1){
        m_noteOnTopInTheLayout = Q_NULLPTR;
    }

    // delete from database
    deleteNoteFromDataBase(note);

    int noteVisibleIdx = m_visibleNotesList.indexOf(note);

    // delete from visible list
    m_visibleNotesList.removeOne(note);
    // remove note widget from the layout
    m_noteWidgetsContainer->removeWidget(note);
    // delete
    delete note;

    if(isFromUser){

        // clear text edit
        ui->textEdit->blockSignals(true);
        ui->editorDateLabel->clear();
        ui->textEdit->clear();
        ui->textEdit->clearFocus();
        ui->textEdit->blockSignals(false);

        if(m_visibleNotesList.size() > 0){
            // select the next note and highlight it
            m_currentSelectedNote = noteVisibleIdx == 0? m_visibleNotesList.at(0)
                                                       : m_visibleNotesList.at(noteVisibleIdx-1);
            m_currentSelectedNote->setSelectedWithFocus(true,true);

            // show the note in text edit
            showNoteInEditor(m_currentSelectedNote);
        }else{
            m_currentSelectedNote = Q_NULLPTR;
        }
    }
}

/**
 * @brief delete the specified note with animation,
 * @param isFromUser : true if the clicked on trash button
 */
void MainWindow::deleteNoteWithAnimation(NoteData *note, bool isFromUser)
{
    if(note != Q_NULLPTR){
        // animate the deletion
        auto start = QPair<int, int>(note->y(),note->height());
        auto end = QPair<int, int>(note->y(),0);
        QPropertyAnimation* animation = createAnimation(note, start, end, 190);

        connect(animation, &QPropertyAnimation::valueChanged, [this, note, isFromUser](QVariant v){
            note->setFixedHeight(v.toRect().height());
        });

        connect(animation, &QPropertyAnimation::finished, [this, note, isFromUser](){
            deleteNote(note, isFromUser);
        });

        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

/**
* @brief
* Delete the selected note
*/
void MainWindow::deleteSelectedNote ()
{
    if(m_currentSelectedNote != Q_NULLPTR)
        deleteNoteWithAnimation(m_currentSelectedNote, true);
}

/**
* @brief
* Set focus on textEdit
*/
void MainWindow::setFocusOnText ()
{
    if(m_currentSelectedNote != Q_NULLPTR && !ui->textEdit->hasFocus())
        ui->textEdit->setFocus();
}

/**
* @brief
* Set focus on current selected note
*/
void MainWindow::setFocusOnCurrentNote ()
{
    if(m_currentSelectedNote != Q_NULLPTR)
        m_currentSelectedNote->setSelectedWithFocus(true,true);
}

/**
* @brief
* Select the note above the currentSelectedNote
*/
void MainWindow::selectNoteUp ()
{
    if(m_currentSelectedNote != Q_NULLPTR){
        int currNoteIndex = m_visibleNotesList.indexOf(m_currentSelectedNote);
        if(currNoteIndex < m_visibleNotesList.size()-1){
            NoteData* aboveNote = m_visibleNotesList.at(currNoteIndex+1);
            ui->scrollArea->ensureWidgetVisible(aboveNote);
            m_visibleNotesList.at(currNoteIndex + 1)->pressed();
        }
    }
}

/**
* @brief
* Select the note below the currentSelectedNote
*/
void MainWindow::selectNoteDown ()
{
    if(m_currentSelectedNote != Q_NULLPTR){
        int currNoteIndex = m_visibleNotesList.indexOf(m_currentSelectedNote);
        if(currNoteIndex > 0){
            NoteData* aboveNote = m_visibleNotesList.at(currNoteIndex-1);
            ui->scrollArea->ensureWidgetVisible(aboveNote);
            aboveNote->pressed();
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
    if(m_tempNote != Q_NULLPTR){
        ui->textEdit->blockSignals(true);
        deleteNoteFromDataBase(m_tempNote);
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
        ui->greenMaximizeButton->setIcon(QIcon(":images/greenInPressed.png"));
    }else{
        ui->greenMaximizeButton->setIcon(QIcon(":images/greenPressed.png"));
    }
}

/**
* @brief
* When the yellow button is pressed set it's icon accordingly
*/
void MainWindow::onYellowMinimizeButtonPressed ()
{
    ui->yellowMinimizeButton->setIcon(QIcon(":images/yellowPressed.png"));
}

/**
* @brief
* When the red button is pressed set it's icon accordingly
*/
void MainWindow::onRedCloseButtonPressed ()
{
    ui->redCloseButton->setIcon(QIcon(":images/redPressed.png"));
}

/**
* @brief
* When the green button is released the window goes fullscrren
*/
void MainWindow::onGreenMaximizeButtonClicked()
{
    ui->greenMaximizeButton->setIcon(QIcon(":images/green.png"));

    fullscreenWindow();
}

/**
* @brief
* When yellow button is released the window is minimized
*/
void MainWindow::onYellowMinimizeButtonClicked()
{
    ui->yellowMinimizeButton->setIcon(QIcon(":images/yellow.png"));

    minimizeWindow();
}

/**
* @brief
* When red button is released the window get closed
* If a new note created but wasn't edited, delete it from the database
*/
void MainWindow::onRedCloseButtonClicked()
{
    ui->redCloseButton->setIcon(QIcon(":images/red.png"));

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

    if(m_currentSelectedNote != Q_NULLPTR
            && m_currentSelectedNote->isModified()
            && m_tempNote == Q_NULLPTR){

        saveCurrentNoteToDB();
    }

    if(m_tempNote != Q_NULLPTR)
        m_notesDatabase->remove(m_tempNote->noteName());

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
 * @brief MainWindow::createAnimation create QPropertyAnimation
 * on the geometry of the widget specified
 * @param note : the note to appy the annimation on
 * @param start : the start value (y, height) of the animation
 * @param end : the end value (y, height) of the animation
 * @param duration : duration of the animation
 * @return a pointer to the animation created
 */
QPropertyAnimation *MainWindow::createAnimation(NoteData *note, const QPair<int, int>& start, const QPair<int, int>& end, const int duration)
{
    QRect startRect(note->x(),start.first, note->width(), start.second);
    QRect endRect(note->x(),end.first, note->width(), end.second);
    QPropertyAnimation *animation = new QPropertyAnimation(note, "geometry", this);
    animation->setEasingCurve(QEasingCurve::Linear);
    animation->setDuration(duration);
    animation->setStartValue(startRect);
    animation->setEndValue(endRect);

    return animation;
}

/**
 * @brief MainWindow::moveNoteToTop : moves the current note Widget
 * to the top of the layout
 */
void MainWindow::moveNoteToTop()
{
    if(m_currentSelectedNote != Q_NULLPTR
            && m_noteOnTopInTheLayout != m_currentSelectedNote){

        m_noteOnTopInTheLayout = m_currentSelectedNote;

        // scroll to top
        int scrollBarMinValue = ui->scrollArea->verticalScrollBar()->minimum();
        ui->scrollArea->verticalScrollBar()->setValue(scrollBarMinValue);

        // move the current selected note to the top (m_allNotesList/m_visibleNotesList: top = back)
        m_noteWidgetsContainer->removeWidget(m_currentSelectedNote);
        m_noteWidgetsContainer->insertWidget(0,m_currentSelectedNote);
        int currNoteDbIdx = m_allNotesList.indexOf(m_currentSelectedNote);
        m_allNotesList.move(currNoteDbIdx, m_allNotesList.count()-1);
        int currNoteVisibleIdx = m_visibleNotesList.indexOf(m_currentSelectedNote);
        m_visibleNotesList.move(currNoteVisibleIdx, m_visibleNotesList.count()-1);
    }
}

/**
 * @brief MainWindow::moveNoteToTopWithAnimation : moves the current note Widget
 * to the top of the layout using animation
 */
void MainWindow::moveNoteToTopWithAnimation()
{
    int tempHeight = m_currentSelectedNote->height();

    // Animation for removing
    int currNotePosY = m_currentSelectedNote->y();
    int currNoteHeight = m_currentSelectedNote->height();
    auto rmStart = QPair<int,int>(currNotePosY, currNoteHeight);
    auto rmEnd = QPair<int,int>(m_currentSelectedNote->y(), 0);
    QPropertyAnimation *rmAnimation = createAnimation(m_currentSelectedNote,
                                                      rmStart,
                                                      rmEnd,
                                                      60);

    // animation for inserting
    auto insStart = QPair<int,int>(tempHeight, 0);
    auto insEnd = QPair<int,int>(0, tempHeight);
    QPropertyAnimation *insAnimation = createAnimation(m_currentSelectedNote,
                                                       insStart,
                                                       insEnd,
                                                       60);

    // start the inserting animation after removing annimation finishes
    connect(rmAnimation, &QPropertyAnimation::finished, this, [this, insAnimation](){
        moveNoteToTop();
        // start the insert animation
        insAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // start the remove animation
    rmAnimation->start(QAbstractAnimation::DeleteWhenStopped);
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
        if(object == ui->redCloseButton
                || object == ui->yellowMinimizeButton
                || object == ui->greenMaximizeButton){

            ui->redCloseButton->setIcon(QIcon(":images/redHovered.png"));
            ui->yellowMinimizeButton->setIcon(QIcon(":images/yellowHovered.png"));
            if(this->windowState() == Qt::WindowFullScreen){
                ui->greenMaximizeButton->setIcon(QIcon(":images/greenInHovered.png"));
            }else{
                ui->greenMaximizeButton->setIcon(QIcon(":images/greenHovered.png"));
            }
        }
    }

    if(event->type() == QEvent::Leave){
        // When not hivering, change back the icons of the traffic lights to their default icon
        if(object == ui->redCloseButton
                || object == ui->yellowMinimizeButton
                || object == ui->greenMaximizeButton){

            ui->redCloseButton->setIcon(QIcon(":images/red.png"));
            ui->yellowMinimizeButton->setIcon(QIcon(":images/yellow.png"));
            ui->greenMaximizeButton->setIcon(QIcon(":images/green.png"));
        }
    }

    if(event->type() == QEvent::FocusIn){
        if(object == ui->textEdit){

            if(m_visibleNotesList.isEmpty()){
                this->createNewNoteWithAnimation();
            }else if(m_currentSelectedNote != Q_NULLPTR){
                m_currentSelectedNote->setSelectedWithFocus(true, false);
            }

            // When clicking in a note's content while searching,
            // reload all the notes and go and select that note
            if(!m_focusBreaker
                    && !ui->lineEdit->text().isEmpty()
                    && m_currentSelectedNote != Q_NULLPTR){

                ui->lineEdit->clear();
                goToAndSelectNote(m_currentSelectedNote);
                ui->textEdit->setFocus();
            }
        }
    }

    return QObject::eventFilter(object, event);
}

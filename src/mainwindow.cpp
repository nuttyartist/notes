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
    m_lay(0),
    m_tempNote(0),
    m_currentSelectedNote(0),
    m_currentHoveredNote(0),
    m_tempSelectedNoteBeforeSearching(0)
{
    ui->setupUi(this);
    setupMainWindow();
    setupSignalsSlots();
    setupKeyboardShortcuts();
    setupNewNoteButtonAndTrahButton();
    setupEditorDateLabel();
    setupSplitter();
    setupLine();
    setupFrame ();
    setupTitleBarButtons();
    createMagnifyingGlassIcon();
    setupLineEdit();
    setupScrollArea();
    setupTextEdit();
    initializeVariables();
    setupDatabases();
    restoreStates();
    setLayoutForScrollArea();

    QTimer::singleShot(200,this, SLOT(InitData()));
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    /*QPainter painter(this);

    painter.setPen(QPen(QColor::fromRgb(254, 206, 9), 2));
    painter.drawRoundedRect(1,1,871, 394,0,0);*/

    e->accept();
}

/**
* Deconstructor of the class
*/
MainWindow::~MainWindow ()
{
    delete ui;
}

/**
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
* Setting up the keyboard shortcuts
*/
void MainWindow::setupKeyboardShortcuts ()
{
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_N), this, SLOT(createNewNote()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Delete), this, SLOT(deleteSelectedNote()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), ui->lineEdit, SLOT(setFocus()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_E), ui->lineEdit, SLOT(clear()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this, SLOT(setFocusOnScrollArea()));
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
* Set up the splitter that control the size of the scrollArea and the textEdit
*/
void MainWindow::setupSplitter()
{
    ui->splitter->setStretchFactor(1, 1);
    ui->splitter->setStretchFactor(2, 0);
}

/**
* Set up the vertical line that seperate between the scrollArea to the textEdit
*/
void MainWindow::setupLine ()
{
    ui->line->setStyleSheet("border: 1px solid rgb(221, 221, 221)");
}

/**
* Set up a frame above textEdit and behind the other widgets for a unifed background in thet editor section
*/
void MainWindow::setupFrame ()
{
    ui->frame->setStyleSheet("QFrame { background-image: url(:images/textSideBackground.png); border: none;}");

}

/**
* Setting up the red (close), yellow (minimize), and green (maximize) buttons
* Make only the buttons icon visible
* And install this class event filter to them, to act when hovering on one of them
*/
void MainWindow::setupTitleBarButtons ()
{
    ui->redCloseButton->setStyleSheet("QPushButton { border: none; padding: 0px; }");
    ui->yellowMinimizeButton->setStyleSheet("QPushButton { border: none; padding: 0px; }");
    ui->greenMaximizeButton->setStyleSheet("QPushButton { border: none; padding: 0px; }");

    ui->redCloseButton->installEventFilter(this);
    ui->yellowMinimizeButton->installEventFilter(this);
    ui->greenMaximizeButton->installEventFilter(this);

}

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
* Setting up the magnifing glass icon in the search box (lineEdit)
*/
void MainWindow::createMagnifyingGlassIcon ()
{
    QToolButton *searchButton = new QToolButton(ui->lineEdit);

    QPixmap newPixmap(":images/magnifyingGlass.png");
    searchButton->setIcon(QIcon(newPixmap));
    QSize searchSize(25, 25);
    searchButton->move(-1, -1);
    searchButton->setIconSize(searchSize);
    searchButton->setCursor(Qt::ArrowCursor);
    searchButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
}

/**
* Set the lineedit to start a bit to the right and end a bit to the left (pedding)
*/
void MainWindow::setupLineEdit ()
{
    // There is a problem with Helvetica here so we usa Arial, someone sguggestion?
#ifdef __APPLE__
    ui->lineEdit->setFont(QFont("Arial", 12));
#endif

    int frameWidth = ui->lineEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    ui->lineEdit->setStyleSheet(QString("QLineEdit { padding-right: %1px; padding-left: 19px } ") // border-radius: 3px; border: 1px solid rgb(173, 169, 165);
                                .arg(frameWidth + 1));
}

/**
* Setting up the scrollArea widget:
* Setup the style of the scrollBar
* Disable the horizontal scrolling in scrollArea
* And install this class event filter to act when scrollArea is out of focus and when hovering the scrollBar
*/
void MainWindow::setupScrollArea ()
{
#ifdef __APPLE__
    ui->scrollArea->setGeometry(ui->scrollArea->x() + 1, ui->scrollArea->y(), ui->scrollArea->width() - 1, ui->scrollArea->height());
#endif

    QString styleSheet = QString("QScrollArea QWidget{background-color:white; color:black;} "
                                 "QScrollBar {margin-right: 2px; background: transparent;} "
                                 "QScrollBar:hover { background-color: rgb(217, 217, 217);}"
                                 "QScrollBar:handle:vertical:hover { background: rgb(170, 170, 171); } "
                                 "QScrollBar:handle:vertical:pressed { background: rgb(149, 149, 149);}"
                                 "QScrollBar:vertical { border: none; width: 10px; border-radius: 4px;} "
                                 "QScrollBar::handle:vertical { border-radius: 4px; background: rgb(188, 188, 188); min-height: 20px; }  "
                                 "QScrollBar::add-line:vertical { height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  "
                                 "QScrollBar::sub-line:vertical { height: 0px; subcontrol-position: top; subcontrol-origin: margin; }");

    ui->scrollArea->setStyleSheet(styleSheet);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

/**
* Setting up textEdit:
* Setup the style of the scrollBar and set textEdit background to an image
* Make the textEdit pedding few pixels right and left, to compel with a beautiful proportional grid
* And install this class event filter to act when scrollArea is out of focus and when hovering the scrollBar
*/
void MainWindow::setupTextEdit ()
{
    //? ui->textEdit->setTextColor(QColor::fromRgb(25, 25, 25));

#ifdef Q_OS_LINUX
    m_textEditLeftPadding = 5;
#elif _WIN32
    textEditLeftPadding = 5;
#elif __APPLE__
    textEditLeftPadding = 18;
#else
#error "We don't support that version yet..."
#endif

    QString styleSheet = QString("QTextEdit {background-image: url(:images/textSideBackground.png); padding-left: %1px; padding-right: %2px; padding-bottom:2px;} "
                                 "QScrollBar::handle:vertical:hover { background: rgb(170, 170, 171); } "
                                 "QScrollBar::handle:vertical:pressed { background: rgb(149, 149, 149); } "
                                 "QScrollBar::handle:vertical { border-radius: 4px; background: rgb(188, 188, 188); min-height: 20px; }  "
                                 "QScrollBar::vertical {border-radius: 4px; width: 8px; color: rgba(255, 255, 255,0);} "
                                 "QScrollBar {margin: 0; background: transparent;} "
                                 "QScrollBar:hover { background-color: rgb(217, 217, 217);}"
                                 "QScrollBar::add-line:vertical { width:0px; height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  "
                                 "QScrollBar::sub-line:vertical { width:0px; height: 0px; subcontrol-position: top; subcontrol-origin: margin; }"
                                 ).arg(QString::number(ui->newNoteButton->width() - m_textEditLeftPadding), "27");
    ui->textEdit->setStyleSheet(styleSheet);

    ui->textEdit->installEventFilter(this);
    ui->textEdit->verticalScrollBar()->installEventFilter(this);

    connect(ui->textEdit->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(textEditScrollBarValueChange(int)));

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

/**
* I don't know why i need to initialize them, but i have to (Aren't they already suppose to be a null and a false?)
*/
void MainWindow::initializeVariables ()
{
    m_currentSelectedNote = 0;
    m_currentHoveredNote = 0;
    m_tempNote = 0;
    m_isTemp = false;
    m_currentVerticalScrollAreaRange = 0;
    m_canMoveWindow = false;
    m_focusBreaker = false;
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
* The layout we use to store the list of notes (made by groupboxes, button, frame and labels) inside ScrollArea
*/
void MainWindow::setLayoutForScrollArea ()
{
    m_lay = ui->verticalLayout_scrollArea_2;

    m_lay->addStretch();
    m_lay->setSpacing(0);
    m_lay->setMargin(0);
    m_lay->setContentsMargins(0,0,0,0);
}

/**
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
* Get a string 'str' and return it's short form with "..." if it is wider than the label present it (titleLabel)
* Example: ("This string is really really long") -> "This string is rea..."
*/
QString MainWindow::getElidedText (QString str, QFontMetrics labelFontMetrics, int size)
{
    QString croppedText = labelFontMetrics.elidedText(str, Qt::ElideRight, size);

    if(croppedText.right(2) == " …") // Using ellipsis character  // Old: if(labelFontMetrics.width(str) >= size && croppedText.at(croppedText.length()-2) == ' ')
        croppedText.replace(croppedText.length()-2, 1, '.');

    return croppedText;
}

/**
* Get a note's noteData and return its first line after eliding it.
*/
QPair<QString, QString> MainWindow::getFirstLineAndElide(NoteData *note)
{
    // We can improve more if we could load right from the Hard Drive database only one line and not the whole text
    QString text = m_notesDatabase->value(note->m_noteName + "/content", "Error").toString();

    QString firstLine = getFirstLine(text);

    QString firstLineElided = getElidedText(firstLine, note->m_titleLabel->fontMetrics(), ui->splitter->sizes().first()-20);

    return std::move(QPair<QString, QString>(firstLine, firstLineElided));
}

/**
* Get a date string of a note from database and put it's data into a QDateTime object
* This function is not efficient
* If QVariant would include toStdString() it might help, aka: notesDatabase->value().toStdString
*/
QDateTime MainWindow::getQDateTime (QString date)
{
    return QDateTime::fromString(date, Qt::ISODate);
}

/**
* Get the full date of the selected note from the database and return it as a string in form for editorDateLabel
*/
QString MainWindow::getNoteDateEditor (QString dateEdited)
{
    QDateTime dateTimeEdited(getQDateTime(dateEdited));
    QLocale usLocale(QLocale("en_US"));

    return usLocale.toString(dateTimeEdited, "MMMM d, yyyy, h:mm A");
}

/**
* Get the full date of a note from database to present it in the right way in the notes list
*/
QString MainWindow::getNoteDate (QString dateEdited)
{
    QDateTime dateTimeEdited(getQDateTime(dateEdited));
    QLocale usLocale(QLocale("en_US"));

    if(dateTimeEdited.date() == QDate::currentDate()){
        return usLocale.toString(dateTimeEdited.time(),"h:mm A");
    }else if(dateTimeEdited.daysTo(QDateTime::currentDateTime()) == 1){
        return "Yesterday";
    }else if(dateTimeEdited.daysTo(QDateTime::currentDateTime()) >= 2 &&
             dateTimeEdited.daysTo(QDateTime::currentDateTime()) <= 7){
        return usLocale.toString(dateTimeEdited.date(), "dddd");
    }

    return dateTimeEdited.date().toString("M/d/yy");
}

/**
* Add a note from database or create a new note, into the notes list (in scrollArea)
* Determine if it's loading an existing note or creating a new note and present it accordingly
* Return a pointer to the new note
* isLoadingOrNew - > true = Loading a note from database
* isLoadingOrNew - > false = Creating a new note
*/
NoteData *MainWindow::addNote(QString noteName, bool isLoadingOrNew)
{
    NoteData* newNote = new NoteData(noteName, this);
    m_lay->insertWidget(0, newNote, 0, Qt::AlignTop);

    auto firstLineAndElide = isLoadingOrNew ? getFirstLineAndElide(newNote) : QPair<QString, QString>("New Note","New Note");
    QString noteDate = getNoteDate(m_notesDatabase->value(noteName + "/dateEdited", "Error").toString());
    newNote->m_fullTitle = firstLineAndElide.first;
    newNote->m_titleLabel->setText(firstLineAndElide.second);
    newNote->m_dateLabel->setText(noteDate);

    m_visibleNotesList.push_back(newNote);

    newNote->installEventFilter(this);

    connect(newNote, SIGNAL(pressed()), this, SLOT(onNotePressed()));
    connect(ui->splitter, SIGNAL(splitterMoved(int,int)),newNote, SLOT(onParentSizeChanged()));

    return newNote;
}

void MainWindow::showNoteInEditor(NoteData *note)
{
    ui->textEdit->blockSignals(true);
    ui->textEdit->setText(m_notesDatabase->value(note->m_noteName + "/content", "Error").toString());
    QString noteDate = m_notesDatabase->value(note->m_noteName + "/dateEdited", "Error").toString();
    ui->editorDateLabel->setText(getNoteDateEditor(noteDate));
    int tempScrollBarPosition = note->m_scrollBarPosition;
    ui->textEdit->verticalScrollBar()->setValue(tempScrollBarPosition);
    ui->textEdit->blockSignals(false);
}

/**
* Sort the notes that were loaded into the array 'stringNotesList'
* We are putting the data from stringNotesList into structer called QList
* So the std::stable_sort could manipulate on the data and sort it efficently
* We are sorting the notes by there last-edit-time from (oldest -> newest)
* So the oldest note will be placed at the bottom of the scrollArea and newest one at the top
* (adding notes from the vector happen left to right)
*/
void MainWindow::sortNotesList (QStringList &stringNotesList)
{
    m_notesDataForSorting.clear();

    for(int i = 0; i < stringNotesList.length() - 1; i += 3){
        noteDataForSorting tempNoteDataForSorting;

        tempNoteDataForSorting.m_noteName = stringNotesList[i].split("/")[0];

        QString dateDB = m_notesDatabase->value(tempNoteDataForSorting.m_noteName + "/dateEdited", "Error").toString();
        tempNoteDataForSorting.m_dateTime =  getQDateTime(dateDB);

        m_notesDataForSorting.push_back(tempNoteDataForSorting);
    }

    std::stable_sort(m_notesDataForSorting.begin(), m_notesDataForSorting.end());
}

/**
* Load all the notes from database into the notes list (in scrollArea)
*/
void MainWindow::loadNotes ()
{
    QStringList stringNotesList = m_notesDatabase->allKeys();

    sortNotesList(stringNotesList);

    QString noteName;
    NoteData* newNote;
    for(int i = 0; i < m_notesDataForSorting.length(); i++){
        noteName = m_notesDataForSorting.at(i).m_noteName;

        newNote = addNote(noteName, true);

        m_allNotesList.push_back(newNote);
    }

    if(stringNotesList.at(0) != "notesCounter")
        m_noteOnTopInTheLayoutName = m_notesDataForSorting.last().m_noteName;
}

/**
* Select the first note in the notes list
*/
void MainWindow::selectFirstNote ()
{
    if(!m_visibleNotesList.empty()){
        m_currentSelectedNote = m_visibleNotesList.back();
        highlightNote(m_currentSelectedNote, "rgb(254, 206, 9)");
        showNoteInEditor(m_currentSelectedNote);
    }
}

/**
* create a new note if there are no notes
*/
void MainWindow::createNewNoteIfEmpty ()
{
    if(m_allNotesList.empty())
        createNewNote();
}

/**
* Create a new note when clicking the 'new note' button
*/
void MainWindow::onNewNoteButtonClicked()
{
    createNewNote();
}

/**
* Delete selected note when clicking the 'delete note' button
*/
void MainWindow::onTrashButtonClicked()
{
    deleteSelectedNote();
}

/**
* Unhighlight the given note
*/
void MainWindow::unhighlightNote (NoteData* note)
{
    note->m_containerBox->setStyleSheet("");
    note->m_titleLabel->setStyleSheet("QLabel { color: black; }");
    note->m_dateLabel->setStyleSheet("QLabel { color: rgb(132, 132, 132); }");
}

/**
* Highlight the given note
*/
void MainWindow::highlightNote (NoteData* note, QString rgbStringColor)
{
    note->m_containerBox->setStyleSheet(QString(".QGroupBox { background-color: %1; }").arg(rgbStringColor));
    note->m_titleLabel->setStyleSheet(QString("QLabel { background-color: %1; color: black; }").arg(rgbStringColor));
    note->m_dateLabel->setStyleSheet(QString("QLabel { background-color: %1; color: rgb(132, 132, 132); }").arg(rgbStringColor));
}


/**
* Get a note and return it's animation property for deletion
*/
QPropertyAnimation* MainWindow::getAnimationForDeletion (NoteData *note)
{
    int tempY = note->m_fakeContainer->y();

    QPropertyAnimation* animation = new QPropertyAnimation(note->m_fakeContainer, "geometry");
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->setDuration(330);
    QRect geo = note->m_fakeContainer->geometry();
    animation->setStartValue(geo);
    geo.setY(tempY - note->m_fakeContainer->height()*2);
    animation->setEndValue(geo);

    return animation;
}

/**
* When clicking on a note in the scrollArea:
* Unhighlight the previous selected note
* If selecting a note when temporery note exist, delete the temp note
* Highlight the selected note
* Load the selected note content into textedit
* Set editorDateLabel text to the the selected note date
* And restore the scrollBar position if it changed before.
*/
void MainWindow::onNotePressed ()
{
    if(sender() != 0){
        if(m_currentSelectedNote != 0 && m_currentSelectedNote != m_tempNote)
            unhighlightNote(m_currentSelectedNote);

        NoteData* note = qobject_cast<NoteData *>(sender());
        m_currentSelectedNote = note;

        if(m_tempNote != 0 && m_currentSelectedNote != m_tempNote){
            QPropertyAnimation* animation = getAnimationForDeletion(m_tempNote);

            connect(animation, SIGNAL(finished()), this, SLOT(deleteTempNoteFromVisual()));
            animation->start();
            deleteNoteFromDataBase(m_tempNote);
        }

        highlightNote(m_currentSelectedNote, "rgb(254, 206, 9)");
        showNoteInEditor(m_currentSelectedNote);
        m_currentSelectedNote->setFocus();

    }
}

/**
* When clearButton in the search box (lineEdit) is pressed,
* clear the text in lineEdit and the date label in the editor.
*/
void MainWindow::clearButtonClicked ()
{
    ui->lineEdit->clear();
}

/**
* Delete a given note from the database
*/
void MainWindow::deleteNoteFromDataBase (NoteData *note)
{
    for(unsigned int i = 0; i < m_allNotesList.size(); i++){
        if(m_allNotesList.at(i)->m_noteName == note->m_noteName){
            m_allNotesList.erase(m_allNotesList.begin() + i);
        }
    }

    // Putting the deleted note in trash
    int counter = m_trashDatabase->value("notesCounter").toInt() + 1;
    m_trashDatabase->setValue("notesCounter", counter);

    QString noteName = QString("noteID_%1").arg(counter);
    m_trashDatabase->setValue(noteName + "/content",  m_notesDatabase->value(note->m_noteName + "/content", "Error"));

    QString dateDBCreated = m_notesDatabase->value(note->m_noteName + "/dateEdited", "Error").toString();
    QString dateDBEdited = m_notesDatabase->value(note->m_noteName + "/dateEdited", "Error").toString();
    QString noteDate = QDateTime::currentDateTime().toString(Qt::ISODate);

    m_trashDatabase->setValue(noteName + "/dateCreated", dateDBCreated);
    m_trashDatabase->setValue(noteName + "/dateEdited", dateDBEdited);
    m_trashDatabase->setValue(noteName + "/dateTrashed", noteDate);

    m_notesDatabase->remove(note->m_noteName);

    m_notesDatabase->sync();
    m_trashDatabase->sync();
}

/**
* Remove a given note from the interface
*/
void MainWindow::deleteNoteFromVisual (NoteData *note)
{
    m_visibleNotesList.removeOne(note);
    m_lay->removeWidget(note);
    delete note;
}

/**
* When the text on textEdit change:
* Save the changes to database (auto-saving)
* if the note edited is not on top of the list, we will make that happen
* If the text changed is of a new (empty) note, reset temp values
* (We have those valuse to make sure to delete the note from database, if the program is closed and the new note is empty)
*/
void MainWindow::onTextEditTextChanged ()
{
    //if(text has really changed)
    if(m_currentSelectedNote != 0 && ui->textEdit->toPlainText() != m_notesDatabase->value(m_currentSelectedNote->m_noteName + "/content", "Error")){
        m_notesDatabase->setValue(m_currentSelectedNote->m_noteName + "/content", ui->textEdit->toPlainText());

        auto firstLineAndElide = getFirstLineAndElide(m_currentSelectedNote);
        m_currentSelectedNote->m_fullTitle = firstLineAndElide.first;
        m_currentSelectedNote->m_titleLabel->setText(firstLineAndElide.second);

        QString noteDate = QDateTime::currentDateTime().toString(Qt::ISODate);
        m_notesDatabase->setValue(m_currentSelectedNote->m_noteName + "/dateEdited", noteDate);
        m_currentSelectedNote->m_dateLabel->setText(getNoteDate(noteDate));
        ui->editorDateLabel->setText(getNoteDateEditor(noteDate));

        //notesDatabase->sync(); // We may want to remove that

        if(m_currentSelectedNote->m_noteName != m_noteOnTopInTheLayoutName){
            m_noteOnTopInTheLayoutName = m_currentSelectedNote->m_noteName;

            int tempHeight = m_currentSelectedNote->height();

            // Animation for removing
            QPropertyAnimation *removeAnimation = new QPropertyAnimation(m_currentSelectedNote, "geometry");
            removeAnimation->setEasingCurve(QEasingCurve::OutQuint);
            removeAnimation->setDuration(165);
            QRect removeGeo = m_currentSelectedNote->geometry();
            removeAnimation->setStartValue(removeGeo);
            removeGeo.setHeight(0);
            removeAnimation->setEndValue(removeGeo);

            // animation for inserting
            QPropertyAnimation *insertAnimation = new QPropertyAnimation(m_currentSelectedNote, "geometry");
            insertAnimation->setEasingCurve(QEasingCurve::OutQuint);
            insertAnimation->setDuration(165);
            QRect insertGeo = m_currentSelectedNote->geometry();
            insertGeo.setHeight(0);
            insertGeo.setY(0);
            insertAnimation->setStartValue(insertGeo);
            insertGeo.setHeight(tempHeight);
            insertAnimation->setEndValue(insertGeo);

            // start the inserting animation after removing annimation finishes
            connect(removeAnimation, &QPropertyAnimation::finished, this, [this, insertAnimation](){
                // scroll to top
                int scrollBarMinValue = ui->scrollArea->verticalScrollBar()->minimum();
                ui->scrollArea->verticalScrollBar()->setValue(scrollBarMinValue);
                // move the current selected note to the top (m_allNotesList: top = back)
                m_lay->removeWidget(m_currentSelectedNote);
                m_lay->insertWidget(0,m_currentSelectedNote);
                m_allNotesList.move(m_allNotesList.indexOf(m_currentSelectedNote), m_allNotesList.count()-1);
                // start insert animation
                insertAnimation->start(QAbstractAnimation::DeleteWhenStopped);
            });

            // start the remove animation
            removeAnimation->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }

    if(m_isTemp && !m_notesDatabase->value(m_tempNote->m_noteName + "/content", "Error").toString().isEmpty()){
        m_tempNote = 0;
        m_isTemp = false;
    }
}

/**
* Return true if string 'keyword' is found in string 'content', else return false
*/
bool MainWindow::isFound (QString keyword, QString content)
{
    return content.contains(keyword, Qt::CaseInsensitive);
}

/**
* Unhighlight the given note without modifing other notes
*/
void MainWindow::simpleUnhighlightNote (NoteData *note)
{
    note->m_seperateLine->show();
    note->m_containerBox->setStyleSheet("");
    note->m_titleLabel->setStyleSheet("QLabel { color: black; }");
    note->m_dateLabel->setStyleSheet("QLabel { color: rgb(132, 132, 132); }");
}

/**
* Remove all the notes from the scrollArea
*/
void MainWindow::clearAllNotesFromVisual ()
{
    foreach (NoteData *note, m_visibleNotesList) {
        simpleUnhighlightNote(note);
        note->hide();
    }

    m_visibleNotesList.clear();
    m_currentSelectedNote = 0;
}

/**
* Given a note name name, select the given note
* and set the scrollArea's scrollBar position to it
* return true if the given note was found, else return false
*/
bool MainWindow::goToAndSelectNote (NoteData* note)
{
    // Is there a better way? (We are doing this because the value of the scrollBar won't change unitl all notes in GUI are loaded)
    QCoreApplication::processEvents(QEventLoop::AllEvents);

    bool found = false;

    int noteIndex = m_visibleNotesList.indexOf(note);
    if(noteIndex != -1){
        found = true;
        note->pressed();
        unsigned int noteSize = note->height();
        ui->scrollArea->verticalScrollBar()->setValue((m_visibleNotesList.size()-1 - noteIndex) * noteSize);
    }

    return found;
}

/**
* When text on lineEdit change:
* If there is a temp note "New Note" while searching, we delete it
* Saving the last selected note for recovery after searching
* Make the clear button visible when typing in the search box, or invisble when empty
* Clear all the notes from scrollArea and
* If text is empty, reload all the notes from database
* Else, load all the notes contain the string in lineEdit from database
*/
void MainWindow::onLineEditTextChanged (const QString &arg1)
{

    if(m_tempNote != 0){
        deleteNoteFromDataBase(m_tempNote);
        deleteNoteFromVisual(m_tempNote);
        m_tempNote = 0;
        m_isTemp = false;
    }

    if(m_currentSelectedNote != 0)
        m_tempSelectedNoteBeforeSearching = m_currentSelectedNote;

    clearAllNotesFromVisual();

    if(arg1.isEmpty()){

        foreach(NoteData* note, m_allNotesList){
            note->show();
            m_visibleNotesList.push_back(note);
        }

        bool found = goToAndSelectNote(m_tempSelectedNoteBeforeSearching);

        if(!found)
            selectFirstNote();

        m_tempSelectedNoteBeforeSearching = 0;

    }else{

        ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->minimum());

        QString noteName;
        foreach (NoteData *note, m_allNotesList) {
            noteName = note->m_noteName;
            if(isFound(arg1, m_notesDatabase->value(noteName + "/content", "Error").toString())){
                note->show();
                m_visibleNotesList.push_back(note);
            }
        }

        if(m_visibleNotesList.empty()){
            ui->textEdit->blockSignals(true);
            ui->textEdit->clear();
            ui->editorDateLabel->clear();
            ui->textEdit->blockSignals(false);
        }

        selectFirstNote();

        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
}

/**
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
* Making the new note animation
* I have noticed somethings wierd:
* when the animation accour (new note is created) and the scrollArea's scrollBar is showing,
* Something wierd is going on at the start of the animation
* Yet if you click again on the 'newNoteButton' everything is fine
*/
void MainWindow::newNoteAnimation ()
{
    int tempY = m_currentSelectedNote->m_fakeContainer->y() - m_currentSelectedNote->m_fakeContainer->height() * 2;

    QPropertyAnimation *animation = new QPropertyAnimation(m_currentSelectedNote->m_fakeContainer, "geometry");
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->setDuration(330);

    QRect geo = m_currentSelectedNote->m_fakeContainer->geometry();
    geo.setY(tempY);
    animation->setStartValue(geo);
    geo.setY(0);

    animation->setEndValue(geo);
    animation->start();
}

/**
* When Creating a new note:
* Put all the note data first to the database
* And then present it visualy in the scrollArea
* Triger the temp values, until note is edited
* Unhighlight the previous selected note (if there is one)
* Highlight the new note
* Set scrollBar value to 0 (top), clear textEdit and set focus to it
* Set editorDateLabel text to the the new note date
* Take care of a beautiful animation to give sense of forming
* (We have a problem with that ^ look at the code for info)
*/
void MainWindow::createNewNote ()
{
    if(!m_isTemp){
        if(!ui->lineEdit->text().isEmpty())
            ui->lineEdit->clear();

        QString noteName = createNewNoteInDatabase();
        m_tempNote = addNote(noteName, false);
        m_allNotesList.push_back(m_tempNote);
        m_noteOnTopInTheLayoutName = noteName;
        m_isTemp = true;

        if(m_currentSelectedNote != 0)
            unhighlightNote(m_currentSelectedNote);

        m_currentSelectedNote = m_tempNote;
        highlightNote(m_currentSelectedNote, "rgb(255, 235, 80)");

        ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->minimum());
        ui->editorDateLabel->setText(getNoteDateEditor(m_notesDatabase->value(noteName + "/dateEdited", "Error").toString()));
        ui->textEdit->blockSignals(true);
        ui->textEdit->clear();
        ui->textEdit->setFocus();
        ui->textEdit->blockSignals(false);

        QCoreApplication::processEvents();

        newNoteAnimation();

    }else{
        ui->textEdit->blockSignals(true);
        ui->textEdit->setFocus();
        ui->textEdit->blockSignals(false);
        ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->minimum());

        newNoteAnimation();
    }
}

/**
* Take care of all the visual things around note deletion:
* Delete the selected note from the interface
* And selecting another note (if there is one)
* If the last note is deleted, select the note above
* else, select the note under it
*/
void MainWindow::deleteSelectedNoteFromVisual ()
{

   int tempNotePlaceInLayout = m_visibleNotesList.indexOf(m_currentSelectedNote);

    deleteNoteFromVisual(m_currentSelectedNote);
    m_currentSelectedNote = 0;

    ui->textEdit->blockSignals(true);
    ui->textEdit->clearFocus();
    ui->textEdit->blockSignals(false);

    if(m_visibleNotesList.size() > 0){
        m_currentSelectedNote = tempNotePlaceInLayout == 0 ? m_visibleNotesList.at(tempNotePlaceInLayout)
                                                           : m_visibleNotesList.at(tempNotePlaceInLayout-1);
        highlightNote(m_currentSelectedNote, "rgb(254, 206, 9)");
        showNoteInEditor(m_currentSelectedNote);
    }
}

/**
* Delete the temporary note (a new note that was created but wasn't edited)
*/
void MainWindow::deleteTempNoteFromVisual ()
{
    deleteNoteFromVisual(m_tempNote);

    m_tempNote = 0;
    m_isTemp = false;
}

/**
* Delete the selected note:
* If a note is selected:
* Remove it from database
* Remove it from visual
* Clear the date label above the editor
* If the note is temp and has not been edited, reset temp values
* Activate animation
*/
void MainWindow::deleteSelectedNote ()
{
    if(m_currentSelectedNote != 0){
        if(m_tempNote != 0 && m_currentSelectedNote->m_noteName == m_tempNote->m_noteName){
            m_tempNote = 0;
            m_isTemp = false;
        }

        QPropertyAnimation *animation = getAnimationForDeletion(m_currentSelectedNote);

        //Why can't Qt let pass data throught SLOT?
        connect(animation, SIGNAL(finished()), this, SLOT(deleteSelectedNoteFromVisual()));

        animation->start();

        deleteNoteFromDataBase(m_currentSelectedNote);

        ui->textEdit->blockSignals(true);
        ui->editorDateLabel->clear();
        ui->textEdit->clear();
        ui->textEdit->clearFocus();
        ui->textEdit->blockSignals(false);
    }
}

/**
* Set focus on textEdit
*/
void MainWindow::setFocusOnText ()
{
    if(m_currentSelectedNote != 0 && !ui->textEdit->hasFocus())
        ui->textEdit->setFocus();
}

/**
* Set focus on scrollArea and highlight the currentSelectedNote
*/
void MainWindow::setFocusOnScrollArea ()
{
    ui->scrollArea->setFocus();

    if(m_currentSelectedNote != 0)
        highlightNote(m_currentSelectedNote, "rgb(254, 206, 9)");
}

/**
* Select the note above the currentSelectedNote
*/
void MainWindow::selectNoteUp ()
{
    if(m_currentSelectedNote != 0){
        if(!ui->scrollArea->hasFocus())
            ui->scrollArea->setFocus();

        for(unsigned int i = 0; i < m_visibleNotesList.size() - 1; i++){
            if(m_visibleNotesList.at(i)->m_fakeContainer  == m_currentSelectedNote->m_fakeContainer){
                unsigned int noteSize = m_currentSelectedNote->m_fakeContainer->height();

                if((m_visibleNotesList.size() - i - 1) * noteSize < ui->scrollArea->verticalScrollBar()->value() + noteSize)
                    ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value() - noteSize);

                m_visibleNotesList.at(i + 1)->m_button->pressed();
                break;
            }
        }
    }
}

/**
* Select the note under the currentSelectedNote
*/
void MainWindow::selectNoteDown ()
{
    if(m_currentSelectedNote != 0){
        if(!ui->scrollArea->hasFocus())
            ui->scrollArea->setFocus();

        for(unsigned int i = 1; i < m_visibleNotesList.size(); i++){
            if(m_visibleNotesList.at(i)->m_fakeContainer  == m_currentSelectedNote->m_fakeContainer){
                unsigned int noteSize = m_currentSelectedNote->m_fakeContainer->height();

                if((m_visibleNotesList.size() - i - 1) * noteSize > ui->scrollArea->verticalScrollBar()->value() + ui->scrollArea->height() - noteSize * 2)
                    ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value() + noteSize);

                m_visibleNotesList.at(i - 1)->m_button->pressed();
                break;
            }
        }
    }
}

/**
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
* Minimize the window
*/
void MainWindow::minimizeWindow ()
{
    this->setWindowState(Qt::WindowMinimized);
    this->showNormal(); // I don't know why, but it's need to be here
}

/**
* Exit the application
* If a new note created but wasn't edited, delete it from the database
*/
void MainWindow::QuitApplication ()
{
    if(m_tempNote != 0){
        ui->textEdit->blockSignals(true);
        deleteNoteFromDataBase(m_tempNote);
    }

    //QApplication::quit();
    MainWindow::close();
}

/**
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
* When the yellow button is pressed set it's icon accordingly
*/
void MainWindow::onYellowMinimizeButtonPressed ()
{
    ui->yellowMinimizeButton->setIcon(QIcon(":images/yellowPressed.png"));
}

/**
* When the red button is pressed set it's icon accordingly
*/
void MainWindow::onRedCloseButtonPressed ()
{
    ui->redCloseButton->setIcon(QIcon(":images/redPressed.png"));
}

/**
* When the green button is released the window goes fullscrren
*/
void MainWindow::onGreenMaximizeButtonClicked()
{
    ui->greenMaximizeButton->setIcon(QIcon(":images/green.png"));

    fullscreenWindow();
}

/**
* When yellow button is released the window is minimized
*/
void MainWindow::onYellowMinimizeButtonClicked()
{
    ui->yellowMinimizeButton->setIcon(QIcon(":images/yellow.png"));

    minimizeWindow();
}

/**
* When red button is released the window get closed
* If a new note created but wasn't edited, delete it from the database
*/
void MainWindow::onRedCloseButtonClicked()
{
    ui->redCloseButton->setIcon(QIcon(":images/red.png"));

    QuitApplication();
}

/**
* Get called everytime the textEdit's scrollBar value change
* Gets called to save the value of the scrollBar in the currentSelectedNote
* for recovering the scrollBar position when browsing different notes
*/
void MainWindow::textEditScrollBarValueChange (int verticalScrollBarValue)
{
    if(m_currentSelectedNote != 0)
        m_currentSelectedNote->m_scrollBarPosition = verticalScrollBarValue;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(windowState() != Qt::WindowFullScreen && windowState() != Qt::WindowMaximized)
        m_settingsDatabase->setValue("windowGeometry", saveGeometry());

    m_settingsDatabase->setValue("splitterSizes", ui->splitter->saveState());

    QWidget::closeEvent(event);
}

/**
* Set variables to the position of the window when the mouse is pressed
* And set variables for resizing
*/
void MainWindow::mousePressEvent (QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton){
        if(event->pos().x() < ui->centralWidget->geometry().width() && event->pos().y() < ui->textEdit->y()){
            m_canMoveWindow = true;
            m_nMouseClick_X_Coordinate = event->x();
            m_nMouseClick_Y_Coordinate = event->y();
        }
    }

    event->accept();
}

/**
* Move the window according to the mouse positions
* And resizing
*/
void MainWindow::mouseMoveEvent (QMouseEvent* event)
{
    if(m_canMoveWindow){
        this->setCursor(Qt::ClosedHandCursor);
        move (event->globalX() - m_nMouseClick_X_Coordinate, event->globalY() - m_nMouseClick_Y_Coordinate);
    }
}

/**
* Set some variables after releasing the mouse for moving the window and resizing
*/
void MainWindow::mouseReleaseEvent (QMouseEvent *event)
{
    m_canMoveWindow = false;
    this->unsetCursor();
    event->accept();
}

bool MainWindow::isClickingButton (QPoint mousePos, QPushButton* button)
{
    if(mousePos.y() > button->y() &&
            mousePos.y() < button->y() + button->height() &&
            mousePos.x() > button->x() &&
            mousePos.x() < button->x() + button->width())
        return true;

    return false;
}

/**
* When the blank area at the top of window is double-clicked the window get maximized
*/
void MainWindow::mouseDoubleClickEvent (QMouseEvent *e)
{
    if(e->pos().y() < ui->textEdit->y() &&
            !isClickingButton(e->pos(), ui->newNoteButton) &&
            !isClickingButton(e->pos(), ui->trashButton))
        maximizeWindow();

    e->accept();
}

void MainWindow::InitData()
{
    loadNotes();
    selectFirstNote();
    createNewNoteIfEmpty();
}

/**
* return true if the given spacer item is inside the given layout, else retuen false
*/
bool MainWindow::isSpacerInsideLayout(QSpacerItem *spacer, QVBoxLayout *layout)
{
    for(int i = 0; i < layout->count(); i++){
        if(layout->itemAt(i)->spacerItem() == spacer)
            return true;
    }

    return false;
}

/**
* Mostly take care on what happens when hovering/unhovering some widgets
*/
bool MainWindow::eventFilter (QObject *object, QEvent *event)
{
    if(event->type() == QEvent::Enter){
        // When hovering one of the traffic light buttons (red, yellow, green),
        // set new icons to show their function
        if(object == ui->redCloseButton || object == ui->yellowMinimizeButton || object == ui->greenMaximizeButton){
            ui->redCloseButton->setIcon(QIcon(":images/redHovered.png"));
            ui->yellowMinimizeButton->setIcon(QIcon(":images/yellowHovered.png"));
            if(this->windowState() == Qt::WindowFullScreen){
                ui->greenMaximizeButton->setIcon(QIcon(":images/greenInHovered.png"));
            }else{
                ui->greenMaximizeButton->setIcon(QIcon(":images/greenHovered.png"));
            }
        }

        // When hovering upon note with the mouse, highlight it
        if(QString(object->metaObject()->className()) == "NoteData"){
            m_currentHoveredNote = qobject_cast<NoteData *>(object);
            if((m_currentSelectedNote != 0 && m_currentHoveredNote != m_currentSelectedNote) || m_currentSelectedNote == 0)
                highlightNote(m_currentHoveredNote, "rgb(207, 207, 207)");
        }

        // When hovering upon textEdit's scrollBar, show its border
        if(object == ui->textEdit->verticalScrollBar()){
            if(!ui->lineEdit->text().isEmpty() && m_currentSelectedNote != 0)
                m_focusBreaker = true;
        }
    }

    if(event->type() == QEvent::Leave){
        // When not hivering, change back the icons of the traffic lights to their default icon
        if(object == ui->redCloseButton || object == ui->yellowMinimizeButton || object == ui->greenMaximizeButton){
            ui->redCloseButton->setIcon(QIcon(":images/red.png"));
            ui->yellowMinimizeButton->setIcon(QIcon(":images/yellow.png"));
            ui->greenMaximizeButton->setIcon(QIcon(":images/green.png"));
        }

        // When getting out of a highlighted note, unghighlight it
        if(QString(object->metaObject()->className()) == "NoteData"){
            NoteData* note = qobject_cast<NoteData *>(object);

            if(m_currentHoveredNote != 0 && m_currentHoveredNote == note){
                if((m_currentSelectedNote != 0 && m_currentHoveredNote != m_currentSelectedNote) || m_currentSelectedNote == 0){

                    unhighlightNote(m_currentHoveredNote);
                }
            }
        }

        // When not hovering the textEdit's scrollBar, change back to its default style
        if(object == ui->textEdit->verticalScrollBar()){

            m_focusBreaker = false;
            if(!ui->lineEdit->text().isEmpty() && m_currentSelectedNote != 0)
                ui->textEdit->clearFocus();

        }
    }

    if(event->type() == QEvent::FocusIn){
        if(object == ui->textEdit){
            // If there are no notes, and the user click the textEdit, create a new note
            if(m_currentSelectedNote == 0){
                createNewNote();
            }else{
                highlightNote(m_currentSelectedNote, "rgb(255, 235, 80)");
            }


            // When clicking in a note's content while searching,
            // reload all the notes and go and select that note
            if(!m_focusBreaker && !ui->lineEdit->text().isEmpty() && m_currentSelectedNote != 0){
                ui->lineEdit->clear();

                goToAndSelectNote(m_currentSelectedNote);

                highlightNote(m_currentSelectedNote, "rgb(255, 235, 80)");
                ui->textEdit->setFocus();
            }
        }
    }

    if(event->type() == QEvent::FocusOut){
        // If the scrollArea is out of focus change the highligted color of currentSelectedNote
        if(object == m_currentSelectedNote){
            if(m_currentSelectedNote != 0)
                highlightNote(m_currentSelectedNote, "rgb(255, 235, 80)");
        }
    }

    return false;
}

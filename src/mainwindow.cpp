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

#define FIRST_LINE_MAX 80

/**
* Setting up the main window and it's content
*/
MainWindow::MainWindow (QWidget *parent) :
    QMainWindow (parent),
    ui (new Ui::MainWindow)
{
    ui->setupUi(this);

    SetUpMainWindow();

    SetUpKeyboardShortcuts();

    SetUpNewNoteButtonAndTrahButton();

    SetUpEditorDateLabel();

    SetUpLine();

    SetUpFrame ();

    SetUpTitleBarButtons();

    CreateClearButton();

    CreateMagnifyingGlassIcon();

    SetUpLineEdit();

    SetUpScrollArea();

    SetUpTextEdit();

    InitializeVariables();

    SetUpDatabase();

    SetUpOffsets();

    RestoreGeometry();

    SetLayoutForScrollArea();

    LoadNotes();

    SelectFirstNote();
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
void MainWindow::SetUpMainWindow ()
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

    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(Pal);

    ui->newNoteButton->setToolTip("Create New Note");
    ui->trashButton->setToolTip("Delete Selected Note");
}

/**
* Setting up the keyboard shortcuts
*/
void MainWindow::SetUpKeyboardShortcuts ()
{
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_N), this, SLOT(Create_new_note()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Delete), this, SLOT(DeleteSelectedNote()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), ui->lineEdit, SLOT(setFocus()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_E), ui->lineEdit, SLOT(clear()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this, SLOT(SetFocusOnScrollArea()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down), this, SLOT(SelectNoteDown()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Up), this, SLOT(SelectNoteUp()));
    new QShortcut(QKeySequence(Qt::Key_Down), this, SLOT(SelectNoteDown()));
    new QShortcut(QKeySequence(Qt::Key_Up), this, SLOT(SelectNoteUp()));
    new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(SetFocusOnText()));
    new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(SetFocusOnText()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F), this, SLOT(FullscreenWindow()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L), this, SLOT(MaximizeWindow()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_M), this, SLOT(MinimizeWindow()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(QuitApplication()));
}

/**
* We need to set up some different values when using apple os x
* This is because if we want to get the native button look in os x,
* due to some bug in Qt, I think, the values of width and height of buttons
* needs to be more than 50 and less than 34 respectively.
* So some modifications needs to be done.
*/
void MainWindow::SetUpNewNoteButtonAndTrahButton ()
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
void MainWindow::SetUpEditorDateLabel()
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
* Set up the vertical line that seperate between the scrollArea to the textEdit
*/
void MainWindow::SetUpLine ()
{
    ui->line->setStyleSheet("border: 1px solid rgb(221, 221, 221)");
    ui->line_2->setStyleSheet("border: 1px solid rgb(221, 221, 221)");
    ui->line_3->setStyleSheet("border: 1px solid rgb(221, 221, 221)");
}

/**
* Set up a frame above textEdit and behind the other widgets for a unifed background in thet editor section
*/
void MainWindow::SetUpFrame ()
{
    frame = new QFrame(this);
    frame->setStyleSheet("QFrame { background-image: url(:/images/textSideBackground.png); border: none;}");
    frame->lower();
}

/**
* Setting up the red (close), yellow (minimize), and green (maximize) buttons
* Make only the buttons icon visible
* And install this class event filter to them, to act when hovering on one of them
*/
void MainWindow::SetUpTitleBarButtons ()
{
    ui->redCloseButton->setStyleSheet("QPushButton { border: none; padding: 0px; }");
    ui->yellowMinimizeButton->setStyleSheet("QPushButton { border: none; padding: 0px; }");
    ui->greenMaximizeButton->setStyleSheet("QPushButton { border: none; padding: 0px; }");

    ui->redCloseButton->installEventFilter(this);
    ui->yellowMinimizeButton->installEventFilter(this);
    ui->greenMaximizeButton->installEventFilter(this);

}

/**
* Setting up the clear button in the search box (lineEdit)
*/
void MainWindow::CreateClearButton ()
{
    clearButton = new QToolButton(ui->lineEdit);

    QPixmap pixmap(":/images/closeButton.gif");
    clearButton->setIcon(QIcon(pixmap));
    clearButton->setIconSize(QSize(pixmap.size().width()-5, pixmap.size().height()-5));
    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    clearButton->hide();
    QSize sz = clearButton->sizeHint();
    int frameWidth = ui->lineEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    clearButton->move(ui->lineEdit->rect().right() - frameWidth - sz.width(), (ui->lineEdit->rect().bottom() + 1 - sz.height())/2);

    connect(clearButton, SIGNAL(clicked()), this, SLOT(ClearButtonClicked()));
}

/**
* Setting up the magnifing glass icon in the search box (lineEdit)
*/
void MainWindow::CreateMagnifyingGlassIcon ()
{
    QToolButton *searchButton = new QToolButton(ui->lineEdit);

    QPixmap newPixmap(":/images/magnifyingGlass.png");
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
void MainWindow::SetUpLineEdit ()
{
    // There is a problem with Helvetica here so we usa Arial, someone sguggestion?
    #ifdef __APPLE__
        ui->lineEdit->setFont(QFont("Arial", 12));
    #endif

    int frameWidth = ui->lineEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    ui->lineEdit->setStyleSheet(QString("QLineEdit { padding-right: %1px; padding-left: 19px } ") // border-radius: 3px; border: 1px solid rgb(173, 169, 165);
                                .arg(clearButton->sizeHint().width() + frameWidth + 1));
}

/**
* Setting up the scrollArea widget:
* Setup the style of the scrollBar
* Disable the horizontal scrolling in scrollArea
* And install this class event filter to act when scrollArea is out of focus and when hovering the scrollBar
*/
void MainWindow::SetUpScrollArea ()
{
    #ifdef __APPLE__
        ui->scrollArea->setGeometry(ui->scrollArea->x() + 1, ui->scrollArea->y(), ui->scrollArea->width() - 1, ui->scrollArea->height());
    #endif

    ui->scrollArea->setStyleSheet("QScrollBar:handle:vertical:hover { background: rgb(170, 170, 171); } QScrollBar:handle:vertical:pressed { background: rgb(149, 149, 149); } * { background-color: rgb(255, 255, 255);  } QScrollBar:vertical {border: none; width: 8px; } QScrollBar::handle:vertical { border-radius: 2.5px; background: rgb(188, 188, 188); min-height: 20px; }  QScrollBar::add-line:vertical { height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  QScrollBar::sub-line:vertical { height: 0px; subcontrol-position: top; subcontrol-origin: margin; }");
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->scrollArea->installEventFilter(this);
    ui->scrollArea->verticalScrollBar()->installEventFilter(this);

    connect(ui->scrollArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(ScrollAreaScrollBarRangeChange(int,int)));
}

/**
* Setting up textEdit:
* Setup the style of the scrollBar and set textEdit background to an image
* Make the textEdit pedding few pixels right and left, to compel with a beautiful proportional grid
* And install this class event filter to act when scrollArea is out of focus and when hovering the scrollBar
*/
void MainWindow::SetUpTextEdit ()
{
    //? ui->textEdit->setTextColor(QColor::fromRgb(25, 25, 25));

    #ifdef Q_OS_LINUX
        textEditLeftPadding = 5;
    #elif _WIN32
        textEditLeftPadding = 5;
    #elif __APPLE__
        textEditLeftPadding = 18;
    #else
        #error "We don't support that version yet..."
    #endif

    ui->textEdit->setStyleSheet(QString("QScrollBar:handle:vertical:hover { background: rgb(170, 170, 171); } QScrollBar:handle:vertical:pressed { background: rgb(149, 149, 149); } QTextEdit { background-image: url(:/images/textSideBackground.png); padding-left: %1px; padding-right: %2px; } QScrollBar:vertical { border: none; width: 8px; } QScrollBar::handle:vertical { border-radius: 2.5px; background: rgb(188, 188, 188); min-height: 20px; }  QScrollBar::add-line:vertical { height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  QScrollBar::sub-line:vertical { height: 0px; subcontrol-position: top; subcontrol-origin: margin; }")
                                .arg(QString::number(ui->newNoteButton->width() - textEditLeftPadding), "27"));

    ui->textEdit->installEventFilter(this);
    ui->textEdit->verticalScrollBar()->installEventFilter(this);

    connect(ui->textEdit->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(TextEditScrollBarRangeChange(int,int)));
    connect(ui->textEdit->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(TextEditScrollBarValueChange(int)));

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
void MainWindow::InitializeVariables ()
{
    currentSelectedNote = 0;
    currentHoveredNote = 0;
    tempNote = 0;
    isTemp = false;
    currentVerticalScrollAreaRange = 0;
    canMoveWindow = false;
    focusBreaker = false;
}

void MainWindow::InitializeSettingsDatabase()
{
    if(settingsDatabase->value("version", "NULL") == "NULL")
    {
        settingsDatabase->setValue("version", "0.8.0");
    }

    if(settingsDatabase->value("defaultWindowWidth", "NULL") == "NULL")
    {
        settingsDatabase->setValue("defaultWindowWidth", 757);
    }

    if(settingsDatabase->value("defaultWindowHeight", "NULL") == "NULL")
    {
        settingsDatabase->setValue("defaultWindowHeight", 341);
    }

    if(settingsDatabase->value("geometry", "NULL") == "NULL")
    {
        settingsDatabase->setValue("geometry", saveGeometry());
    }
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
void MainWindow::SetUpDatabase ()
{
    notesDatabase = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Awesomeness", "Notes");
    notesDatabase->setFallbacksEnabled(false);

    if(notesDatabase->value("notesCounter", "NULL") == "NULL")
    {
        notesDatabase->setValue("notesCounter", 0);
    }

    trashDatabase = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Awesomeness", "Trash");
    trashDatabase->setFallbacksEnabled(false);

    if(trashDatabase->value("notesCounter", "NULL") == "NULL")
    {
        trashDatabase->setValue("notesCounter", 0);
    }

    settingsDatabase = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Awesomeness", "Settings");
    settingsDatabase->setFallbacksEnabled(false);

    InitializeSettingsDatabase();

    notesDatabase->sync();
    trashDatabase->sync();
}

/**
* Restore the latest data of the position (x, y) and size of the window (width, height)
* from database
*/
void MainWindow::RestoreGeometry()
{
    this->restoreGeometry(settingsDatabase->value("geometry").toByteArray());
}

/**
* The layout we use to store the list of notes (made by groupboxes, button, frame and labels) inside ScrollArea
*/
void MainWindow::SetLayoutForScrollArea ()
{
    lay = new QVBoxLayout();

    lay->addStretch();
    lay->setSpacing(0);
    lay->setMargin(0);
    lay->setContentsMargins(0,0,0,0);

    ui->scrollAreaWidgetContents->setLayout(lay);
}

/**
* Saves The offset of some widgets for resizing
*/
void MainWindow::SetUpOffsets ()
{
    scrollAreaOffset = MainWindow::height() - ui->scrollArea->height();
    textEditOffset1 = MainWindow::width() - ui->textEdit->width();
    textEditOffset2 = MainWindow::height() - ui->textEdit->height();
    trashButtonOffset = MainWindow::width() - ui->trashButton->x();
}

/**
* Get a string 'str' and return only the first line of it
* If the string contain no text, return "New Note"
* TODO: We might make it more efficient by not loading the entire string into the memory
*/
QString MainWindow::GetFirstLine (QString str)
{
    if(str.simplified().isEmpty())
    {
        return "New Note";
    }

    int indexToStart = 0;
    for(int i = 0; i < str.length(); i++)
    {
        if(str.at(i) != '\n' && str.at(i) != '\r')
        {
            indexToStart = i;
            break;
        }
    }

    int indexToFinish = FIRST_LINE_MAX;
    for(int i = indexToStart; i < str.length(); i++)
    {
        if(str.at(i) == '\n' || str.at(i) == '\r')
        {
            indexToFinish = i;
            break;
        }
    }

    QString firstLine = str.mid(indexToStart, indexToFinish-indexToStart);

    return firstLine;
}

/**
* Get a string 'str' and return it's short form with "..." if it is wider than the label present it (titleLabel)
* Example: ("This string is really really long") -> "This string is rea..."
*/
QString MainWindow::GetElidedText (QString str, QFontMetrics labelFontMetrics, int size)
{
    QString croppedText = labelFontMetrics.elidedText(str, Qt::ElideRight, size);

    if(croppedText.right(2) == " …") // Using ellipsis character  // Old: if(labelFontMetrics.width(str) >= size && croppedText.at(croppedText.length()-2) == ' ')
    {
        croppedText.replace(croppedText.length()-2, 1, '.');
    }

    return croppedText;
}

/**
* Get a note's noteData and return its first line after eliding it.
*/
QString MainWindow::GetFirstLineAndElide (noteData* note)
{
    // We can improve more if we could load right from the Hard Drive database only one line and not the whole text
    QString text = notesDatabase->value(note->noteName + "/content", "Error").toString();

    QString firstLine = GetFirstLine(text);

    firstLine = GetElidedText(firstLine, note->titleLabel->fontMetrics(), note->titleLabel->width());

    return firstLine;
}

/**
* Get a date string of a note from database and put it's data into a QDateTime object
* This function is not efficient
* If QVariant would include toStdString() it might help, aka: notesDatabase->value().toStdString
*/
QDateTime MainWindow::GetQDateTime (QString date)
{
    return QDateTime::fromString(date, Qt::ISODate);
}

/**
* Get the full date of the selected note from the database and return it as a string in form for editorDateLabel
*/
QString MainWindow::GetNoteDateEditor (QString dateEdited)
{
    QDateTime dateTimeEdited(GetQDateTime(dateEdited));
    QLocale usLocale(QLocale("en_US"));

    return usLocale.toString(dateTimeEdited, "MMMM d, yyyy, h:mm A");
}

/**
* Get the full date of a note from database to present it in the right way in the notes list
*/
QString MainWindow::GetNoteDate (QString dateEdited)
{
    QDateTime dateTimeEdited(GetQDateTime(dateEdited));
    QLocale usLocale(QLocale("en_US"));

    if(dateTimeEdited.date() == QDate::currentDate())
    {
        return usLocale.toString(dateTimeEdited.time(),"h:mm A");
    }
    else if(dateTimeEdited.daysTo(QDateTime::currentDateTime()) == 1)
    {
        return "Yesterday";
    }
    else if(dateTimeEdited.daysTo(QDateTime::currentDateTime()) >= 2 && dateTimeEdited.daysTo(QDateTime::currentDateTime()) <= 7)
    {
        return usLocale.toString(dateTimeEdited.date(), "dddd");
    }

    return dateTimeEdited.date().toString("M/d/yy");
}

/**
* Add a note from database or create a new note, into the notes list (in scrollArea)
* Determine if it's loading an existing note or creating a new note and present it accordingly
* Return a pointer to the new note
*/
MainWindow::noteData* MainWindow::AddNote (QString noteName, bool isLoadingOrNew)
{
    #ifdef Q_OS_LINUX
        QFont titleLabelFont("Liberation Sans");
        QFont dateLabelFont("Liberation Sans");
    #elif _WIN32
        QFont titleLabelFont("Arial");
        QFont dateLabelFont("Arial");
    #elif __APPLE__
        QFont titleLabelFont("Helvetica");
        QFont dateLabelFont("Helvetica");
    #else
        #error "We don't support that version yet..."
    #endif
    titleLabelFont.setBold(true);
    titleLabelFont.setPixelSize(13);
    dateLabelFont.setPixelSize(11);

    noteData* newNote = new noteData;
    newNote->noteName = noteName;
    newNote->fakeContainer = new QGroupBox();
    newNote->containerBox = new QGroupBox(newNote->fakeContainer);
    newNote->button = new QPushButton("", newNote->containerBox);
    newNote->titleLabel = new QLabel("", newNote->containerBox);
    newNote->dateLabel = new QLabel("", newNote->containerBox);
    newNote->seperateLine = new QFrame(newNote->containerBox);
    newNote->scrollBarPosistion = 0;

    newNote->button->setObjectName("button");
    newNote->seperateLine->setObjectName("seperateLine");

    visibleNotesList.push_back(newNote);

    // On windows +2 is not enough room so we do it os specific
    // Maybe it's because of the way windows render its fonts
    #ifdef Q_OS_LINUX
        const double distanceBetweenEverything = 4;
        int addToTitleLabelHeight = 2;
        int addToDateLabelHeight = 0;
        int distanceToTitleLabel = distanceBetweenEverything;
    #elif _WIN32
        const double distanceBetweenEverything = 3.5;
        int addToTitleLabelHeight = 3;
        int addToDateLabelHeight = 1;
        int distanceToTitleLabel = 4;
    #elif __APPLE__
        const double distanceBetweenEverything = 4;
        int addToTitleLabelHeight = 2;
        int addToDateLabelHeight = 0;
        int distanceToTitleLabel = distanceBetweenEverything;
    #else
        #error "We don't support that version yet..."
    #endif

    newNote->titleLabel->setFont(titleLabelFont);
    newNote->titleLabel->setStyleSheet("QLabel { color : black; }");
    newNote->titleLabel->resize(ui->lineEdit->width()-1, 0);
    newNote->titleLabel->setFixedHeight(titleLabelFont.pixelSize() + addToTitleLabelHeight); // + So there would be room for letters like g,y etc..
    newNote->titleLabel->move(ui->horizontalSpacer_leftLineEdit->sizeHint().width(), distanceToTitleLabel);
    newNote->dateLabel->setFont(dateLabelFont);
    newNote->dateLabel->setFixedWidth(ui->scrollArea->width());
    newNote->dateLabel->setFixedHeight(dateLabelFont.pixelSize() + addToDateLabelHeight); // + So there would be room for letters like g,y etc..
    newNote->dateLabel->move(ui->horizontalSpacer_leftLineEdit->sizeHint().width(), newNote->titleLabel->height() + distanceBetweenEverything*2);
    newNote->dateLabel->setStyleSheet("QLabel { color : rgb(132, 132, 132); }");
    newNote->seperateLine->setFrameShape(QFrame::HLine);
    newNote->seperateLine->setGeometry(ui->horizontalSpacer_leftLineEdit->sizeHint().width(), newNote->titleLabel->height() + newNote->dateLabel->height() + distanceBetweenEverything*3, ui->lineEdit->width()-1, 1);
    newNote->seperateLine->setStyleSheet("QFrame { color : rgb(221, 221, 221); }");
    newNote->button->setGeometry(0, 0, ui->scrollArea->width(), newNote->titleLabel->height() + newNote->dateLabel->height() + newNote->seperateLine->height() + distanceBetweenEverything*3);
    newNote->button->raise();
    newNote->button->setStyleSheet("QPushButton { background-color: rgba(254, 206, 9, 0) }");
    newNote->button->setFlat(true);
    newNote->button->setFocusPolicy(Qt::NoFocus);
    newNote->fakeContainer->setFixedSize(ui->scrollArea->width(), newNote->titleLabel->height() + newNote->dateLabel->height() + newNote->seperateLine->height() + distanceBetweenEverything*3);
    newNote->fakeContainer->setFlat(true);
    newNote->fakeContainer->setStyleSheet("QGroupBox { border: none; }");
    newNote->containerBox->resize(ui->scrollArea->width(), newNote->titleLabel->height() + newNote->dateLabel->height() + newNote->seperateLine->height() + distanceBetweenEverything*3);
    newNote->containerBox->installEventFilter(this);

    if(currentVerticalScrollAreaRange > 0)
    {
        newNote->titleLabel->resize(ui->lineEdit->width() - 11, 0);
        newNote->seperateLine->resize(ui->lineEdit->width() - 11, 1);
    }

    QString noteFirstLine;
    // isLoadingOrNew - > true = Loading a note from database
    // isLoadingOrNew - > false = Creating a new note
    if(isLoadingOrNew)
    {
        noteFirstLine = GetFirstLineAndElide(newNote);
    }
    else
    {
        noteFirstLine = "New Note";
    }

    QString noteDate = GetNoteDate(notesDatabase->value(noteName + "/dateEdited", "Error").toString());

    newNote->titleLabel->setText(noteFirstLine);
    newNote->dateLabel->setText(noteDate);

    connect(newNote->button, SIGNAL(pressed()), this, SLOT(note_buttuon_pressed()));

    lay->insertWidget(0, newNote->fakeContainer, 0, Qt::AlignTop);

    return newNote;
}

/**
* Sort the notes that were loaded into the array 'stringNotesList'
* We are putting the data from stringNotesList into structer called QList
* So the std::stable_sort could manipulate on the data and sort it efficently
* We are sorting the notes by there last-edit-time from (oldest -> newest)
* So the oldest note will be placed at the bottom of the scrollArea and newest one at the top
* (adding notes from the vector happen left to right)
*/
void MainWindow::SortNotesList (QStringList &stringNotesList)
{
    notesDataForSorting.clear();

    for(int i = 0; i < stringNotesList.length() - 1; i += 3)
    {
        noteDataForSorting tempNoteDataForSorting;

        tempNoteDataForSorting.noteName = stringNotesList[i].split("/")[0];

        QString dateDB = notesDatabase->value(tempNoteDataForSorting.noteName + "/dateEdited", "Error").toString();
        tempNoteDataForSorting.dateTime =  GetQDateTime(dateDB);

        notesDataForSorting.push_back(tempNoteDataForSorting);
    }

    std::stable_sort(notesDataForSorting.begin(), notesDataForSorting.end());
}

/**
* Load all the notes from database into the notes list (in scrollArea)
*/
void MainWindow::LoadNotes ()
{
    QStringList stringNotesList = notesDatabase->allKeys();

    SortNotesList(stringNotesList);

    QString noteName;
    noteData* newNote;
    for(int i = 0; i < notesDataForSorting.length(); i++)
    {
        noteName = notesDataForSorting.at(i).noteName;

        newNote = AddNote(noteName, true);

        allNotesList.push_back(newNote);
    }

    if(stringNotesList.at(0) != "notesCounter")
    {
        noteOnTopInTheLayoutName = notesDataForSorting.last().noteName;
    }
}

/**
* Create a new note when clicking the 'new note' button
*/
void MainWindow::on_newNoteButton_clicked()
{
    Create_new_note();
}

/**
* Delete selected note when clicking the 'delete note' button
*/
void MainWindow::on_trashButton_clicked()
{
    DeleteSelectedNote();
}

/**
* Unhighlight the given note
*/
void MainWindow::UnhighlightNote (noteData* note)
{
    for(unsigned int i = 0; i < visibleNotesList.size() - 1; i++)
    {
        if(visibleNotesList.at(i) == note)
        {
            visibleNotesList.at(i + 1)->seperateLine->show();
            break;
        }
    }
    note->seperateLine->show();

    note->containerBox->setStyleSheet("");
    note->titleLabel->setStyleSheet("QLabel { color: black; }");
    note->dateLabel->setStyleSheet("QLabel { color: rgb(132, 132, 132); }");
}

/**
* Highlight the given note
*/
void MainWindow::HighlightNote (noteData* note, QString rgbStringColor)
{
    for(unsigned int i = 0; i < visibleNotesList.size() - 1; i++)
    {
        if(visibleNotesList.at(i) == note)
        {
            visibleNotesList.at(i + 1)->seperateLine->hide();
            break;
        }
    }
    note->seperateLine->hide();

    note->containerBox->setStyleSheet(QString(".QGroupBox { background-color: %1; }").arg(rgbStringColor));
    note->titleLabel->setStyleSheet(QString("QLabel { background-color: %1; color: black; }").arg(rgbStringColor));
    note->dateLabel->setStyleSheet(QString("QLabel { background-color: %1; color: rgb(132, 132, 132); }").arg(rgbStringColor));
}

/**
* Get a note and return it's animation property for deletion
*/
QPropertyAnimation* MainWindow::GetAnimationForDeletion (noteData* note)
{
    int tempY = note->fakeContainer->y();

    QPropertyAnimation* animation = new QPropertyAnimation(note->fakeContainer, "geometry");
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->setDuration(330);
    QRect geo = note->fakeContainer->geometry();
    animation->setStartValue(geo);
    geo.setY(tempY - note->fakeContainer->height()*2);
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
void MainWindow::note_buttuon_pressed ()
{
    for(unsigned int i = 0; i < visibleNotesList.size(); i++)
    {
        if(QObject::sender() == visibleNotesList.at(i)->button)
        {
            if(currentSelectedNote != 0 && currentSelectedNote != tempNote)
            {
                UnhighlightNote(currentSelectedNote);
            }

            currentSelectedNote = visibleNotesList.at(i);

            if(tempNote != 0 && currentSelectedNote != tempNote)
            {
                QPropertyAnimation* animation = GetAnimationForDeletion(tempNote);

                connect(animation, SIGNAL(finished()), this, SLOT(DeleteTempNoteFromVisual()));

                animation->start();

                DeleteNoteFromDataBase(tempNote);

                currentSelectedNote = visibleNotesList.at(i);
            }

            HighlightNote(currentSelectedNote, "rgb(254, 206, 9)");

            ui->textEdit->blockSignals(true);
            int tempScrollBarPosition = currentSelectedNote->scrollBarPosistion;
            ui->textEdit->setText(notesDatabase->value(currentSelectedNote->noteName + "/content", "Error").toString());
            ui->textEdit->verticalScrollBar()->setValue(tempScrollBarPosition);
            QString noteDate = notesDatabase->value(currentSelectedNote->noteName + "/dateEdited", "Error").toString();
            ui->editorDateLabel->setText(GetNoteDateEditor(noteDate));
            ui->textEdit->blockSignals(false);
        }
    }
}

/**
* Select the first note in the notes list
*/
void MainWindow::SelectFirstNote ()
{
    if(!visibleNotesList.empty())
    {
        visibleNotesList.back()->button->pressed();
    }
}

/**
* When clearButton in the search box (lineEdit) is pressed,
* clear the text in lineEdit and the date label in the editor.
*/
void MainWindow::ClearButtonClicked ()
{
    ui->lineEdit->clear();
}

/**
* Delete a given note from the database
*/
void MainWindow::DeleteNoteFromDataBase (noteData* note)
{
    for(unsigned int i = 0; i < allNotesList.size(); i++)
    {
        if(allNotesList.at(i)->noteName == note->noteName)
        {
            allNotesList.erase(allNotesList.begin() + i);
        }
    }

    // Putting the deleted note in trash
    int counter = trashDatabase->value("notesCounter").toInt() + 1;
    trashDatabase->setValue("notesCounter", counter);

    QString noteName = QString("noteID_%1").arg(counter);
    trashDatabase->setValue(noteName + "/content",  notesDatabase->value(note->noteName + "/content", "Error"));

    QString dateDBCreated = notesDatabase->value(note->noteName + "/dateEdited", "Error").toString();
    QString dateDBEdited = notesDatabase->value(note->noteName + "/dateEdited", "Error").toString();
    QString noteDate = QDateTime::currentDateTime().toString(Qt::ISODate);

    trashDatabase->setValue(noteName + "/dateCreated", dateDBCreated);
    trashDatabase->setValue(noteName + "/dateEdited", dateDBEdited);
    trashDatabase->setValue(noteName + "/dateTrashed", noteDate);

    notesDatabase->remove(note->noteName);

    notesDatabase->sync();
    trashDatabase->sync();
}

/**
* Remove a given note from the interface
*/
void MainWindow::DeleteNoteFromVisual (noteData* note)
{
    // Make the seperateLine of the note above it visible
    for(unsigned int i = 0; i < visibleNotesList.size(); i++)
    {
        if(visibleNotesList.at(i) == note)
        {
            if(i != visibleNotesList.size() - 1)
            {
                visibleNotesList.at(i + 1)->seperateLine->show();
            }
            visibleNotesList.erase(visibleNotesList.begin() + i);
            break;
        }
    }

    lay->removeWidget(note->fakeContainer);
    delete note;
}

/**
* When the text on textEdit change:
* Save the changes to database (auto-saving)
* if the note edited is not on top of the list, we will make that happen
* If the text changed is of a new (empty) note, reset temp values
* (We have those valuse to make sure to delete the note from database, if the program is closed and the new note is empty)
*/
void MainWindow::on_textEdit_textChanged ()
{
    //if(text has really changed)
    if(currentSelectedNote != 0 && ui->textEdit->toPlainText() != notesDatabase->value(currentSelectedNote->noteName + "/content", "Error"))
    {
        notesDatabase->setValue(currentSelectedNote->noteName + "/content", ui->textEdit->toPlainText());

        currentSelectedNote->titleLabel->setText(GetFirstLineAndElide(currentSelectedNote));
        QString noteDate = QDateTime::currentDateTime().toString(Qt::ISODate);
        notesDatabase->setValue(currentSelectedNote->noteName + "/dateEdited", noteDate);
        currentSelectedNote->dateLabel->setText(GetNoteDate(noteDate));
        ui->editorDateLabel->setText(GetNoteDateEditor(noteDate));

        //notesDatabase->sync(); // We may want to remove that

        if(currentSelectedNote->noteName != noteOnTopInTheLayoutName)
        {
            noteOnTopInTheLayoutName = currentSelectedNote->noteName;

            int tempY = currentSelectedNote->fakeContainer->y();
            tempY = (tempY > 330) ? 330 : tempY;
            QString noteName = currentSelectedNote->noteName;

            noteData* tempNoteToRemove = currentSelectedNote;
            DeleteNoteFromVisual(currentSelectedNote);

            currentSelectedNote = AddNote(noteName, true);
            allNotesList.push_back(currentSelectedNote);

            for(unsigned int i = 0; i < allNotesList.size(); i++)
            {
                if(tempNoteToRemove == allNotesList.at(i))
                {
                    allNotesList.erase(allNotesList.begin() + i);
                    break;
                }
            }

            HighlightNote(currentSelectedNote, "rgb(255, 232, 82)");

            ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->minimum());

            QPropertyAnimation *animation = new QPropertyAnimation(currentSelectedNote->containerBox, "geometry");
            animation->setEasingCurve(QEasingCurve::OutQuint);
            animation->setDuration(330);
            currentSelectedNote->containerBox->setGeometry(currentSelectedNote->containerBox->x(), tempY, currentSelectedNote->containerBox->width(), currentSelectedNote->containerBox->height());
            QRect geo = currentSelectedNote->fakeContainer->geometry();
            geo.setY(tempY);
            animation->setStartValue(geo);
            geo.setY(0);
            animation->setEndValue(geo);
            animation->start();
        }
    }

    if(isTemp && !notesDatabase->value(tempNote->noteName + "/content", "Error").toString().isEmpty())
    {
        tempNote = 0;
        isTemp = false;
    }
}

/**
* Return true if string 'keyword' is found in string 'content', else return false
*/
bool MainWindow::IsFound (QString keyword, QString content)
{
    return content.contains(keyword, Qt::CaseInsensitive);
}

/**
* Unhighlight the given note without modifing other notes
*/
void MainWindow::SimpleUnhighlightNote (noteData* note)
{
    note->seperateLine->show();
    note->containerBox->setStyleSheet("");
    note->titleLabel->setStyleSheet("QLabel { color: black; }");
    note->dateLabel->setStyleSheet("QLabel { color: rgb(132, 132, 132); }");
}

/**
* Remove all the notes from the scrollArea
*/
void MainWindow::ClearAllNotesFromVisual ()
{
    noteData *note;
    for(unsigned int i = 0; i < visibleNotesList.size(); i++)
    {
        note = visibleNotesList.at(i);
        note->fakeContainer->hide();
        SimpleUnhighlightNote(note);
    }

    visibleNotesList.clear();
    currentSelectedNote = 0;
}

/**
* Given a note name name, select the given note
* and set the scrollArea's scrollBar position to it
* return true if the given note was found, else return false
*/
bool MainWindow::GoToAndSelectNote (QString noteName)
{
    // Is there a better way? (We are doing this because the value of the scrollBar won't change unitl all notes in GUI are loaded)
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();

    bool found = false;

    for(unsigned int i = 0; i < visibleNotesList.size(); i++)
    {
        if(visibleNotesList.at(i)->noteName == noteName)
        {
            found = true;
            visibleNotesList.at(i)->button->pressed();
            unsigned int noteSize = visibleNotesList.at(i)->fakeContainer->height();
            ui->scrollArea->verticalScrollBar()->setValue((visibleNotesList.size()-1 - i) * noteSize);
        }
    }

    return found;
}

/*void MainWindow::UpdateAllNotesList ()
{

}*/

/**
* When text on lineEdit change:
* If there is a temp note "New Note" while searching, we delete it
* Saving the last selected note for recovery after searching
* Make the clear button visible when typing in the search box, or invisble when empty
* Clear all the notes from scrollArea and
* If text is empty, reload all the notes from database
* Else, load all the notes contain the string in lineEdit from database
*/
void MainWindow::on_lineEdit_textChanged (const QString &arg1)
{
    if(tempNote != 0)
    {
        DeleteNoteFromDataBase(tempNote);
        DeleteNoteFromVisual(tempNote);
        tempNote = 0;
        isTemp = false;
    }

    if(tempSelectedNoteBeforeSearchingName.isEmpty() && currentSelectedNote != 0)
    {
        tempSelectedNoteBeforeSearchingName = currentSelectedNote->noteName;
    }

    /*for(unsigned int i = 0; i < visibleNotesList.size(); i++)
    {
        UnhighlightNote(visibleNotesList.at(i));
    }*/
    ClearAllNotesFromVisual();
    //UpdateAllNotesList();

    if(arg1.isEmpty())
    {
        clearButton->setVisible(false);

        //UpdateAllNotesList();
        for(unsigned int i = 0; i < allNotesList.size(); i++)
        {
            allNotesList.at(i)->fakeContainer->show();
            visibleNotesList.push_back(allNotesList.at(i)); // Do we need to change the
            // strucuture from std::vector to Qlist so we could copy the list more efficiently?
        }
        //visibleNotesList = allNotesList; not working

        bool found = GoToAndSelectNote(tempSelectedNoteBeforeSearchingName);

        if(!found)
        {
            SelectFirstNote();
        }

        tempSelectedNoteBeforeSearchingName.clear();
    }
    else
    {
        clearButton->setVisible(true);

        ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->minimum());

        //UpdateAllNotesList();
        QString noteName;
        for(unsigned int i = 0; i < allNotesList.size(); i++)
        {
            noteName = allNotesList.at(i)->noteName;
            if(IsFound(arg1, notesDatabase->value(noteName + "/content", "Error").toString()))
            {
                allNotesList.at(i)->fakeContainer->show();
                visibleNotesList.push_back(allNotesList.at(i));
            }
        }


        if(visibleNotesList.empty())
        {
            ui->textEdit->blockSignals(true);
            ui->textEdit->clear();
            ui->editorDateLabel->clear();
            ui->textEdit->blockSignals(false);
        }

        SelectFirstNote();

        QCoreApplication::processEvents();
        QCoreApplication::processEvents();
    }
}

/**
* Create a new note in database
*/
QString MainWindow::CreateNewNoteInDatabase ()
{
    int counter = notesDatabase->value("notesCounter").toInt() + 1;
    notesDatabase->setValue("notesCounter", counter);

    QString noteName = QString("noteID_%1").arg(counter);
    notesDatabase->setValue(noteName + "/content",  "");

    QString noteDate = QDateTime::currentDateTime().toString(Qt::ISODate);
    notesDatabase->setValue(noteName + "/dateCreated", noteDate);
    notesDatabase->setValue(noteName + "/dateEdited", noteDate);

    notesDatabase->sync();

    return noteName;
}

/**
* Making the new note animation
* I have noticed somethings wierd:
* when the animation accour (new note is created) and the scrollArea's scrollBar is showing,
* Something wierd is going on at the start of the animation
* Yet if you click again on the 'newNoteButton' everything is fine
*/
void MainWindow::NewNoteAnimation ()
{
    int tempY = currentSelectedNote->fakeContainer->y() - currentSelectedNote->fakeContainer->height() * 2;

    QPropertyAnimation *animation = new QPropertyAnimation(currentSelectedNote->fakeContainer, "geometry");
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->setDuration(330);

    QRect geo = currentSelectedNote->fakeContainer->geometry();
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
void MainWindow::Create_new_note ()
{
    if(!isTemp)
    {
        if(!ui->lineEdit->text().isEmpty())
        {
            ui->lineEdit->clear();
        }

        QString noteName = CreateNewNoteInDatabase();
        tempNote = AddNote(noteName, false);
        allNotesList.push_back(tempNote);
        noteOnTopInTheLayoutName = noteName;
        isTemp = true;

        if(currentSelectedNote != 0)
        {
            UnhighlightNote(currentSelectedNote);
        }
        currentSelectedNote = tempNote;
        HighlightNote(currentSelectedNote, "rgb(255, 235, 80)");

        ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->minimum());
        ui->editorDateLabel->setText(GetNoteDateEditor(notesDatabase->value(noteName + "/dateEdited", "Error").toString()));
        ui->textEdit->blockSignals(true);
        ui->textEdit->clear();
        ui->textEdit->setFocus();
        ui->textEdit->blockSignals(false);

        QCoreApplication::processEvents();

        NewNoteAnimation();
    }
    else
    {
        ui->textEdit->blockSignals(true);
        ui->textEdit->setFocus();
        ui->textEdit->blockSignals(false);
        ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->minimum());

        NewNoteAnimation();
    }
}

/**
* Take care of all the visual things around note deletion:
* Delete the selected note from the interface
* And selecting another note (if there is one)
* If the last note is deleted, select the note above
* else, select the note under it
*/
void MainWindow::DeleteSelectedNoteFromVisual ()
{
    unsigned int tempNotePlaceInLayout = 0;
    for(unsigned int i = 0; i < visibleNotesList.size(); i++)
    {
        if(visibleNotesList.at(i) == currentSelectedNote)
        {
            tempNotePlaceInLayout = i;
            break;
        }
    }

    DeleteNoteFromVisual(currentSelectedNote);
    currentSelectedNote = 0;

    ui->textEdit->blockSignals(true);
    ui->textEdit->clearFocus();
    ui->textEdit->blockSignals(false);

    if(visibleNotesList.size() > 0)
    {
        if(tempNotePlaceInLayout == 0)
        {
            visibleNotesList.at(tempNotePlaceInLayout)->button->pressed();
        }
        else
        {
            visibleNotesList.at(tempNotePlaceInLayout - 1)->button->pressed();
        }
    }
}

/**
* Delete the temporary note (a new note that was created but wasn't edited)
*/
void MainWindow::DeleteTempNoteFromVisual ()
{
    DeleteNoteFromVisual(tempNote);

    tempNote = 0;
    isTemp = false;
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
void MainWindow::DeleteSelectedNote ()
{
    if(currentSelectedNote != 0)
    {
        if(tempNote != 0 && currentSelectedNote->noteName == tempNote->noteName)
        {
            tempNote = 0;
            isTemp = false;
        }

        QPropertyAnimation *animation = GetAnimationForDeletion(currentSelectedNote);

        //Why can't Qt let pass data throught SLOT?
        connect(animation, SIGNAL(finished()), this, SLOT(DeleteSelectedNoteFromVisual()));

        animation->start();

        DeleteNoteFromDataBase(currentSelectedNote);

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
void MainWindow::SetFocusOnText ()
{
    if(currentSelectedNote != 0 && !ui->textEdit->hasFocus())
    {
        ui->textEdit->setFocus();
    }
}

/**
* Set focus on scrollArea and highlight the currentSelectedNote
*/
void MainWindow::SetFocusOnScrollArea ()
{
    ui->scrollArea->setFocus();

    if(currentSelectedNote != 0)
    {
        HighlightNote(currentSelectedNote, "rgb(254, 206, 9)");
    }
}

/**
* Select the note above the currentSelectedNote
*/
void MainWindow::SelectNoteUp ()
{
    if(currentSelectedNote != 0)
    {
        if(!ui->scrollArea->hasFocus())
        {
            ui->scrollArea->setFocus();
        }

        for(unsigned int i = 0; i < visibleNotesList.size() - 1; i++)
        {
            if(visibleNotesList.at(i)->fakeContainer  == currentSelectedNote->fakeContainer)
            {
                unsigned int noteSize = currentSelectedNote->fakeContainer->height();

                if((visibleNotesList.size() - i - 1) * noteSize < ui->scrollArea->verticalScrollBar()->value() + noteSize)
                {
                    ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value() - noteSize);
                }

                visibleNotesList.at(i + 1)->button->pressed();
                break;
            }
        }
    }
}

/**
* Select the note under the currentSelectedNote
*/
void MainWindow::SelectNoteDown ()
{
    if(currentSelectedNote != 0)
    {
        if(!ui->scrollArea->hasFocus())
        {
            ui->scrollArea->setFocus();
        }

        for(unsigned int i = 1; i < visibleNotesList.size(); i++)
        {
            if(visibleNotesList.at(i)->fakeContainer  == currentSelectedNote->fakeContainer)
            {
                unsigned int noteSize = currentSelectedNote->fakeContainer->height();

                if((visibleNotesList.size() - i - 1) * noteSize > ui->scrollArea->verticalScrollBar()->value() + ui->scrollArea->height() - noteSize * 2)
                {
                    ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value() + noteSize);
                }

                visibleNotesList.at(i - 1)->button->pressed();
                break;
            }
        }
    }
}

/**
* Switch to fullscreen mode
*/
void MainWindow::FullscreenWindow ()
{
    if(this->windowState() == Qt::WindowFullScreen)
    {
        this->setWindowState(Qt::WindowNoState);
    }
    else
    {
        this->setWindowState(Qt::WindowFullScreen);
    }
}

/**
* Maximize the window
*/
void MainWindow::MaximizeWindow ()
{
    if(this->windowState() == Qt::WindowMaximized || this->windowState() == Qt::WindowFullScreen)
    {
        this->setWindowState(Qt::WindowNoState);
    }
    else
    {
        this->setWindowState(Qt::WindowMaximized);
    }
}

/**
* Minimize the window
*/
void MainWindow::MinimizeWindow ()
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
    if(tempNote != 0)
    {
        ui->textEdit->blockSignals(true);
        DeleteNoteFromDataBase(tempNote);
    }

    //QApplication::quit();
    MainWindow::close();
}

/**
* When the green button is pressed set it's icon accordingly
*/
void MainWindow::on_greenMaximizeButton_pressed ()
{
    if(this->windowState() == Qt::WindowFullScreen)
    {
        ui->greenMaximizeButton->setIcon(QIcon(":/images/greenInPressed.png"));
    }
    else
    {
        ui->greenMaximizeButton->setIcon(QIcon(":/images/greenPressed.png"));
    }
}

/**
* When the yellow button is pressed set it's icon accordingly
*/
void MainWindow::on_yellowMinimizeButton_pressed ()
{
    ui->yellowMinimizeButton->setIcon(QIcon(":/images/yellowPressed.png"));
}

/**
* When the red button is pressed set it's icon accordingly
*/
void MainWindow::on_redCloseButton_pressed ()
{
    ui->redCloseButton->setIcon(QIcon(":/images/redPressed.png"));
}

/**
* When the green button is released the window goes fullscrren
*/
void MainWindow::on_greenMaximizeButton_clicked()
{
    ui->greenMaximizeButton->setIcon(QIcon(":/images/green.png"));

    FullscreenWindow();
}

/**
* When yellow button is released the window is minimized
*/
void MainWindow::on_yellowMinimizeButton_clicked()
{
    ui->yellowMinimizeButton->setIcon(QIcon(":/images/yellow.png"));

    MinimizeWindow();
}

/**
* When red button is released the window get closed
* If a new note created but wasn't edited, delete it from the database
*/
void MainWindow::on_redCloseButton_clicked()
{
    ui->redCloseButton->setIcon(QIcon(":/images/red.png"));

    QuitApplication();
}


/**
* Get called everytime the scrollArea's scrollBar range change
* If the scrollBar is showing we decrease the width of the note
* Else, we return it to it's default width
* We are doing this so the notes will look equaly clear in both situations
* (when scrollBar is visible, and when not)
*/
void MainWindow::ScrollAreaScrollBarRangeChange (int, int verticalScrollBarRange)
{
    currentVerticalScrollAreaRange = verticalScrollBarRange;

    // We need this because this function isn't executed before AddNote
    if(verticalScrollBarRange > 0)
    {
        for(unsigned int i = 0; i < visibleNotesList.size(); i++)
        {
            if(visibleNotesList.at(i)->titleLabel->width() != ui->lineEdit->width()- 11)
            {
                visibleNotesList.at(i)->titleLabel->resize(ui->lineEdit->width()- 11, 0);
                visibleNotesList.at(i)->seperateLine->resize(ui->lineEdit->width()- 11, 1);
                visibleNotesList.at(i)->titleLabel->setText(GetFirstLineAndElide(visibleNotesList.at(i)));
            }
        }
    }
    else
    {
        for(unsigned int i = 0; i < visibleNotesList.size(); i++)
        {
            if(visibleNotesList.at(i)->titleLabel->width() != ui->lineEdit->width()-1)
            {
                visibleNotesList.at(i)->titleLabel->resize(ui->lineEdit->width()-1, 0);
                visibleNotesList.at(i)->seperateLine->resize(ui->lineEdit->width()-1, 1);
                visibleNotesList.at(i)->titleLabel->setText(GetFirstLineAndElide(visibleNotesList.at(i)));
            }
        }
    }
}

/**
* Get called everytime the textEdit's scrollBar range change
* If the scrollBar is showing change the padding of textEdit
* Else, we return it to it's default padding
* We are doing this so the text padding will be equal in both situations
* (when scrollBar is visible, and when not)
*/
void MainWindow::TextEditScrollBarRangeChange (int, int verticalScrollBarRange)
{
    if(verticalScrollBarRange > 0)
    {
        ui->textEdit->setStyleSheet(QString("QScrollBar:handle:vertical:hover { background: rgb(170, 170, 171); } QScrollBar:handle:vertical:pressed { background: rgb(149, 149, 149); } QTextEdit { background-image: url(:/images/textSideBackground.png); padding-left: %1px; padding-right: %2px; } QScrollBar:vertical { border: none; width: 8px; } QScrollBar::handle:vertical { border-radius: 2.5px; background: rgb(188, 188, 188); min-height: 20px; }  QScrollBar::add-line:vertical { height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  QScrollBar::sub-line:vertical { height: 0px; subcontrol-position: top; subcontrol-origin: margin; }")
                                    .arg(QString::number(ui->newNoteButton->width() - textEditLeftPadding), "19"));
    }
    else
    {
        ui->textEdit->setStyleSheet(QString("QScrollBar:handle:vertical:hover { background: rgb(170, 170, 171); } QScrollBar:handle:vertical:pressed { background: rgb(149, 149, 149); } QTextEdit { background-image: url(:/images/textSideBackground.png); padding-left: %1px; padding-right: %2px; } QScrollBar:vertical { border: none; width: 8px; } QScrollBar::handle:vertical { border-radius: 2.5px; background: rgb(188, 188, 188); min-height: 20px; }  QScrollBar::add-line:vertical { height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  QScrollBar::sub-line:vertical { height: 0px; subcontrol-position: top; subcontrol-origin: margin; }")
                                    .arg(QString::number(ui->newNoteButton->width() - textEditLeftPadding), "27"));
    }
}

/**
* Get called everytime the textEdit's scrollBar value change
* Gets called to save the value of the scrollBar in the currentSelectedNote
* for recovering the scrollBar position when browsing different notes
*/
void MainWindow::TextEditScrollBarValueChange (int verticalScrollBarValue)
{
    if(currentSelectedNote != 0)
    {
        currentSelectedNote->scrollBarPosistion = verticalScrollBarValue;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(windowState() != Qt::WindowFullScreen && windowState() != Qt::WindowMaximized)
    {
        settingsDatabase->setValue("geometry", saveGeometry());
    }
    QWidget::closeEvent(event);
}

/**
* Set variables to the position of the window when the mouse is pressed
* And set variables for resizing
*/
void MainWindow::mousePressEvent (QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if(event->pos().x() < ui->centralWidget->geometry().width() && event->pos().y() < ui->textEdit->y())
        {
            canMoveWindow = true;
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
    if(canMoveWindow)
    {
        this->setCursor(Qt::ClosedHandCursor);

        move (event->globalX() - m_nMouseClick_X_Coordinate, event->globalY() - m_nMouseClick_Y_Coordinate);
    }
}

/**
* Set some variables after releasing the mouse for moving the window and resizing
*/
void MainWindow::mouseReleaseEvent (QMouseEvent *event)
{
    canMoveWindow = false;
    this->unsetCursor();
    event->accept();
}

bool MainWindow::IsClickingButton (QPoint mousePos, QPushButton* button)
{
    if(mousePos.y() > button->y() &&
            mousePos.y() < button->y() + button->height() &&
            mousePos.x() > button->x() &&
            mousePos.x() < button->x() + button->width())
    {
        return true;
    }

    return false;
}

/**
* When the blank area at the top of window is double-clicked the window get maximized
*/
void MainWindow::mouseDoubleClickEvent (QMouseEvent *e)
{
    if(e->pos().y() < ui->textEdit->y() && !IsClickingButton(e->pos(), ui->newNoteButton) && !IsClickingButton(e->pos(), ui->trashButton))
    {
        MaximizeWindow();
    }
    e->accept();
}

/**
* When the window is resizing some widgets size adjust accordingly
*/
void MainWindow::resizeEvent (QResizeEvent *)
{
    frame->move(ui->scrollArea->width()+ui->line->width(), 0);
    frame->resize(ui->centralWidget->width()-ui->scrollArea->width()-ui->line->width(), ui->redCloseButton->height()+ui->newNoteButton->height()+ui->verticalSpacer_upLineEdit->sizeHint().height()+ui->verticalSpacer_upScrollArea->sizeHint().height()+ui->line_3->height());
}

/**
* Mostly take care on what happens when hovering/unhovering some widgets
*/
bool MainWindow::eventFilter (QObject *object, QEvent *event)
{
    if(event->type() == QEvent::Enter)
    {
        // When hovering one of the traffic light buttons (red, yellow, green),
        // set new icons to show their function
        if(object == ui->redCloseButton || object == ui->yellowMinimizeButton || object == ui->greenMaximizeButton)
        {
            ui->redCloseButton->setIcon(QIcon(":/images/redHovered.png"));
            ui->yellowMinimizeButton->setIcon(QIcon(":/images/yellowHovered.png"));
            if(this->windowState() == Qt::WindowFullScreen)
            {
                ui->greenMaximizeButton->setIcon(QIcon(":/images/greenInHovered.png"));
            }
            else
            {
                ui->greenMaximizeButton->setIcon(QIcon(":/images/greenHovered.png"));
            }
        }

        // When hovering upon note with the mouse, highlight it
        if(QString(object->metaObject()->className()) == "QGroupBox")
        {
            QGroupBox *containerBox = qobject_cast<QGroupBox *>(object);

            for(unsigned int i = 0; i < visibleNotesList.size(); i++)
            {
                if(visibleNotesList.at(i)->containerBox == containerBox)
                {
                    if((currentSelectedNote != 0 && visibleNotesList.at(i) != currentSelectedNote) || currentSelectedNote == 0)
                    {
                        currentHoveredNote = visibleNotesList.at(i);
                        HighlightNote(currentHoveredNote, "rgb(207, 207, 207)");
                    }
                    break;
                }
            }
        }

        // When hoovering upon scrollArea's scrollBar, show it's border
        if(object == ui->scrollArea->verticalScrollBar())
        {
            ui->scrollArea->setStyleSheet("QScrollBar:handle:vertical:hover { background: rgb(170, 170, 171); } QScrollBar:handle:vertical:pressed { background: rgb(149, 149, 149); } * { background-color: rgb(255, 255, 255);  } QScrollBar:vertical { background: rgb(218, 218, 218); border: 1px rgb(218, 218, 218); border-radius: 5px; width: 11px; } QScrollBar::handle:vertical { border-radius: 5px; background: rgb(188, 188, 188); min-height: 20px; }  QScrollBar::add-line:vertical { height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  QScrollBar::sub-line:vertical { height: 0px; subcontrol-position: top; subcontrol-origin: margin; }");
        }

        // When hoovering upon textEdit's scrollBar, show it's border
        if(object == ui->textEdit->verticalScrollBar())
        {
            ui->textEdit->setStyleSheet(QString("QScrollBar:handle:vertical:hover { background: rgb(170, 170, 171); } QScrollBar:handle:vertical:pressed { background: rgb(149, 149, 149); } QTextEdit { background-image: url(:/images/textSideBackground.png); padding-left: %1px; padding-right: %2px; } QScrollBar:vertical { background: rgb(218, 218, 218); border: 1px rgb(218, 218, 218); border-radius: 5px; width: 11px; } QScrollBar::handle:vertical { border-radius: 5px; background: rgb(188, 188, 188); min-height: 20px; }  QScrollBar::add-line:vertical { height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  QScrollBar::sub-line:vertical { height: 0px; subcontrol-position: top; subcontrol-origin: margin; }")
                                        .arg(QString::number(ui->newNoteButton->width() - textEditLeftPadding), "16"));

            if(!ui->lineEdit->text().isEmpty() && currentSelectedNote != 0)
            {
                focusBreaker = true;
            }
        }
    }

    if(event->type() == QEvent::Leave)
    {
        // When not hivering, change back the icons of the traffic lights to their default icon
        if(object == ui->redCloseButton || object == ui->yellowMinimizeButton || object == ui->greenMaximizeButton)
        {
            ui->redCloseButton->setIcon(QIcon(":/images/red.png"));
            ui->yellowMinimizeButton->setIcon(QIcon(":/images/yellow.png"));
            ui->greenMaximizeButton->setIcon(QIcon(":/images/green.png"));
        }

        // When getting out of a highlighted note, unghighlight it
        if(QString(object->metaObject()->className()) == "QGroupBox")
        {
            QGroupBox *containerBox = qobject_cast<QGroupBox *>(object);

            if(currentHoveredNote != 0 && currentHoveredNote->containerBox == containerBox)
            {
                if((currentSelectedNote != 0 && currentHoveredNote != currentSelectedNote) || currentSelectedNote == 0)
                {
                    for(unsigned int i = 0; i < visibleNotesList.size(); i++)
                    {
                        if(visibleNotesList.at(i) == currentHoveredNote)
                        {
                            if((i > 0 && visibleNotesList.at(i - 1) != currentSelectedNote) || i == 0)
                            {
                                currentHoveredNote->seperateLine->show();
                            }

                            if(i < visibleNotesList.size() - 1 && visibleNotesList.at(i + 1) != currentSelectedNote)
                            {
                                visibleNotesList.at(i + 1)->seperateLine->show();
                            }
                        }
                    }

                    currentHoveredNote->containerBox->setStyleSheet("");
                    currentHoveredNote->titleLabel->setStyleSheet("QLabel { color: black; }");
                    currentHoveredNote->dateLabel->setStyleSheet("QLabel { color: rgb(132, 132, 132); }");
                }
            }
        }

        // When note havering the scrollArea's scrollBar, change back to it's default style
        if(object == ui->scrollArea->verticalScrollBar())
        {
            ui->scrollArea->setStyleSheet("QScrollBar:handle:vertical:hover { background: rgb(170, 170, 171); } QScrollBar:handle:vertical:pressed { background: rgb(149, 149, 149); } * { background-color: rgb(255, 255, 255);  } QScrollBar:vertical { border: none; width: 8px; } QScrollBar::handle:vertical { border-radius: 2.5px; background: rgb(188, 188, 188); min-height: 20px; }  QScrollBar::add-line:vertical { height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  QScrollBar::sub-line:vertical { height: 0px; subcontrol-position: top; subcontrol-origin: margin; }");
        }

        // When note havering the textEdit's scrollBar, change back to it's default style
        if(object == ui->textEdit->verticalScrollBar())
        {
            ui->textEdit->setStyleSheet(QString("QScrollBar:handle:vertical:hover { background: rgb(170, 170, 171); } QScrollBar:handle:vertical:pressed { background: rgb(149, 149, 149); } QTextEdit { background-image: url(:/images/textSideBackground.png); padding-left: %1px; padding-right: %2px; } QScrollBar:vertical { border: none; width: 8px; } QScrollBar::handle:vertical { border-radius: 2.5px; background: rgb(188, 188, 188); min-height: 20px; }  QScrollBar::add-line:vertical { height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; }  QScrollBar::sub-line:vertical { height: 0px; subcontrol-position: top; subcontrol-origin: margin; }")
                                        .arg(QString::number(ui->newNoteButton->width() - textEditLeftPadding), "19"));

            focusBreaker = false;
            if(!ui->lineEdit->text().isEmpty() && currentSelectedNote != 0)
            {
                ui->textEdit->clearFocus();
            }
        }
    }

    if(event->type() == QEvent::FocusIn)
    {
        if(object == ui->textEdit)
        {
            // If there are no notes, and the user click the textEdit, create a new note
            if(currentSelectedNote == 0)
            {
                Create_new_note();
            }

            // When clicking in a note's content while searching,
            // reload all the notes and go and select that note
            if(!focusBreaker && !ui->lineEdit->text().isEmpty() && currentSelectedNote != 0)
            {
                QString tempName = currentSelectedNote->noteName;

                ui->lineEdit->clear();

                GoToAndSelectNote(tempName);

                HighlightNote(currentSelectedNote, "rgb(255, 235, 80)");
            }
        }
    }

    if(event->type() == QEvent::FocusOut)
    {
        // If the scrollArea is out of focus change the highligted color of currentSelectedNote
        if(object == ui->scrollArea)
        {
            if(currentSelectedNote != 0)
            {
                HighlightNote(currentSelectedNote, "rgb(255, 235, 80)");
            }
        }
    }

    return false;
}

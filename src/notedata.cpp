#include "notedata.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QSplitter>
#include <QFocusEvent>

NoteData::NoteData(const QString& noteName, QWidget *parent) :
    QWidget(parent),
    m_isSelected(false),
    m_isModified(false),
    m_scrollBarPosition(0),
    m_noteName(noteName),
    m_focusColor(qRgb(254, 206, 9)),
    m_unfocusColor(qRgb(255, 235, 80)),
    m_enterColor(qRgb(207, 207, 207)),
    m_defaultColor(qRgb(255, 255, 255)),
    m_frameContainer(new QFrame(this)),
    m_button(new QPushButton("", m_frameContainer)),
    m_titleLabel(new QLabel("", m_frameContainer)),
    m_dateLabel(new QLabel("", m_frameContainer))
{
    setFocusPolicy(Qt::StrongFocus);
    setupWidget();
    connect(m_button, SIGNAL(pressed()), this, SLOT(onButtonPressed()));
}

NoteData::~NoteData()
{
}

void NoteData::setTitle(QString& title)
{
    m_fullTitle = title;
    elideTitle();
}

void NoteData::resizeEvent(QResizeEvent *)
{
    m_button->setFixedWidth(this->width());
    elideTitle();
}

void NoteData::focusInEvent(QFocusEvent *)
{
    setBackgroundColor(m_focusColor);
}

void NoteData::focusOutEvent(QFocusEvent *)
{
    if(m_isSelected){
        setBackgroundColor(m_unfocusColor);
    }else{
        setBackgroundColor(m_defaultColor);
    }
}

void NoteData::enterEvent(QEvent *)
{
    if(!m_isSelected)
        setBackgroundColor(m_enterColor);
}

void NoteData::leaveEvent(QEvent *)
{
    if(!m_isSelected)
        setBackgroundColor(m_defaultColor);
}

void NoteData::setupWidget()
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

    m_button->setObjectName("button");
    m_titleLabel->setObjectName("titleLabel");
    m_dateLabel->setObjectName("dateLabel");
    m_frameContainer->setObjectName("container");

    // On windows +2 is not enough room so we do it os specific
    // Maybe it's because of the way windows render its fonts
#ifdef Q_OS_LINUX
    const int distanceBetweenEverything = 4;
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

    // title label
    m_titleLabel->setFont(titleLabelFont);
    m_titleLabel->resize(this->width() - 1, 0);
    int tHeight = titleLabelFont.pixelSize() + addToTitleLabelHeight;
    m_titleLabel->setFixedHeight(tHeight);
    m_titleLabel->move(0, distanceToTitleLabel);
    // date label
    m_dateLabel->setFont(dateLabelFont);
    m_dateLabel->resize(this->width(), m_dateLabel->height());
    int dHeight = dateLabelFont.pixelSize() + addToDateLabelHeight;
    m_dateLabel->setFixedHeight(dHeight);
    int dy = m_titleLabel->height() + distanceBetweenEverything*2;
    m_dateLabel->move(0, dy);
    // Note Height
    int noteHeight = m_titleLabel->height()
            + m_dateLabel->height()
            + distanceBetweenEverything*3;
    this->setFixedHeight(noteHeight);
    // button
    m_button->setGeometry(0, 0, this->width(), this->height());
    m_button->raise();
    m_button->setFlat(true);
    m_button->setFocusPolicy(Qt::NoFocus);
    //container
    m_frameContainer->setFixedHeight(this->height());

    // setup layouts
    QVBoxLayout* vLayoutFakeContainer = new QVBoxLayout;
    vLayoutFakeContainer->setContentsMargins(0,0,0,0);
    vLayoutFakeContainer->setSpacing(0);
    vLayoutFakeContainer->addWidget(m_titleLabel);
    vLayoutFakeContainer->addWidget(m_dateLabel);
    m_frameContainer->setLayout(vLayoutFakeContainer);

    QVBoxLayout* vLayoutNote = new QVBoxLayout;
    vLayoutNote->setContentsMargins(0,0,0,0);
    vLayoutNote->setSpacing(0);
    vLayoutNote->addWidget(m_frameContainer);
    this->setLayout(vLayoutNote);


    QString ss = "#container { "
                 "  border: none; "
                 "  border-bottom:1px solid rgb(221, 221, 221); "
                 "  background-color: white"
                 "}"
                 "#titleLabel{"
                 "  margin-left:9px;"
                 "  background-color:transparent;"
                 "  color: black"
                 "}"
                 "#dateLabel{"
                 "  margin-left:9px;"
                 "  color: rgb(132, 132, 132);"
                 "  background-color:transparent;"
                 "}"
                 "#button{"
                 "  border: none; "
                 "  outline: none;"
                 "  background-color: transparent;"
                 "}";

    this->setStyleSheet(ss);
}

void NoteData::setBackgroundColor(QColor color)
{
    QString ss = QString("#container{"
                         "  border: none; "
                         "  border-bottom:1px solid rgb(221, 221, 221); "
                         "  background-color: %1; margin:0"
                         "}"
                         ).arg(color.name());
    m_frameContainer->setStyleSheet(ss);
}

void NoteData::elideTitle()
{
    QFontMetrics fontMetrics = m_titleLabel->fontMetrics();
    QString elidedText = fontMetrics.elidedText(m_fullTitle,
                                            Qt::ElideRight,
                                            this->width()-25);
    m_titleLabel->setText(elidedText);
}

QString NoteData::parseDateTime(QDateTime dateTimeEdited)
{
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

int NoteData::scrollBarPosition() const
{
    return m_scrollBarPosition;
}

void NoteData::setScrollBarPosition(int scrollBarPosition)
{
    m_scrollBarPosition = scrollBarPosition;
}

QDateTime NoteData::dateTime() const
{
    return m_dateTime;
}

void NoteData::setDateTime(const QDateTime &dateTime)
{
    m_dateTime = dateTime;
    QString dateTimeForLabel = parseDateTime(dateTime);
    m_dateLabel->setText(dateTimeForLabel);
}

QString NoteData::text() const
{
    return m_text;
}

void NoteData::setText(const QString &text)
{
    m_text = text;
}

QString NoteData::noteName() const
{
    return m_noteName;
}

void NoteData::setNoteName(const QString &noteName)
{
    m_noteName = noteName;
}

bool NoteData::isModified() const
{
    return m_isModified;
}

void NoteData::setModified(bool isModified)
{
    m_isModified = isModified;
}

bool NoteData::isSelected() const
{
    return m_isSelected;
}

void NoteData::setSelected(bool isSelected)
{
    m_isSelected = isSelected;
    isSelected ? setBackgroundColor(m_focusColor)
               : setBackgroundColor(m_defaultColor);
}

void NoteData::setSelectedWithFocus(bool isSelected, bool focus)
{
    m_isSelected = isSelected;
    if(m_isSelected){
        focus ? setFocus()
              : setBackgroundColor(m_unfocusColor);
    }else{
        setBackgroundColor(m_defaultColor);
    }
}

void NoteData::onButtonPressed()
{
    setSelected(true);
    emit pressed();
}

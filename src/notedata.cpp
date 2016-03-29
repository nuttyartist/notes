#include "notedata.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QSplitter>

NoteData::NoteData(const QString& noteName, QWidget *parent) :
    QWidget(parent),
    m_isSelected(false),
    m_isModified(false),
    m_scrollBarPosition(0),
    m_noteName(noteName),
    m_focusColor(qRgb(254, 206, 9)),
    m_unfocusColor(qRgb(255, 235, 80)),
    m_enterColor(qRgb(207, 207, 207)),
    m_defaultColor(Qt::white),
    m_backgroundColor(Qt::white),
    m_frameContainer(new QFrame(this)),
    m_button(new QPushButton(m_frameContainer)),
    m_titleLabel(new QLabel(m_frameContainer)),
    m_dateLabel(new QLabel(m_frameContainer)),
    m_titleFontMetrics(m_dateLabel->fontMetrics())

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

void NoteData::showSeparator(bool doShow)
{
    QString serparatorColorName = doShow ? QColor(qRgb(221, 221, 221)).name()
                                         : m_backgroundColor.name();
    QString ssButton = QStringLiteral("#button{"
                                      "  border: none; "
                                      "  border-bottom:1px solid %1; "
                                      "  outline: none;"
                                      "  background-color: transparent;"
                                      "  margin-left : 10px"
                                      "}").arg(serparatorColorName);
    m_button->setStyleSheet(ssButton);
}

void NoteData::updateWidth()
{
    m_button->setFixedWidth(this->width());
    elideTitle();
}

void NoteData::resizeEvent(QResizeEvent *)
{
    updateWidth();
}

void NoteData::focusInEvent(QFocusEvent *)
{
    updateStyleSheet(m_focusColor, false);
    emit focusedIn();
}

void NoteData::focusOutEvent(QFocusEvent *)
{
    if(m_isSelected){
        updateStyleSheet(m_unfocusColor, false);
    }else{
        updateStyleSheet(m_defaultColor,true);
    }
    emit focusedOut();
}

void NoteData::enterEvent(QEvent *)
{
    if(!m_isSelected)
        updateStyleSheet(m_enterColor,false);

    emit hoverEntered();
}

void NoteData::leaveEvent(QEvent *)
{
    if(!m_isSelected)
        updateStyleSheet(m_defaultColor, true);

    emit hoverLeft();
}

void NoteData::setupWidget()
{
#ifdef Q_OS_LINUX
    QFont titleLabelFont(QStringLiteral("Liberation Sans"));
    QFont dateLabelFont(QStringLiteral("Liberation Sans"));
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

    m_button->setObjectName(QStringLiteral("button"));
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));
    m_dateLabel->setObjectName(QStringLiteral("dateLabel"));
    m_frameContainer->setObjectName(QStringLiteral("container"));

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
    int tHeight = titleLabelFont.pixelSize() + addToTitleLabelHeight;
    m_titleLabel->resize(this->width() - 1, tHeight);
    m_titleLabel->move(0, distanceToTitleLabel);
    m_titleLabel->setAttribute(Qt::WA_TranslucentBackground);
    m_titleFontMetrics = m_titleLabel->fontMetrics();
    // date label
    m_dateLabel->setFont(dateLabelFont);
    int dHeight = dateLabelFont.pixelSize() + addToDateLabelHeight;
    m_dateLabel->resize(this->width(), dHeight);
    int dy = m_titleLabel->height() + distanceBetweenEverything*2;
    m_dateLabel->move(0, dy);
    m_dateLabel->setAttribute(Qt::WA_TranslucentBackground);
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
    m_button->setAttribute(Qt::WA_TranslucentBackground);
    showSeparator(true);
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
}

void NoteData::updateStyleSheet(QColor color, bool doShowSeparator)
{
    m_backgroundColor = color;
    QString ssContainer = QStringLiteral("#container{"
                                         "  border: none; "
                                         "  background-color: %1; "
                                         "}"
                                         ).arg(m_backgroundColor.name());

    m_frameContainer->setStyleSheet(ssContainer);

    showSeparator(doShowSeparator);
}

void NoteData::elideTitle()
{
    if (m_titleFontMetrics.width(m_fullTitle) > this->width()-25) {
        QString elidedText = m_titleFontMetrics.elidedText(m_fullTitle,
                                                           Qt::ElideRight,
                                                           this->width()-25);
        m_titleLabel->setText(elidedText);
    }else{
        m_titleLabel->setText(m_fullTitle);
    }
}

QString NoteData::parseDateTime(QDateTime dateTimeEdited)
{
    QLocale usLocale(QLocale("en_US"));

    auto currDateTime = QDateTime::currentDateTime();

    if(dateTimeEdited.date() == currDateTime.date()){
        return usLocale.toString(dateTimeEdited.time(),"h:mm A");
    }else if(dateTimeEdited.daysTo(currDateTime) == 1){
        return "Yesterday";
    }else if(dateTimeEdited.daysTo(currDateTime) >= 2 &&
             dateTimeEdited.daysTo(currDateTime) <= 7){
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
    isSelected ? updateStyleSheet(m_focusColor, false)
               : updateStyleSheet(m_defaultColor, true);
}

void NoteData::setSelectedWithFocus(bool isSelected, bool focus)
{
    m_isSelected = isSelected;
    if(m_isSelected){
        focus ? setFocus()
              : updateStyleSheet(m_unfocusColor, false);
    }else{
        updateStyleSheet(m_defaultColor, true);
    }
}

void NoteData::onButtonPressed()
{
    setSelected(true);
    emit pressed();
}

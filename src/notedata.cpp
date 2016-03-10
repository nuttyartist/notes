#include "notedata.h"
#include <QVBoxLayout>

NoteData::NoteData(const QString& noteName, QWidget *parent) :
    QWidget(parent),
    m_noteName(noteName),
    m_fakeContainer(new QGroupBox(this)),
    m_containerBox(new QGroupBox(m_fakeContainer)),
    m_button(new QPushButton("", m_fakeContainer)),
    m_titleLabel(new QLabel("", m_fakeContainer)),
    m_dateLabel(new QLabel("", m_fakeContainer)),
    m_seperateLine(new QFrame(m_containerBox)),
    m_scrollBarPosition(0)
{

    setupWidget();
}

NoteData::~NoteData()
{
}

void NoteData::resizeEvent(QResizeEvent *)
{
    m_button->setFixedWidth(this->width() - 18);
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
    m_seperateLine->setObjectName("seperateLine");

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
    this->setFixedHeight(50);
    // title label
    m_titleLabel->setFont(titleLabelFont);
    m_titleLabel->setStyleSheet("QLabel { color : black; }");
    m_titleLabel->resize(this->width() - 1, 0);
    m_titleLabel->setFixedHeight(titleLabelFont.pixelSize() + addToTitleLabelHeight); // + So there would be room for letters like g,y etc..
    m_titleLabel->move(0, distanceToTitleLabel);
    // date label
    m_dateLabel->setFont(dateLabelFont);
    m_dateLabel->resize(this->width(), m_dateLabel->height());
    m_dateLabel->setFixedHeight(dateLabelFont.pixelSize() + addToDateLabelHeight); // + So there would be room for letters like g,y etc..
    m_dateLabel->move(0, m_titleLabel->height() + distanceBetweenEverything*2);
    m_dateLabel->setStyleSheet("QLabel { color : rgb(132, 132, 132); }");
    // separate line
    m_seperateLine->setFrameShape(QFrame::HLine);
    m_seperateLine->setGeometry(0, m_titleLabel->height() + m_dateLabel->height() + distanceBetweenEverything*3, this->width()-1, 1);
    m_seperateLine->setStyleSheet("QFrame { color : rgb(221, 221, 221); }");
    // button
    m_button->setGeometry(0, 0, this->width(), m_titleLabel->height() + m_dateLabel->height() + m_seperateLine->height() + distanceBetweenEverything*3);
    m_button->raise();
    m_button->setStyleSheet("QPushButton { background-color: rgba(254, 206, 9, 0) }");
    m_button->setFlat(true);
    m_button->setFocusPolicy(Qt::NoFocus);
    // fake container
    m_fakeContainer->setFixedHeight(m_titleLabel->height() + m_dateLabel->height() + m_seperateLine->height() + distanceBetweenEverything*3);
    m_fakeContainer->setFlat(true);
    m_fakeContainer->setStyleSheet("QGroupBox { border: none; }");
    // container box
    m_containerBox->resize(this->width(), m_titleLabel->height() + m_dateLabel->height() + m_seperateLine->height() + distanceBetweenEverything*3);
    m_containerBox->installEventFilter(this);

    QVBoxLayout* vLayoutContainerBox = new QVBoxLayout;
    vLayoutContainerBox->setContentsMargins(9,0,0,0);
    vLayoutContainerBox->setSpacing(0);
    vLayoutContainerBox->addWidget(m_titleLabel);
    vLayoutContainerBox->addWidget(m_dateLabel);
    m_containerBox->setLayout(vLayoutContainerBox);

    QVBoxLayout* vLayoutFakeContainer = new QVBoxLayout;
    vLayoutFakeContainer->setContentsMargins(0,0,0,0);
    vLayoutFakeContainer->setSpacing(0);
    vLayoutFakeContainer->addWidget(m_containerBox);
    vLayoutFakeContainer->addWidget(m_seperateLine);
    m_fakeContainer->setLayout(vLayoutFakeContainer);

    QVBoxLayout* vLayoutNote = new QVBoxLayout;
    vLayoutNote->setContentsMargins(0,0,0,0);
    vLayoutNote->setSpacing(0);
    vLayoutNote->addWidget(m_fakeContainer);
    this->setLayout(vLayoutNote);

}

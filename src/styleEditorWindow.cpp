#include "styleEditorWindow.h"
#include "ui_styleEditorWindow.h"

#include <QDebug>
#include <QPushButton>

/**
 * Initializes the window components and configures the StyleEditorWindow
 */
StyleEditorWindow::StyleEditorWindow(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::StyleEditorWindow)
{
    m_ui->setupUi(this);
    this->installEventFilter(this);
    this->setWindowTitle("Editor Settings");
    this->setWindowFlags(Qt::WindowStaysOnTopHint);

    this->setFont(QFont(QFont(QStringLiteral("SF Pro Text")).exactMatch() ? QStringLiteral("SF Pro Text") : QStringLiteral("Roboto")));

//    QString ss = QStringLiteral("QWidget{ "
//                         "  border-radius: 20px;" // Doesn't work!
//                         "  background: rgb(255, 255, 255);"
//                         "} "
//                         );

//    this->setStyleSheet(ss);


    connect(m_ui->serifButton, &QPushButton::clicked, this, [this]{StyleEditorWindow::changeFontType(FontTypeface::Serif);});
    connect(m_ui->sansSerifButton, &QPushButton::clicked, this, [this]{StyleEditorWindow::changeFontType(FontTypeface::SansSerif);});
    connect(m_ui->monoButton, &QPushButton::clicked, this, [this]{StyleEditorWindow::changeFontType(FontTypeface::Mono);});
    connect(m_ui->decreaseButton, &QPushButton::clicked, this, [this]{StyleEditorWindow::changeFontSize(FontSizeAction::Decrease);});
    connect(m_ui->increaseButton, &QPushButton::clicked, this, [this]{StyleEditorWindow::changeFontSize(FontSizeAction::Increase);});
    connect(m_ui->fullWidthButton, &QPushButton::clicked, this, [this]{StyleEditorWindow::changeEditorTextWidth(EditorTextWidth::FullWidth);});
    connect(m_ui->increaseWidthButton, &QPushButton::clicked, this, [this]{StyleEditorWindow::changeEditorTextWidth(EditorTextWidth::Increase);});
    connect(m_ui->decreaseWidthButton, &QPushButton::clicked, this, [this]{StyleEditorWindow::changeEditorTextWidth(EditorTextWidth::Decrease);});
    connect(m_ui->resetDefaultButton, &QPushButton::clicked, this, [this]{StyleEditorWindow::resetEditorToDefaultSettings();});
    connect(m_ui->lightButton, &QPushButton::clicked, this, [this]{StyleEditorWindow::changeThemeColor(ThemeColor::Light);});
    connect(m_ui->darkButton, &QPushButton::clicked, this, [this]{StyleEditorWindow::changeThemeColor(ThemeColor::Dark);});
    connect(m_ui->sepiaButton, &QPushButton::clicked, this, [this]{StyleEditorWindow::changeThemeColor(ThemeColor::Sepia);});
}

StyleEditorWindow::~StyleEditorWindow()
{
    /* Delete UI controls */
    delete m_ui;
}

void StyleEditorWindow::changeSelectedFontLabel(FontTypeface selectedFontType, QString selectedFontName)
{
    switch(selectedFontType) {
    case FontTypeface::Mono:
        m_ui->monoButton->setText(QString("Mono") + QString("\n") + selectedFontName);
        break;
    case FontTypeface::Serif:
        m_ui->serifButton->setText(QString("Serif") + QString("\n") + selectedFontName);
        break;
    case FontTypeface::SansSerif:
        m_ui->sansSerifButton->setText(QString("Sans-serif") + QString("\n") + selectedFontName);
        break;
    }
}

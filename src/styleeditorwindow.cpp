#include "styleeditorwindow.h"
#include "ui_styleeditorwindow.h"

#include <QDebug>
#include <QTimer>
#include <QShortcut>
#include <qstyle.h>
#include <sstream>
#include <QFile>

/**
 * Initializes the window components and configures the StyleEditorWindow
 */
StyleEditorWindow::StyleEditorWindow(QWidget *parent)
    : QDialog(parent, Qt::Tool),
      m_ui(new Ui::StyleEditorWindow),
      m_currentlyClickedButton(nullptr),
      m_currentSelectedFontButton(nullptr),
      m_currentSelectedThemeButton(nullptr),
      m_isFullWidthClicked(false)
{
    m_ui->setupUi(this);
    setWindowTitle(tr("Editor Settings"));

    m_ui->serifButton->setToolTip(
            "Change the text editor font to a serif font.\nClick again to try different fonts");
    m_ui->sansSerifButton->setToolTip("Change the text editor font to a sans-serif font.\nClick "
                                      "again to try different fonts");
    m_ui->monoButton->setToolTip(
            "Change the text editor font to a monospace font.\nClick again to try different fonts");
    m_ui->decreaseButton->setToolTip("Decrease font size");
    m_ui->increaseButton->setToolTip("Increase font size");
    m_ui->fullWidthButton->setToolTip("Make text stretch to the full width of the text editor");
    m_ui->increaseWidthButton->setToolTip("Increase the text width by one character");
    m_ui->decreaseWidthButton->setToolTip("Decrease the text width by one character");
    m_ui->lightButton->setToolTip("Change app theme to Light");
    m_ui->darkButton->setToolTip("Change app theme to Dark");
    m_ui->sepiaButton->setToolTip("Change app theme to Sepia");
    m_ui->resetDefaultButton->setToolTip("Reset all to default settings");

    connect(m_ui->serifButton, &QPushButton::clicked, this, [this] {
        emit StyleEditorWindow::changeFontType(FontTypeface::Serif);
        buttonClicked(m_ui->serifButton);
    });
    connect(m_ui->sansSerifButton, &QPushButton::clicked, this, [this] {
        emit StyleEditorWindow::changeFontType(FontTypeface::SansSerif);
        buttonClicked(m_ui->sansSerifButton);
    });
    connect(m_ui->monoButton, &QPushButton::clicked, this, [this] {
        emit StyleEditorWindow::changeFontType(FontTypeface::Mono);
        buttonClicked(m_ui->monoButton);
    });
    connect(m_ui->decreaseButton, &QPushButton::clicked, this, [this] {
        emit StyleEditorWindow::changeFontSize(FontSizeAction::Decrease);
        buttonClicked(m_ui->decreaseButton);
    });
    connect(m_ui->increaseButton, &QPushButton::clicked, this, [this] {
        emit StyleEditorWindow::changeFontSize(FontSizeAction::Increase);
        buttonClicked(m_ui->increaseButton);
    });
    connect(m_ui->fullWidthButton, &QPushButton::clicked, this, [this] {
        emit StyleEditorWindow::changeEditorTextWidth(EditorTextWidth::FullWidth);
        buttonClicked(m_ui->fullWidthButton);
    });
    connect(m_ui->increaseWidthButton, &QPushButton::clicked, this, [this] {
        emit StyleEditorWindow::changeEditorTextWidth(EditorTextWidth::Increase);
        buttonClicked(m_ui->increaseWidthButton);
    });
    connect(m_ui->decreaseWidthButton, &QPushButton::clicked, this, [this] {
        emit StyleEditorWindow::changeEditorTextWidth(EditorTextWidth::Decrease);
        buttonClicked(m_ui->decreaseWidthButton);
    });
    connect(m_ui->lightButton, &QPushButton::clicked, this, [this] {
        emit StyleEditorWindow::changeTheme(Theme::Light);
        buttonClicked(m_ui->lightButton);
    });
    connect(m_ui->darkButton, &QPushButton::clicked, this, [this] {
        emit StyleEditorWindow::changeTheme(Theme::Dark);
        buttonClicked(m_ui->darkButton);
    });
    connect(m_ui->sepiaButton, &QPushButton::clicked, this, [this] {
        emit StyleEditorWindow::changeTheme(Theme::Sepia);
        buttonClicked(m_ui->sepiaButton);
    });
    connect(m_ui->resetDefaultButton, &QPushButton::clicked, this, [this] {
        emit StyleEditorWindow::resetEditorToDefaultSettings();
        buttonClicked(m_ui->resetDefaultButton);
    });

    QString fontDisplayName;
#ifdef __APPLE__

    if (QFont(QStringLiteral("SF Pro Text")).exactMatch()) {
        fontDisplayName = QStringLiteral("SF Pro Text");
    } else if (QFont(QStringLiteral("Helvetica Neue")).exactMatch()) {
        fontDisplayName = QStringLiteral("Helvetica Neue");
    } else if (QFont(QStringLiteral("Helvetica")).exactMatch()) {
        fontDisplayName = QStringLiteral("Helvetica");
    } else {
        fontDisplayName = QStringLiteral("Roboto");
    }

#elif _WIN32
    fontDisplayName = QFont(QStringLiteral("Segoe UI")).exactMatch() ? QStringLiteral("Segoe UI")
                                                                     : QStringLiteral("Roboto");

#else
    fontDisplayName = QStringLiteral("Roboto");
#endif
    setFont(QFont(fontDisplayName));

#ifdef __APPLE__
    int fontDisplaySize = 13;
#else
    int fontDisplaySize = 9;
#endif

    // load stylesheet
    QFile file(QStringLiteral(":/styles/style-editor-window.css"));
    file.open(QFile::ReadOnly);
    QString styleSheet = QString::fromLatin1(file.readAll());

    // apply stylesheet to all buttons
    QList<QPushButton *> listChildrenButtons = findChildren<QPushButton *>();
    foreach (QPushButton *childButton, listChildrenButtons) {
        childButton->setFont(QFont(fontDisplayName, fontDisplaySize, QFont::Bold));
        childButton->setStyleSheet(styleSheet);
        setButtonStyle(childButton, ButtonState::Normal, m_currentTheme);
    }

    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), this,
                  SLOT(toggleWindowVisibility()));

    m_ui->decreaseButton->setIcon(QIcon(QStringLiteral(":images/minus.png")));
    m_ui->increaseButton->setIcon(QIcon(QStringLiteral(":images/plus.png")));
    QIcon decreaseWidthIcon(QStringLiteral(":images/decrease-width.png"));
    QIcon increaseWidthIcon(QStringLiteral(":images/increase-width.png"));
    m_ui->decreaseWidthButton->setIcon(decreaseWidthIcon);
    m_ui->decreaseWidthButton->setIconSize(
            decreaseWidthIcon.actualSize(m_ui->decreaseWidthButton->size()));
    m_ui->increaseWidthButton->setIcon(increaseWidthIcon);
    m_ui->increaseWidthButton->setIconSize(
            increaseWidthIcon.actualSize(m_ui->increaseWidthButton->size()));
}

StyleEditorWindow::~StyleEditorWindow()
{
    /* Delete UI controls */
    delete m_ui;
}

/*!
 * \brief StyleEditorWindow::buttonClicked
 * Handles the styling of selected buttons
 * (those who needs to keep show they are selected)
 * \param button
 */
void StyleEditorWindow::buttonClicked(QPushButton *button)
{
    m_currentlyClickedButton = button;

    // font buttons
    if (button == m_ui->serifButton || button == m_ui->sansSerifButton
        || button == m_ui->monoButton) {
        if (m_currentSelectedFontButton != nullptr) {
            setButtonStyle(m_currentSelectedFontButton, ButtonState::Normal, m_currentTheme);
        }
        m_currentSelectedFontButton = button;
    }

    // theme buttons
    if (button == m_ui->lightButton || button == m_ui->darkButton || button == m_ui->sepiaButton) {
        if (m_currentSelectedThemeButton != nullptr) {
            setButtonStyle(m_currentSelectedThemeButton, ButtonState::Normal, m_currentTheme);
        }
        m_currentSelectedThemeButton = button;
        setButtonStyle(m_currentSelectedThemeButton, ButtonState::Clicked, m_currentTheme);
    }

    // full width button
    if (button == m_ui->fullWidthButton) {
        if (m_isFullWidthClicked) {
            setButtonStyle(m_ui->fullWidthButton, ButtonState::Normal, m_currentTheme);
            m_isFullWidthClicked = false;
        } else {
            setButtonStyle(m_ui->fullWidthButton, ButtonState::Clicked, m_currentTheme);
            m_isFullWidthClicked = true;
        }
    }

    // increase/decrease width buttons
    if (button == m_ui->increaseWidthButton || button == m_ui->decreaseWidthButton) {
        setButtonStyle(m_ui->fullWidthButton, ButtonState::Normal, m_currentTheme);
        m_isFullWidthClicked = false;
    }

    if (button != m_currentSelectedFontButton && button != m_currentSelectedThemeButton
        && button != m_ui->fullWidthButton) {
        QTimer::singleShot(200, this, [this] {
            setButtonStyle(m_currentlyClickedButton, ButtonState::Normal, m_currentTheme);
        });
    }
}

/*!
 * \brief StyleEditorWindow::changeSelectedFont
 * Called from MainWindow so we can let the style editor window
 * know the font name (known only from the font lists in MainWindow)
 * \param selectedFontType
 * \param selectedFontName
 */
void StyleEditorWindow::changeSelectedFont(FontTypeface selectedFontType,
                                           const QString &selectedFontName)
{
    switch (selectedFontType) {
    case FontTypeface::Mono:
        m_selectedMonoFontFamilyName = selectedFontName;
        m_ui->monoButton->changeFont(selectedFontName, QStringLiteral("mono"));
        break;
    case FontTypeface::Serif:
        m_selectedSerifFontFamilyName = selectedFontName;
        m_ui->serifButton->changeFont(selectedFontName, QStringLiteral("serif"));
        break;
    case FontTypeface::SansSerif:
        m_selectedSansSerifFontFamilyName = selectedFontName;
        m_ui->sansSerifButton->changeFont(selectedFontName, QStringLiteral("sansSerif"));
        break;
    }
}

void StyleEditorWindow::setButtonStyle(QPushButton *button, ButtonState buttonState, Theme theme)
{
    std::ostringstream classes;
    classes << QString::fromStdString(to_string(theme)).toLower().toStdString();
    if (buttonState == ButtonState::Clicked) {
        classes << " selected";
    }
    setCSSClassesAndUpdate(button, classes.str());
}

/*!
 * \brief StyleEditorWindow::isSelectedButton
 * Check if the provided button is currently selected
 * (from those buttons who keep their selected state)
 * \param button
 */
bool StyleEditorWindow::isSelectedButton(QPushButton *button)
{
    return button == m_currentSelectedFontButton || button == m_currentSelectedThemeButton
            || (button == m_ui->fullWidthButton && m_isFullWidthClicked);
}

/*!
 * \brief StyleEditorWindow::setTheme
 * Called from MainWindow to set the appropriate theme and suitable color
 * Also textColor is provided as sometimes we want the font in style editor window
 * to not be the same as the Mainwindow->m_textEdit's text  color (as when Sepia is chosen)
 * (from those buttons who keep their selected state)
 * \param theme
 * \param themeColor
 * \param textColor
 */
void StyleEditorWindow::setTheme(Theme theme)
{
    m_currentTheme = theme;

    if (theme == Theme::Dark) {
        m_ui->decreaseButton->setIcon(QIcon(QStringLiteral(":images/minus-dark.png")));
        m_ui->increaseButton->setIcon(QIcon(QStringLiteral(":images/plus-dark.png")));
        QIcon decreaseWidthIcon(QStringLiteral(":images/decrease-width-dark.png"));
        QIcon increaseWidthIcon(QStringLiteral(":images/increase-width-dark.png"));
        m_ui->decreaseWidthButton->setIcon(decreaseWidthIcon);
        m_ui->decreaseWidthButton->setIconSize(
                decreaseWidthIcon.actualSize(m_ui->decreaseWidthButton->size()));
        m_ui->increaseWidthButton->setIcon(increaseWidthIcon);
        m_ui->increaseWidthButton->setIconSize(
                increaseWidthIcon.actualSize(m_ui->increaseWidthButton->size()));
    } else {
        m_ui->decreaseButton->setIcon(QIcon(QStringLiteral(":images/minus.png")));
        m_ui->increaseButton->setIcon(QIcon(QStringLiteral(":images/plus.png")));
        QIcon decreaseWidthIcon(QStringLiteral(":images/decrease-width.png"));
        QIcon increaseWidthIcon(QStringLiteral(":images/increase-width.png"));
        m_ui->decreaseWidthButton->setIcon(decreaseWidthIcon);
        m_ui->decreaseWidthButton->setIconSize(
                decreaseWidthIcon.actualSize(m_ui->decreaseWidthButton->size()));
        m_ui->increaseWidthButton->setIcon(increaseWidthIcon);
        m_ui->increaseWidthButton->setIconSize(
                increaseWidthIcon.actualSize(m_ui->increaseWidthButton->size()));
    }

    m_ui->monoButton->setTheme(theme);
    m_ui->serifButton->setTheme(theme);
    m_ui->sansSerifButton->setTheme(theme);
    m_ui->monoButton->changeFont(m_selectedMonoFontFamilyName, QStringLiteral("mono"));
    m_ui->serifButton->changeFont(m_selectedSerifFontFamilyName, QStringLiteral("serif"));
    m_ui->sansSerifButton->changeFont(m_selectedSansSerifFontFamilyName,
                                      QStringLiteral("sansSerif"));

    QList<QPushButton *> listChildrenButtons = findChildren<QPushButton *>();
    foreach (QPushButton *childButton, listChildrenButtons) {
        if (!isSelectedButton(childButton)) {
            setButtonStyle(childButton, ButtonState::Normal, theme);
        } else {
            setButtonStyle(childButton, ButtonState::Clicked, theme);
        }
    }
}

/*!
 * \brief StyleEditorWindow::restoreSelectedOptions
 * Called from MainWindow to restore the selected options
 * on app start up, if they exist.
 * \param isTextFullWidth
 * \param selectedFontTypeface
 * \param selectedTheme
 */
void StyleEditorWindow::restoreSelectedOptions(bool isTextFullWidth,
                                               FontTypeface selectedFontTypeface,
                                               Theme selectedTheme)
{
    if (isTextFullWidth) {
        buttonClicked(m_ui->fullWidthButton);
    } else {
        // deselect the button
        m_isFullWidthClicked = false;
        setButtonStyle(m_ui->fullWidthButton, ButtonState::Normal, m_currentTheme);
    }

    switch (selectedFontTypeface) {
    case FontTypeface::Mono:
        buttonClicked(m_ui->monoButton);
        break;
    case FontTypeface::Serif:
        buttonClicked(m_ui->serifButton);
        break;
    case FontTypeface::SansSerif:
        buttonClicked(m_ui->sansSerifButton);
        break;
    }

    switch (selectedTheme) {
    case Theme::Light: {
        buttonClicked(m_ui->lightButton);
        break;
    }
    case Theme::Dark: {
        buttonClicked(m_ui->darkButton);
        break;
    }
    case Theme::Sepia: {
        buttonClicked(m_ui->sepiaButton);
        break;
    }
    }
}

void StyleEditorWindow::toggleWindowVisibility()
{
    if (isVisible()) {
        hide();
    } else {
        show();
    }
}

std::ostream &operator<<(std::ostream &os, const FontTypeface &fontTypeface)
{
    switch (fontTypeface) {
    case FontTypeface::Mono:
        os << "Mono";
        break;
    case FontTypeface::Serif:
        os << "Serif";
        break;
    case FontTypeface::SansSerif:
        os << "SansSerif";
        break;
    }
    return os;
}

std::string to_string(FontTypeface fontTypeface)
{
    std::ostringstream os;
    os << fontTypeface;
    return os.str();
}

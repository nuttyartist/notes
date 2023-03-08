#include "styleeditorwindow.h"
#include "ui_styleeditorwindow.h"

#include <QDebug>
#include <QTimer>
#include <QShortcut>

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
    m_ui->sepiaButton->setToolTip("Chaneg app theme to Sepia");
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

    QString ss = QString("QPushButton {background-color: white;} "
                         "QPushButton {border: none;}"
                         "QPushButton {padding: 0px;}");

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
    m_ui->fullWidthButton->setFont(QFont(fontDisplayName, fontDisplaySize, QFont::Bold));
    m_ui->increaseButton->setFont(QFont(fontDisplayName, fontDisplaySize, QFont::Bold));
    m_ui->decreaseButton->setFont(QFont(fontDisplayName, fontDisplaySize, QFont::Bold));
    m_ui->increaseWidthButton->setFont(QFont(fontDisplayName, fontDisplaySize, QFont::Bold));
    m_ui->decreaseWidthButton->setFont(QFont(fontDisplayName, fontDisplaySize, QFont::Bold));
    m_ui->lightButton->setFont(QFont(fontDisplayName, fontDisplaySize, QFont::Bold));
    m_ui->darkButton->setFont(QFont(fontDisplayName, fontDisplaySize, QFont::Bold));
    m_ui->sepiaButton->setFont(QFont(fontDisplayName, fontDisplaySize, QFont::Bold));
    m_ui->resetDefaultButton->setFont(QFont(fontDisplayName, fontDisplaySize, QFont::Bold));

    QList<QPushButton *> listChildrenButtons = findChildren<QPushButton *>();
    foreach (QPushButton *childButton, listChildrenButtons) {
        childButton->setStyleSheet(ss);
        childButton->installEventFilter(this);
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
    bool shouldBeClicked = true;

    if (button == m_ui->serifButton || button == m_ui->sansSerifButton
        || button == m_ui->monoButton) {
        if (m_currentSelectedFontButton) {
            m_currentSelectedFontButton->setStyleSheet(getStyleSheetForButton(ButtonState::Normal));
        }
        m_currentSelectedFontButton = button;
    }

    if (button == m_ui->lightButton || button == m_ui->darkButton || button == m_ui->sepiaButton) {
        if (m_currentSelectedThemeButton) {
            m_currentSelectedThemeButton->setStyleSheet(
                    getStyleSheetForButton(ButtonState::Normal));
        }
        m_currentSelectedThemeButton = button;
    }

    if (button == m_ui->fullWidthButton) {
        if (m_isFullWidthClicked) {
            m_ui->fullWidthButton->setStyleSheet(getStyleSheetForButton(ButtonState::Hovered));
            m_isFullWidthClicked = false;
            shouldBeClicked = false;
        } else {
            m_ui->fullWidthButton->setStyleSheet(getStyleSheetForButton(ButtonState::Clicked));
            m_isFullWidthClicked = true;
        }
    }

    if (button == m_ui->increaseWidthButton || button == m_ui->decreaseWidthButton) {
        m_ui->fullWidthButton->setStyleSheet(getStyleSheetForButton(ButtonState::Normal));
        m_isFullWidthClicked = false;
    }

    if (shouldBeClicked)
        button->setStyleSheet(getStyleSheetForButton(ButtonState::Clicked));

    if (button != m_currentSelectedFontButton && button != m_currentSelectedThemeButton
        && button != m_ui->fullWidthButton) {
        QTimer::singleShot(200, this, [this] {
            m_currentlyClickedButton->setStyleSheet(getStyleSheetForButton(ButtonState::Normal));
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
        m_ui->monoButton->changeFont(selectedFontName, QStringLiteral("mono"), m_currentFontColor);
        m_ui->monoButton->repaint();
        break;
    case FontTypeface::Serif:
        m_selectedSerifFontFamilyName = selectedFontName;
        m_ui->serifButton->changeFont(selectedFontName, QStringLiteral("serif"),
                                      m_currentFontColor);
        m_ui->serifButton->repaint();
        break;
    case FontTypeface::SansSerif:
        m_selectedSansSerifFontFamilyName = selectedFontName;
        m_ui->sansSerifButton->changeFont(selectedFontName, QStringLiteral("sansSerif"),
                                          m_currentFontColor);
        m_ui->sansSerifButton->repaint();
        break;
    }
}

/*!
 * \brief StyleEditorWindow::getStyleSheetForButton
 * Get the required stylesheet of a button based on the provided button's
 * state: Normal, Hovered, Clicked.
 * \param buttonState
 */
QString StyleEditorWindow::getStyleSheetForButton(ButtonState buttonState)
{
    QString ss = QStringLiteral("QPushButton{ "
                                "  background-color: %1; "
                                "  border: none;"
                                "  padding: 0px;");

    // Font color
    if (m_currentTheme == Theme::Dark) {
        ss.append(QStringLiteral("color: rgb(204, 204, 204);"));
    } else {
        ss.append(QStringLiteral("color: rgb(26, 26, 26);"));
    }

    ss.append("}");

    switch (buttonState) {
    case ButtonState::Normal: {
        ss = ss.arg(m_currentThemeColor.name());
        break;
    }
    case ButtonState::Hovered: {
        if (m_currentTheme == Theme::Dark) {
            ss = ss.arg(QColor(43, 43, 43).name());
        } else {
            ss = ss.arg(QColor(207, 207, 207).name());
        }
        break;
    }
    case ButtonState::Clicked: {
        if (m_currentTheme == Theme::Dark) {
            ss = ss.arg(QColor(0, 59, 148).name());
        } else {
            ss = ss.arg(QColor(218, 233, 239).name());
        }
        break;
    }
    }

    return ss;
}

/*!
 * \brief StyleEditorWindow::isSelectedButton
 * Check if the provided button is currently selected
 * (from those buttons who keep their selected state)
 * \param button
 */
bool StyleEditorWindow::isSelectedButton(QPushButton *button)
{
    if ((button != m_currentSelectedFontButton && button != m_currentSelectedThemeButton)
        && !(button == m_ui->fullWidthButton && m_isFullWidthClicked)) {
        return false;
    }

    return true;
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
void StyleEditorWindow::setTheme(Theme theme, QColor themeColor, QColor textColor)
{
    m_currentTheme = theme;
    m_currentThemeColor = themeColor;
    m_currentFontColor = textColor;

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
    m_ui->monoButton->changeFont(m_selectedMonoFontFamilyName, QStringLiteral("mono"),
                                 m_currentFontColor);
    m_ui->serifButton->changeFont(m_selectedSerifFontFamilyName, QStringLiteral("serif"),
                                  m_currentFontColor);
    m_ui->sansSerifButton->changeFont(m_selectedSansSerifFontFamilyName,
                                      QStringLiteral("sansSerif"), m_currentFontColor);
    m_ui->monoButton->repaint();
    m_ui->serifButton->repaint();
    m_ui->sansSerifButton->repaint();

    QList<QPushButton *> listChildrenButtons = findChildren<QPushButton *>();
    foreach (QPushButton *childButton, listChildrenButtons) {
        if (!isSelectedButton(childButton)) {
            childButton->setStyleSheet(getStyleSheetForButton(ButtonState::Normal));
        } else {
            childButton->setStyleSheet(getStyleSheetForButton(ButtonState::Clicked));
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

/*!
 * \brief StyleEditorWindow::eventFilter
 * Handle buttons event when hovered upon/unhovered
 * and set the appropriate stylesheet for them
 * \param object
 * \param event
 */
bool StyleEditorWindow::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter: {
        QList<QPushButton *> listChildrenButtons = findChildren<QPushButton *>();
        foreach (QPushButton *childButton, listChildrenButtons) {
            if ((object == childButton && !isSelectedButton(childButton))) {
                childButton->setStyleSheet(getStyleSheetForButton(ButtonState::Hovered));
            }
        }
        break;
    }
    case QEvent::Leave: {
        QList<QPushButton *> listChildrenButtons = findChildren<QPushButton *>();
        foreach (QPushButton *childButton, listChildrenButtons) {
            if (object == childButton && !isSelectedButton(childButton)) {
                childButton->setStyleSheet(getStyleSheetForButton(ButtonState::Normal));
            }
        }
        break;
    }
    default:
        break;
    }

    return QObject::eventFilter(object, event);
}

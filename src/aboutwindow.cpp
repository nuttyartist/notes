#include "aboutwindow.h"
#include "ui_aboutwindow.h"

#include <QDebug>
#include <QFile>
#include <QScrollBar>
#include <qstyle.h>
#include <sstream>
#include "editorsettingsoptions.h"

/**
 * Initializes the window components and configures the AboutWindow
 */
AboutWindow::AboutWindow(QWidget *parent)
    : QDialog(parent), m_ui(new Ui::AboutWindow), m_isProVersion(false)
{
    m_ui->setupUi(this);
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
#endif
    setWindowTitle(tr("About") + " " + QCoreApplication::applicationName());

    setAboutText();

    m_ui->aboutText->setTextColor(QColor(26, 26, 26));

#ifdef __APPLE__
    QFont fontToUse = QFont(QStringLiteral("SF Pro Text")).exactMatch()
            ? QStringLiteral("SF Pro Text")
            : QStringLiteral("Roboto");
    m_ui->aboutText->setFont(fontToUse);
#elif _WIN32
    QFont fontToUse = QFont(QStringLiteral("Segoe UI")).exactMatch() ? QStringLiteral("Segoe UI")
                                                                     : QStringLiteral("Roboto");
    m_ui->aboutText->setFont(fontToUse);
#else
    m_ui->aboutText->setFont(QFont(QStringLiteral("Roboto")));
#endif

    // load stylesheet for aboutText
    QFile cssFile(":/styles/about-window.css");
    cssFile.open(QFile::ReadOnly);
    m_ui->aboutText->setStyleSheet(cssFile.readAll());

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    QFile scollBarStyleFile(QStringLiteral(":/styles/components/custom-scrollbar.css"));
    scollBarStyleFile.open(QFile::ReadOnly);
    QString scrollbarStyleSheet = QString::fromLatin1(scollBarStyleFile.readAll());
    m_ui->aboutText->verticalScrollBar()->setStyleSheet(scrollbarStyleSheet);
#endif
}

AboutWindow::~AboutWindow()
{
    /* Delete UI controls */
    delete m_ui;
}

void AboutWindow::setAboutText()
{
    QString proVersionText = m_isProVersion ? " (Pro Version)" : "";

    m_ui->aboutText->setText(
            "<strong>Version:</strong> " + QCoreApplication::applicationVersion() + proVersionText
            + "<p>Notes was founded by <a href='https://rubymamistvalove.com'>Ruby "
              "Mamistvalove</a>, "
              "to create an elegant yet powerful cross-platform and open-source note-taking "
              "app.</p><a href='https://www.get-notes.com'>Notes Website"
              "</a><br/><a href='https://github.com/nuttyartist/notes'>Source code on "
              "Github</a><br/><a "
              "href='https://www.get-notes.com/notes-app-terms-privacy-policy'>Terms and Privacy "
              "Policy</a><br/><br/><strong>Acknowledgments</strong><br/>This project couldn't "
              "have "
              "come this far without the help of these amazing "
              "people:<br/><br/><strong>Programmers:</strong><br/>Alex Spataru<br/>Ali Diouri"
              "<br/>David Planella<br/>Diep Ngoc<br/>Guilherme "
              "Silva<br/>Thorbjørn Lindeijer<br/>Tuur Vanhoutte<br/>Waqar "
              "Ahmed<br/><br/><strong>Designers:</strong><br/>Kevin Doyle<br/><br/>And to the "
              "many of our beloved users who keep sending us feedback, you are an essential force "
              "in "
              "helping us improve, thank you!<br/><br/><strong>Notes makes use of the following "
              "third-party libraries:</strong><br/><br/>QMarkdownTextEdit<br/>"
#if defined(UPDATE_CHECKER)
              "QSimpleUpdater<br/>"
#endif
              "QAutostart<br/>QXT<br/><br/><strong>Notes makes use of the following open source "
              "fonts:</strong><br/><br/>Roboto<br/>Source Sans Pro<br/>Trykker<br/>Mate<br/>iA "
              "Writer Mono<br/>iA Writer Duo<br/>iA Writer "
              "Quattro<br/>Font Awesome<br/>Material Symbols<br/><br/><strong>Qt version:</strong> "
            + qVersion() + " (built with " + QT_VERSION_STR + ")");
}

void AboutWindow::setTheme(Theme::Value theme)
{
    setCSSThemeAndUpdate(m_ui->aboutText, theme);
}

void AboutWindow::setProVersion(bool isProVersion)
{
    m_isProVersion = isProVersion;
    setAboutText();
}

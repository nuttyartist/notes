#include "customDocument.h"
#include <QDebug>
#include <QGuiApplication>
#include <QTextCursor>

CustomDocument::CustomDocument(QWidget *parent) : QTextEdit(parent)
{
    installEventFilter(this);
    viewport()->installEventFilter(this);
}

/*!
 * \brief CustomDocument::setDocumentPadding
 * We use a custom document for MainWindow::m_textEdit
 * so we can set the document padding without the (upstream Qt) issue
 * where the vertical scrollbar gets padded with the text as well.
 * This way, only the text gets padded, and the vertical scroll bar stays where it is.
 * \param left
 * \param top
 * \param right
 * \param bottom
 */
void CustomDocument::setDocumentPadding(int left, int top, int right, int bottom)
{
    setViewportMargins(left, top, right, bottom);
}

void CustomDocument::resizeEvent(QResizeEvent *event)
{
    QTextEdit::resizeEvent(event);
    emit resized();
}

bool CustomDocument::eventFilter(QObject *obj, QEvent *event)
{
    // qDebug() << event->type();

    if (event->type() == QEvent::HoverMove) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);

        // toggle cursor when control key has been pressed or released
        viewport()->setCursor(mouseEvent->modifiers().testFlag(Qt::ControlModifier)
                                      ? Qt::PointingHandCursor
                                      : Qt::IBeamCursor);
    } else if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        // set cursor to pointing hand if control key was pressed
        if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            viewport()->setCursor(Qt::PointingHandCursor);
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {

        auto *mouseEvent = static_cast<QMouseEvent *>(event);

        // track `Ctrl + Click` in the text edit
        if ((obj == viewport()) && (mouseEvent->button() == Qt::LeftButton)
            && (QGuiApplication::keyboardModifiers() == Qt::ExtraButton24)) {
            // open the link (if any) at the current position
            // in the noteTextEdit

            qDebug("Ctrl+Click");
            viewport()->setCursor(Qt::IBeamCursor);

            openLinkAtCursorPosition();

            return true;
        }
    } else if (event->type() == QEvent::KeyRelease) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        // reset cursor if control key was released
        if (keyEvent->key() == Qt::Key_Control) {
            QWidget *viewPort = viewport();
            viewPort->setCursor(Qt::IBeamCursor);
        }
    }

    return QTextEdit::eventFilter(obj, event);
}

/**
 * @brief Returns the markdown url at position
 * @param text
 * @param position
 * @return url string
 */
QString CustomDocument::getMarkdownUrlAtPosition(const QString &text, int position)
{
    QString url;

    // get a map of parsed markdown urls with their link texts as key
    const QMap<QString, QString> urlMap = parseMarkdownUrlsFromText(text);
    QMap<QString, QString>::const_iterator i = urlMap.constBegin();
    for (; i != urlMap.constEnd(); ++i) {
        const QString &linkText = i.key();
        const QString &urlString = i.value();

        const int foundPositionStart = text.indexOf(linkText);

        if (foundPositionStart >= 0) {
            // calculate end position of found linkText
            const int foundPositionEnd = foundPositionStart + linkText.size();

            // check if position is in found string range
            if ((position >= foundPositionStart) && (position <= foundPositionEnd)) {
                url = urlString;
                break;
            }
        }
    }

    return url;
}

/**
 * @brief Opens the link (if any) at the current cursor position
 */
bool CustomDocument::openLinkAtCursorPosition()
{
    QTextCursor cursor = textCursor();
    const int clickedPosition = cursor.position();

    // select the text in the clicked block and find out on
    // which position we clicked
    cursor.movePosition(QTextCursor::StartOfBlock);
    const int positionFromStart = clickedPosition - cursor.position();
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    const QString selectedText = cursor.selectedText();

    // find out which url in the selected text was clicked
    const QString urlString = getMarkdownUrlAtPosition(selectedText, positionFromStart);

    const QUrl url = QUrl(urlString);
    const bool isFileUrl = urlString.startsWith(QLatin1String("file://"));

    const bool isLegacyAttachmentUrl = urlString.startsWith(QLatin1String("file://attachments"));
    const bool isLocalFilePath = urlString.startsWith(QLatin1String("/"));

    const bool convertLocalFilepathsToURLs = true;

    // Q_EMIT urlClicked(urlString);

    if ((url.isValid() && isValidUrl(urlString)) || isFileUrl || isLocalFilePath
        || isLegacyAttachmentUrl) {

        if (_ignoredClickUrlSchemata.contains(url.scheme())) {
            qDebug() << __func__ << "ignored URL scheme:" << urlString;
            return false;
        }

        // ignore non-existent files
        if (isFileUrl) {
            QString trimmed = urlString.mid(7);
            if (!QFile::exists(trimmed)) {
                qDebug() << __func__ << "file does not exist:" << urlString;
                return false;
            }
        }

        if (isLocalFilePath && !QFile::exists(urlString)) {
            qDebug() << __func__ << "file does not exist:" << urlString;
            return false;
        }

        if (isLocalFilePath && convertLocalFilepathsToURLs) {
            openUrl(QString("file://") + urlString);
        } else {
            openUrl(urlString);
        }

        return true;
    }

    return false;
}

/**
 * Checks if urlString is a valid url
 *
 * @param urlString
 * @return
 */
bool CustomDocument::isValidUrl(const QString &urlString)
{
    const QRegularExpressionMatch match = QRegularExpression(R"(^\w+:\/\/.+)").match(urlString);
    return match.hasMatch();
}

/**
 * Handles clicked urls
 *
 * examples:
 * - <https://www.qownnotes.org> opens the webpage
 * - <file:///path/to/my/file/QOwnNotes.pdf> opens the file
 *   "/path/to/my/file/QOwnNotes.pdf" if the operating system supports that
 *  handler
 */
void CustomDocument::openUrl(const QString &urlString)
{
    qDebug() << "CustomDocument " << __func__ << " - 'urlString': " << urlString;

    QDesktopServices::openUrl(QUrl(urlString));
}

/**
 * @brief Returns a map of parsed markdown urls with their link texts as key
 *
 * @param text
 * @return parsed urls
 */
QMap<QString, QString> CustomDocument::parseMarkdownUrlsFromText(const QString &text)
{
    QMap<QString, QString> urlMap;
    QRegularExpression regex;
    QRegularExpressionMatchIterator iterator;

    // match urls like this: <http://mylink>
    //    re = QRegularExpression("(<(.+?:\\/\\/.+?)>)");
    regex = QRegularExpression(QStringLiteral("(<(.+?)>)"));
    iterator = regex.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString linkText = match.captured(1);
        QString url = match.captured(2);
        urlMap[linkText] = url;
    }

    regex = QRegularExpression(R"((\[.*?\]\((.+?)\)))");
    iterator = regex.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString linkText = match.captured(1);
        QString url = match.captured(2);
        urlMap[linkText] = url;
    }

    // match urls like this: http://mylink
    regex = QRegularExpression(R"(\b\w+?:\/\/[^\s]+[^\s>\)])");
    iterator = regex.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString url = match.captured(0);
        urlMap[url] = url;
    }

    // match urls like this: www.github.com
    regex = QRegularExpression(R"(\bwww\.[^\s]+\.[^\s]+\b)");
    iterator = regex.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString url = match.captured(0);
        urlMap[url] = QStringLiteral("http://") + url;
    }

    // match reference urls like this: [this url][1] with this later:
    // [1]: http://domain
    regex = QRegularExpression(R"((\[.*?\]\[(.+?)\]))");
    iterator = regex.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString linkText = match.captured(1);
        QString referenceId = match.captured(2);

        QRegularExpression refRegExp(QStringLiteral("\\[") + QRegularExpression::escape(referenceId)
                                     + QStringLiteral("\\]: (.+)"));
        QRegularExpressionMatch urlMatch = refRegExp.match(toPlainText());

        if (urlMatch.hasMatch()) {
            QString url = urlMatch.captured(1);
            urlMap[linkText] = url;
        }
    }

    return urlMap;
}

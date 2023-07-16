#include "customDocument.h"
#include <QDebug>
#include <QGuiApplication>
#include <QTextCursor>
#include <QMessageBox>

CustomDocument::CustomDocument(QWidget *parent) : QTextEdit(parent)
{
    installEventFilter(this);
    viewport()->installEventFilter(this);
    setMouseTracking(true);
    setAttribute(Qt::WidgetAttribute::WA_Hover, true);
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

void CustomDocument::mouseMoveEvent(QMouseEvent *event)
{
    QTextEdit::mouseMoveEvent(event);
    emit mouseMoved();
}

bool CustomDocument::eventFilter(QObject *obj, QEvent *event)
{
    // qDebug() << event->type();

    if (event->type() == QEvent::HoverMove) {
        // if hovering and the control key is active, check whether the mouse is over a link
        if (QGuiApplication::keyboardModifiers() == Qt::ExtraButton24
            && getUrlUnderMouse().isValid()) {
            viewport()->setCursor(Qt::PointingHandCursor);
        } else {
            viewport()->setCursor(Qt::IBeamCursor);
        }
    } else if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Control) {
            // check if mouse is over a link
            auto url = getUrlUnderMouse();
            viewport()->setCursor(url.isValid() ? Qt::PointingHandCursor : Qt::IBeamCursor);
        } else if (keyEvent->modifiers().testFlag(Qt::AltModifier)) {
            // alt + arrow up/down
            if (keyEvent->key() == Qt::Key_Up) {
                moveBlockUp();
                return true;
            } else if (keyEvent->key() == Qt::Key_Down) {
                moveBlockDown();
                return true;
            }
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {

        auto *mouseEvent = static_cast<QMouseEvent *>(event);

        // track `Ctrl + Click` in the text edit
        if ((obj == viewport()) && (mouseEvent->button() == Qt::LeftButton)
            && (QGuiApplication::keyboardModifiers() == Qt::ExtraButton24)) {
            // open the link (if any) at the current position
            // in the noteTextEdit

            viewport()->setCursor(Qt::IBeamCursor);

            openLinkAtCursorPosition();

            return true;
        }
    } else if (event->type() == QEvent::KeyRelease) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        // reset cursor if control key was released
        if (keyEvent->key() == Qt::Key_Control) {
            viewport()->setCursor(Qt::IBeamCursor);
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
            if ((position >= foundPositionStart) && (position < foundPositionEnd)) {
                url = urlString;
                break;
            }
        }
    }

    return url;
}

/**
 * @brief Returns the URL under the current mouse cursor
 *
 * @return QUrl
 */
QUrl CustomDocument::getUrlUnderMouse()
{
    // place a temp cursor at the mouse position
    auto pos = viewport()->mapFromGlobal(QCursor::pos());
    QTextCursor cursor = cursorForPosition(pos);
    const int cursorPosition = cursor.position();

    // select the text of the current block
    cursor.movePosition(QTextCursor::StartOfBlock);
    const int indexInBlock = cursorPosition - cursor.position();
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    // get the correct link from the selected text, or an empty URL if none found
    return QUrl(getMarkdownUrlAtPosition(cursor.selectedText(), indexInBlock));
}

/**
 * @brief Opens the link (if any) at the current cursor position
 */
bool CustomDocument::openLinkAtCursorPosition()
{
    // find out which url in the selected text was clicked
    QUrl const url = getUrlUnderMouse();
    QString const urlString = url.toString();

    const bool isFileUrl = urlString.startsWith(QLatin1String("file://"));

    const bool isLegacyAttachmentUrl = urlString.startsWith(QLatin1String("file://attachments"));
    const bool isLocalFilePath = urlString.startsWith(QLatin1String("/"));

    const bool convertLocalFilepathsToURLs = true;

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
                qDebug() << __func__ << ": File does not exist:" << urlString;
                // show a message box
                QMessageBox::warning(
                        nullptr, tr("File not found"),
                        tr("The file <strong>%1</strong> does not exist.").arg(trimmed));
                return false;
            }
        }

        if (isLocalFilePath && !QFile::exists(urlString)) {
            qDebug() << __func__ << ": File does not exist:" << urlString;
            // show a message box
            QMessageBox::warning(nullptr, tr("File not found"),
                                 tr("The file <strong>%1</strong> does not exist.").arg(urlString));
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

void CustomDocument::moveBlockUp()
{
    QTextCursor cursor = textCursor();

    if (cursor.blockNumber() > 0) {
        QString currentBlock = cursor.block().text();
        const int currentHorizontalPosition = cursor.positionInBlock();

        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        // also remove empty line
        cursor.deletePreviousChar();

        // cursor is now at end of prev line, move to start
        if (!cursor.movePosition(QTextCursor::StartOfBlock)) {
            // block above is empty, this is fine
        }
        // insert the removed block again
        cursor.insertText(currentBlock);
        cursor.insertBlock();

        // move cursor to previous block
        if (!cursor.movePosition(QTextCursor::PreviousBlock)) {
            qDebug() << "Could not move to previous block";
        }
        const int startPosition = cursor.position();
        cursor.setPosition(startPosition + currentHorizontalPosition);

        setTextCursor(cursor);
    }
}

void CustomDocument::moveBlockDown()
{
    QTextCursor cursor = textCursor();

    QString currentBlock = cursor.block().text();
    const int currentHorizontalPosition = cursor.positionInBlock();

    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    // also remove empty line
    cursor.deleteChar();

    if (!cursor.movePosition(QTextCursor::EndOfBlock)) {
        // block below is empty, this is fine
    }
    // insert the removed block again
    cursor.insertBlock();
    cursor.insertText(currentBlock);

    // move cursor to next block
    if (!cursor.movePosition(QTextCursor::StartOfBlock)) {
        qDebug() << "Could not move to start of next block";
    }
    const int startPosition = cursor.position();
    cursor.setPosition(startPosition + currentHorizontalPosition);

    setTextCursor(cursor);
}

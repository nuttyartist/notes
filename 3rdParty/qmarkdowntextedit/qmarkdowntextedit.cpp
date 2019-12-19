/*
 * Copyright (c) 2014-2019 Patrizio Bekerle -- http://www.bekerle.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 */


#include "qmarkdowntextedit.h"
#include <QKeyEvent>
#include <QGuiApplication>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QDir>
#include <QDesktopServices>
#include <QLayout>
#include <QTimer>
#include <QSettings>
#include <QTextBlock>
#include <QPainter>
#include <QScrollBar>
#include <QClipboard>
#include <utility>


QMarkdownTextEdit::QMarkdownTextEdit(QWidget *parent, bool initHighlighter)
        : QPlainTextEdit(parent) {
    installEventFilter(this);
    viewport()->installEventFilter(this);
    _autoTextOptions = AutoTextOption::BracketClosing;

    _openingCharacters = QStringList() << "(" << "[" << "{" << "<" << "*"
                                       << "\"" << "'" << "_" << "~";
    _closingCharacters = QStringList() << ")" << "]" << "}" << ">" << "*"
                                       << "\"" << "'" << "_" << "~";

    // markdown highlighting is enabled by default
    _highlightingEnabled = true;
    if (initHighlighter) {
        _highlighter = new MarkdownHighlighter(document());
    }
//    setHighlightingEnabled(true);

    QFont font = this->font();

    // set the tab stop to the width of 4 spaces in the editor
    const int tabStop = 4;
    QFontMetrics metrics(font);

#if QT_VERSION < QT_VERSION_CHECK(5,11,0)
    setTabStopWidth(tabStop * metrics.width(' '));
#else
    setTabStopDistance(tabStop * metrics.horizontalAdvance(' '));
#endif

    // add shortcuts for duplicating text
//    new QShortcut( QKeySequence( "Ctrl+D" ), this, SLOT( duplicateText() ) );
//    new QShortcut( QKeySequence( "Ctrl+Alt+Down" ), this, SLOT( duplicateText() ) );

    // add a layout to the widget
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setMargin(0);
    layout->addStretch();
    this->setLayout(layout);

    // add the hidden search widget
    _searchWidget = new QPlainTextEditSearchWidget(this);
    this->layout()->addWidget(_searchWidget);

    QObject::connect(this, SIGNAL(textChanged()),
                     this, SLOT(adjustRightMargin()));
    QObject::connect(this, SIGNAL(cursorPositionChanged()),
                     this, SLOT(centerTheCursor()));

    updateSettings();

    // workaround for disabled signals up initialization
    QTimer::singleShot(300, this, SLOT(adjustRightMargin()));
}

/**
 * Enables or disables the markdown highlighting
 *
 * @param enabled
 */
void QMarkdownTextEdit::setHighlightingEnabled(bool enabled) {
    if (_highlightingEnabled == enabled) {
        return;
    }

    _highlighter->setDocument(enabled ? document() : Q_NULLPTR);
    _highlightingEnabled = enabled;

    if (enabled) {
        _highlighter->rehighlight();
    }
}

/**
 * Leave a little space on the right side if the document is too long, so
 * that the search buttons don't get visually blocked by the scroll bar
 */
void QMarkdownTextEdit::adjustRightMargin() {
    QMargins margins = layout()->contentsMargins();
    int rightMargin = document()->size().height() >
                      viewport()->size().height() ? 24 : 0;
    margins.setRight(rightMargin);
    layout()->setContentsMargins(margins);
}

bool QMarkdownTextEdit::eventFilter(QObject *obj, QEvent *event) {
    //qDebug() << event->type();
    if (event->type() == QEvent::HoverMove) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);

        QWidget *viewPort = this->viewport();
        // toggle cursor when control key has been pressed or released
        viewPort->setCursor(mouseEvent->modifiers().testFlag(
                Qt::ControlModifier) ?
                            Qt::PointingHandCursor :
                            Qt::IBeamCursor);
    } else if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        // set cursor to pointing hand if control key was pressed
        if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            QWidget *viewPort = this->viewport();
            viewPort->setCursor(Qt::PointingHandCursor);
        }

        // disallow keys if text edit hasn't focus
        if (!this->hasFocus()) {
            return true;
        }

        if ((keyEvent->key() == Qt::Key_Escape) && _searchWidget->isVisible()) {
            _searchWidget->deactivate();
            return true;
        } else if ((keyEvent->key() == Qt::Key_Tab) ||
                 (keyEvent->key() == Qt::Key_Backtab)) {
            // handle entered tab and reverse tab keys
            return handleTabEntered(
                    keyEvent->key() == Qt::Key_Backtab);
        } else if ((keyEvent->key() == Qt::Key_F) &&
                 keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            _searchWidget->activate();
            return true;
        } else if ((keyEvent->key() == Qt::Key_R) &&
                 keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            _searchWidget->activateReplace();
            return true;
//        } else if (keyEvent->key() == Qt::Key_Delete) {
        } else if (keyEvent->key() == Qt::Key_Backspace) {
            return handleBracketRemoval();
        } else if (keyEvent->key() == Qt::Key_Asterisk) {
            return handleBracketClosing("*");
        } else if (keyEvent->key() == Qt::Key_QuoteDbl) {
            return quotationMarkCheck("\"");
            // apostrophe bracket closing is temporary disabled because
            // apostrophes are used in different contexts
//        } else if (keyEvent->key() == Qt::Key_Apostrophe) {
//            return handleBracketClosing("'");
            // underline bracket closing is temporary disabled because
            // underlines are used in different contexts
//        } else if (keyEvent->key() == Qt::Key_Underscore) {
//            return handleBracketClosing("_");
        }
        else if (keyEvent->key() == Qt::Key_QuoteLeft) {
            return quotationMarkCheck("`");
        } else if (keyEvent->key() == Qt::Key_AsciiTilde) {
            return handleBracketClosing("~");
#ifdef Q_OS_MAC
        } else if (keyEvent->modifiers().testFlag(Qt::AltModifier) &&
                    keyEvent->key() == Qt::Key_ParenLeft) {
            // bracket closing for US keyboard on macOS
            return handleBracketClosing("{", "}");
#endif
        } else if (keyEvent->key() == Qt::Key_ParenLeft) {
            return handleBracketClosing("(", ")");
        } else if (keyEvent->key() == Qt::Key_BraceLeft) {
            return handleBracketClosing("{", "}");
        } else if (keyEvent->key() == Qt::Key_BracketLeft) {
            return handleBracketClosing("[", "]");
        } else if (keyEvent->key() == Qt::Key_Less) {
            return handleBracketClosing("<", ">");
#ifdef Q_OS_MAC
        } else if (keyEvent->modifiers().testFlag(Qt::AltModifier) &&
                   keyEvent->key() == Qt::Key_ParenRight) {
            // bracket closing for US keyboard on macOS
            return bracketClosingCheck("{", "}");
#endif
        } else if (keyEvent->key() == Qt::Key_ParenRight) {
            return bracketClosingCheck("(", ")");
        } else if (keyEvent->key() == Qt::Key_BraceRight) {
            return bracketClosingCheck("{", "}");
        } else if (keyEvent->key() == Qt::Key_BracketRight) {
            return bracketClosingCheck("[", "]");
        } else if (keyEvent->key() == Qt::Key_Return &&
        keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
            QTextCursor cursor = this->textCursor();
            cursor.insertText("  \n");
            return true;
        } else if (keyEvent->key() == Qt::Key_Return &&
                   keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            QTextCursor cursor = this->textCursor();
            cursor.movePosition(QTextCursor::EndOfLine);
            cursor.insertText("\n");
            setTextCursor(cursor);
            return true;
        } else if (keyEvent == QKeySequence::Copy || keyEvent == QKeySequence::Cut) {
            QTextCursor cursor = this->textCursor();
            if (!cursor.hasSelection()) {
                QString text;
                if (cursor.block().length() <= 1) // no content
                    text = "\n";
                else {
                    //cursor.select(QTextCursor::BlockUnderCursor); // negative, it will include the previous paragraph separator
                    cursor.movePosition(QTextCursor::StartOfBlock);
                    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                    text = cursor.selectedText();
                    if (!cursor.atEnd()) {
                        text += "\n";
                        // this is the paragraph separator
                        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 1);
                    }
                }
                if (keyEvent == QKeySequence::Cut) {
                    if (!cursor.atEnd() && text == "\n")
                        cursor.deletePreviousChar();
                    else
                        cursor.removeSelectedText();
                    cursor.movePosition(QTextCursor::StartOfLine);
                    setTextCursor(cursor);
                }
                qApp->clipboard()->setText(text);
                return true;
            }
        }
        else if (keyEvent == QKeySequence::Paste) {
            if (qApp->clipboard()->ownsClipboard() &&
                QRegExp("[^\n]*\n$").exactMatch(qApp->clipboard()->text())) {
                QTextCursor cursor = this->textCursor();
                if (!cursor.hasSelection()) {
                    cursor.movePosition(QTextCursor::StartOfLine);
                    setTextCursor(cursor);
                }
            }
        } else if ((keyEvent->key() == Qt::Key_Down) &&
                 keyEvent->modifiers().testFlag(Qt::ControlModifier) &&
                 keyEvent->modifiers().testFlag(Qt::AltModifier)) {
            // duplicate text with `Ctrl + Alt + Down`
            duplicateText();
            return true;
#ifndef Q_OS_MAC
        } else if ((keyEvent->key() == Qt::Key_Down) &&
                 keyEvent->modifiers().testFlag(Qt::ControlModifier) &&
                 !keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
            // scroll the page down
            auto *scrollBar = verticalScrollBar();
            scrollBar->setSliderPosition(scrollBar->sliderPosition() + 1);
            return true;
        } else if ((keyEvent->key() == Qt::Key_Up) &&
                 keyEvent->modifiers().testFlag(Qt::ControlModifier) &&
                 !keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
            // scroll the page up
            auto *scrollBar = verticalScrollBar();
            scrollBar->setSliderPosition(scrollBar->sliderPosition() - 1);
            return true;
#endif
        } else if ((keyEvent->key() == Qt::Key_Down) &&
                keyEvent->modifiers().testFlag(Qt::NoModifier)) {
            // if you are in the last line and press cursor down the cursor will
            // jump to the end of the line
            QTextCursor cursor = textCursor();
            if (cursor.position() >= document()->lastBlock().position()) {
                cursor.movePosition(QTextCursor::EndOfLine);
                setTextCursor(cursor);
            }
            return false;
        } else if ((keyEvent->key() == Qt::Key_Up) &&
                   keyEvent->modifiers().testFlag(Qt::NoModifier)) {
            // if you are in the first line and press cursor up the cursor will
            // jump to the start of the line
            QTextCursor cursor = textCursor();
            QTextBlock block = document()->firstBlock();
            int endOfFirstLinePos = block.position() + block.length();

            if (cursor.position() <= endOfFirstLinePos) {
                cursor.movePosition(QTextCursor::StartOfLine);
                setTextCursor(cursor);
            }
            return false;
        } else if (keyEvent->key() == Qt::Key_Return) {
            return handleReturnEntered();
        } else if ((keyEvent->key() == Qt::Key_F3)) {
            _searchWidget->doSearch(
                    !keyEvent->modifiers().testFlag(Qt::ShiftModifier));
            return true;
        } else if ((keyEvent->key() == Qt::Key_Z) &&
                   (keyEvent->modifiers().testFlag(Qt::ControlModifier)) &&
                   !(keyEvent->modifiers().testFlag(Qt::ShiftModifier))) {
            undo();
            return true;
        } else if ((keyEvent->key() == Qt::Key_Down) &&
                   (keyEvent->modifiers().testFlag(Qt::ControlModifier)) &&
                   (keyEvent->modifiers().testFlag(Qt::ShiftModifier))) {
            moveTextUpDown(false);
            return true;
        } else if ((keyEvent->key() == Qt::Key_Up) &&
                   (keyEvent->modifiers().testFlag(Qt::ControlModifier)) &&
                   (keyEvent->modifiers().testFlag(Qt::ShiftModifier))) {
            moveTextUpDown(true);
            return true;
        }

        return false;
    } else if (event->type() == QEvent::KeyRelease) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        // reset cursor if control key was released
        if (keyEvent->key() == Qt::Key_Control) {
            resetMouseCursor();
        }

        return false;
    } else if (event->type() == QEvent::MouseButtonRelease) {
        _mouseButtonDown = false;
        auto *mouseEvent = static_cast<QMouseEvent *>(event);

        // track `Ctrl + Click` in the text edit
        if ((obj == this->viewport()) &&
            (mouseEvent->button() == Qt::LeftButton) &&
            (QGuiApplication::keyboardModifiers() == Qt::ExtraButton24)) {
            // open the link (if any) at the current position
            // in the noteTextEdit
            openLinkAtCursorPosition();
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        _mouseButtonDown = true;
    } else if (event->type() == QEvent::MouseButtonDblClick) {
        _mouseButtonDown = true;
    }

    return QPlainTextEdit::eventFilter(obj, event);
}

void QMarkdownTextEdit::centerTheCursor() {
    if (_mouseButtonDown || !_centerCursor) {
        return;
    }

    // centers the cursor every time, but not on the top and bottom
    // bottom is done by setCenterOnScroll() in updateSettings()
    centerCursor();

/*
    QRect cursor = cursorRect();
    QRect vp = viewport()->rect();

    qDebug() << __func__ << " - 'cursor.top': " << cursor.top();
    qDebug() << __func__ << " - 'cursor.bottom': " << cursor.bottom();
    qDebug() << __func__ << " - 'vp': " << vp.bottom();

    int bottom = 0;
    int top = 0;

    qDebug() << __func__ << " - 'viewportMargins().top()': "
             << viewportMargins().top();

    qDebug() << __func__ << " - 'viewportMargins().bottom()': "
             << viewportMargins().bottom();

    int vpBottom = viewportMargins().top() + viewportMargins().bottom() + vp.bottom();
    int vpCenter = vpBottom / 2;
    int cBottom = cursor.bottom() + viewportMargins().top();

    qDebug() << __func__ << " - 'vpBottom': " << vpBottom;
    qDebug() << __func__ << " - 'vpCenter': " << vpCenter;
    qDebug() << __func__ << " - 'cBottom': " << cBottom;


    if (cBottom >= vpCenter) {
        bottom = cBottom + viewportMargins().top() / 2 + viewportMargins().bottom() / 2 - (vp.bottom() / 2);
//        bottom = cBottom - (vp.bottom() / 2);
//        bottom *= 1.5;
    }

//    setStyleSheet(QString("QPlainTextEdit {padding-bottom: %1px;}").arg(QString::number(bottom)));

//    if (cursor.top() < (vp.bottom() / 2)) {
//        top = (vp.bottom() / 2) - cursor.top() + viewportMargins().top() / 2 + viewportMargins().bottom() / 2;
////        top *= -1;
////        bottom *= 1.5;
//    }
    qDebug() << __func__ << " - 'top': " << top;
    qDebug() << __func__ << " - 'bottom': " << bottom;
    setViewportMargins(0,top,0, bottom);


//    QScrollBar* scrollbar = verticalScrollBar();
//
//    qDebug() << __func__ << " - 'scrollbar->value();': " << scrollbar->value();;
//    qDebug() << __func__ << " - 'scrollbar->maximum();': "
//             << scrollbar->maximum();;


//    scrollbar->setValue(scrollbar->value() - offset.y());
//
//    setViewportMargins

//    setViewportMargins(0, 0, 0, bottom);
*/
}

/*
 * Handle the undo event ourselves
 * Retains the selected text as selected after undo if
 * bracket closing was used otherwise performs normal undo
 */
void QMarkdownTextEdit::undo()
{
    QTextCursor cursor = textCursor();
    //if no text selected, call undo
    if(!cursor.hasSelection()) {
       QPlainTextEdit::undo();
       return;
    }

    //if text is selected and bracket closing was used
    //we retain our selection
    if (handleBracketClosingUsed) {
        //get the selection
        int selectionEnd = cursor.selectionEnd();
        int selectionStart = cursor.selectionStart();
        //call undo
        QPlainTextEdit::undo();
        //select again
        cursor.setPosition(selectionStart-1);
        cursor.setPosition(selectionEnd-1, QTextCursor::KeepAnchor);
        this->setTextCursor(cursor);
        handleBracketClosingUsed = false;
        return;
    //else if text was selected but bracket closing wasn't used
    //do normal undo
    } else {
        QPlainTextEdit::undo();
        return;
    }
}

void QMarkdownTextEdit::moveTextUpDown(bool up){

    QTextCursor cursor = textCursor();
    QTextCursor move = cursor;

    move.setVisualNavigation(false);

    move.beginEditBlock();     //open an edit block to keep undo operations sane
    bool hasSelection = cursor.hasSelection();

    if (hasSelection) {
        //if there's a selection inside the block, we select the whole block
        move.setPosition(cursor.selectionStart());
        move.movePosition(QTextCursor::StartOfBlock);
        move.setPosition(cursor.selectionEnd(), QTextCursor::KeepAnchor);
        move.movePosition(move.atBlockStart() ? QTextCursor::Left: QTextCursor::EndOfBlock,
                          QTextCursor::KeepAnchor);
    } else {
        move.movePosition(QTextCursor::StartOfBlock);
        move.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    }

    //get the text of the current block
    QString text = move.selectedText();

    move.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    move.removeSelectedText();

    if (up) { // up key
        move.movePosition(QTextCursor::PreviousBlock);
        move.insertBlock();
        move.movePosition(QTextCursor::Left);
    } else { //down key
        move.movePosition(QTextCursor::EndOfBlock);
        if (move.atBlockStart()) { // empty block
            move.movePosition(QTextCursor::NextBlock);
            move.insertBlock();
            move.movePosition(QTextCursor::Left);
        } else {
            move.insertBlock();
        }
    }

    int start = move.position();
    move.clearSelection();
    move.insertText(text);
    int end = move.position();

    //reselect
    if (hasSelection) {
        move.setPosition(end);
        move.setPosition(start, QTextCursor::KeepAnchor);
    } else {
        move.setPosition(start);
    }

    move.endEditBlock();

    setTextCursor(move);
}

/**
 * Resets the cursor to Qt::IBeamCursor
 */
void QMarkdownTextEdit::resetMouseCursor() const {
    QWidget *viewPort = viewport();
    viewPort->setCursor(Qt::IBeamCursor);
}

/**
 * Resets the cursor to Qt::IBeamCursor if the widget looses the focus
 */
void QMarkdownTextEdit::focusOutEvent(QFocusEvent *event) {
    resetMouseCursor();
    QPlainTextEdit::focusOutEvent(event);
}

/**
 * Enters a closing character after an opening character if needed
 *
 * @param openingCharacter
 * @param closingCharacter
 * @return
 */
bool QMarkdownTextEdit::handleBracketClosing(const QString& openingCharacter,
                                             QString closingCharacter) {
    // check if bracket closing or read-only are enabled
    if (!(_autoTextOptions & AutoTextOption::BracketClosing) || isReadOnly()) {
        return false;
    }

    QTextCursor cursor = textCursor();

    // get the current text from the block (inserted character not included)
    QString text = cursor.block().text();

    if (closingCharacter.isEmpty()) {
        closingCharacter = openingCharacter;
    }

    QString selectedText = cursor.selectedText();

    // When user currently has text selected, we prepend the openingCharacter
    // and append the closingCharacter. E.g. 'text' -> '(text)'. We keep the
    // current selectedText selected.
    if (selectedText != "") {
        // Insert. The selectedText is overwritten.
        QString newText = openingCharacter + selectedText + closingCharacter;
        cursor.insertText(newText);

        // Re-select the selectedText.
        int selectionEnd = cursor.position() - 1;
        int selectionStart = selectionEnd - selectedText.length();

        cursor.setPosition(selectionStart);
        cursor.setPosition(selectionEnd, QTextCursor::KeepAnchor);
        this->setTextCursor(cursor);
        handleBracketClosingUsed = true;
        return true;
    } else {
        // if not text was selected check if we are inside the text
        int positionInBlock = cursor.position() - cursor.block().position();

        // only allow the closing if the cursor was at the end of a block
        // we are making a special allowance for openingCharacter == *
        if ((positionInBlock != text.count()) &&
            !((openingCharacter == "*") &&
              (positionInBlock == (text.count() - 1)))) {
            return false;
        }
    }

    // Remove whitespace at start of string (e.g. in multilevel-lists).
    text = text.remove(QRegExp("^\\s+"));

    // Default positions to move the cursor back.
    int cursorSubtract = 1;

    // Special handling for `*` opening character, as this could be:
    // - start of a list (or sublist);
    // - start of a bold text;
    if (openingCharacter == "*") {
        // User wants: '*'.
        // This could be the start of a list, don't autocomplete.
        if (text == "") {
            return false;
        }
        // User wants: '**'.
        // Not the start of a list, probably bold text. We autocomplete with
        // extra closingCharacter and cursorSubtract to 'catchup'.
        else if (text == "*") {
            closingCharacter = "**";
            cursorSubtract = 2;
        }
        // User wants: '* *'.
        // We are in a list already, proceed as normal (autocomplete).
        else if (text == "* ") {
            // no-op.
        }
    }

    // Auto completion for ``` pair
    if (openingCharacter == "`") {
        if (QRegExp("[^`]*``").exactMatch(text)) {
            cursor.insertText(openingCharacter);
            cursor.insertText(openingCharacter);
            cursorSubtract = 3;
        }
    }

    cursor.insertText(openingCharacter);
    cursor.insertText(closingCharacter);
    cursor.setPosition(cursor.position() - cursorSubtract);
    setTextCursor(cursor);
    return true;
}

/**
 * Checks if the closing character should be output or not
 *
 * @param openingCharacter
 * @param closingCharacter
 * @return
 */
bool QMarkdownTextEdit::bracketClosingCheck(const QString& openingCharacter,
                                            QString closingCharacter) {
    // check if bracket closing or read-only are enabled
    if (!(_autoTextOptions & AutoTextOption::BracketClosing) || isReadOnly()) {
        return false;
    }

    if (closingCharacter.isEmpty()) {
        closingCharacter = openingCharacter;
    }

    QTextCursor cursor = textCursor();
    int positionInBlock = cursor.position() - cursor.block().position();

    // get the current text from the block
    QString text = cursor.block().text();
    int textLength = text.length();

    // if we are at the end of the line we just want to enter the character
    if (positionInBlock >= textLength) {
        return false;
    }

    QString currentChar = text.at(positionInBlock);

    if (closingCharacter == openingCharacter) {

    }

    qDebug() << __func__ << " - 'currentChar': " << currentChar;


    // if the current character is not the closing character we just want to
    // enter the character
    if (currentChar != closingCharacter) {
        return false;
    }

    QString leftText = text.left(positionInBlock);
    int openingCharacterCount = leftText.count(openingCharacter);
    int closingCharacterCount = leftText.count(closingCharacter);

    // if there were enough opening characters just enter the character
    if (openingCharacterCount < (closingCharacterCount + 1)) {
        return false;
    }

    // move the cursor to the right and don't enter the character
    cursor.movePosition(QTextCursor::Right);
    setTextCursor(cursor);
    return true;
}

/**
 * Checks if the closing character should be output or not or if a closing
 * character after an opening character if needed
 *
 * @param quotationCharacter
 * @return
 */
bool QMarkdownTextEdit::quotationMarkCheck(const QString& quotationCharacter) {
    // check if bracket closing or read-only are enabled
    if (!(_autoTextOptions & AutoTextOption::BracketClosing) || isReadOnly()) {
        return false;
    }

    QTextCursor cursor = textCursor();
    int positionInBlock = cursor.position() - cursor.block().position();

    // get the current text from the block
    QString text = cursor.block().text();
    int textLength = text.length();

    // if we are at the end of the line we just want to enter the character
    if (positionInBlock >= textLength) {
        return handleBracketClosing(quotationCharacter);
    }

    QString currentChar = text.at(positionInBlock);

    // if the current character is not the quotation character we just want to
    // enter the character
    if (currentChar != quotationCharacter) {
        return handleBracketClosing(quotationCharacter);
    }

    // move the cursor to the right and don't enter the character
    cursor.movePosition(QTextCursor::Right);
    setTextCursor(cursor);
    return true;
}

/**
 * Handles removing of matching brackets and other markdown characters
 * Only works with backspace to remove text
 *
 * @return
 */
bool QMarkdownTextEdit::handleBracketRemoval() {
    // check if bracket removal or read-only are enabled
    if (!(_autoTextOptions & AutoTextOption::BracketRemoval) || isReadOnly()) {
        return false;
    }

    QTextCursor cursor = textCursor();

    // return if some text was selected
    if (!cursor.selectedText().isEmpty()) {
        return false;
    }

    int position = cursor.position();
    int positionInBlock = position - cursor.block().position();

    // return if backspace was pressed at the beginning of a block
    if (positionInBlock == 0) {
        return false;
    }

    // get the current text from the block
    QString text = cursor.block().text();
    QString charInFront = text.at(positionInBlock - 1);
    int openingCharacterIndex = _openingCharacters.indexOf(charInFront);

    // return if the character in front of the cursor is no opening character
    if (openingCharacterIndex == -1) {
        return false;
    }

    QString closingCharacter = _closingCharacters.at(openingCharacterIndex);

    // remove everything in front of the cursor
    text.remove(0, positionInBlock);
    int closingCharacterIndex = text.indexOf(closingCharacter);

    // return if no closing character was found in the text after the cursor
    if (closingCharacterIndex == -1) {
        return false;
    }

    // removing the closing character
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor,
                   closingCharacterIndex);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    // moving the cursor back to the old position so the previous character
    // can be removed
    cursor.setPosition(position);
    setTextCursor(cursor);
    return false;
}

/**
 * Increases (or decreases) the indention of the selected text
 * (if there is a text selected) in the noteTextEdit
 * @return
 */
bool QMarkdownTextEdit::increaseSelectedTextIndention(bool reverse, QString indentCharacters) {
    QTextCursor cursor = this->textCursor();
    QString selectedText = cursor.selectedText();

    if (selectedText != "") {
        // we need this strange newline character we are getting in the
        // selected text for newlines
        QString newLine = QString::fromUtf8(QByteArray::fromHex("e280a9"));
        QString newText;

        if (reverse) {
            // un-indent text

            QSettings settings;
            const int indentSize = indentCharacters == "\t" ? 4 : indentCharacters.count();

            // remove leading \t or spaces in following lines
            newText = selectedText.replace(
                    QRegularExpression(newLine + "(\\t| {1," + QString::number(indentSize) +"})"), "\n");

            // remove leading \t or spaces in first line
            newText.remove(QRegularExpression("^(\\t| {1," + QString::number(indentSize) +"})"));
        } else {
            // replace trailing new line to prevent an indent of the line after the selection
            newText = selectedText.replace(QRegularExpression(QRegularExpression::escape(newLine) + "$"), "\n");

            // indent text
            newText.replace(newLine, "\n" + indentCharacters).prepend(indentCharacters);

            // remove trailing \t
            newText.remove(QRegularExpression("\\t$"));
        }

        // insert the new text
        cursor.insertText(newText);

        // update the selection to the new text
        cursor.setPosition(cursor.position() - newText.size(), QTextCursor::KeepAnchor);
        this->setTextCursor(cursor);

        return true;
    } else if (reverse) {
        // if nothing was selected but we want to reverse the indention check
        // if there is a \t in front or after the cursor and remove it if so
        int position = cursor.position();

        if (!cursor.atStart()) {
            // get character in front of cursor
            cursor.setPosition(position - 1, QTextCursor::KeepAnchor);
        }

        // check for \t or space in front of cursor
        QRegularExpression re("[\\t ]");
        QRegularExpressionMatch match = re.match(cursor.selectedText());

        if (!match.hasMatch()) {
            // (select to) check for \t or space after the cursor
            cursor.setPosition(position);

            if (!cursor.atEnd()) {
                cursor.setPosition(position + 1, QTextCursor::KeepAnchor);
            }
        }

        match = re.match(cursor.selectedText());

        if (match.hasMatch()) {
            cursor.removeSelectedText();
        }

        return true;
    }

    // else just insert indentCharacters
    cursor.insertText(indentCharacters);

    return true;
}

/**
 * @brief Opens the link (if any) at the current cursor position
 */
bool QMarkdownTextEdit::openLinkAtCursorPosition() {
    QTextCursor cursor = this->textCursor();
    int clickedPosition = cursor.position();

    // select the text in the clicked block and find out on
    // which position we clicked
    cursor.movePosition(QTextCursor::StartOfBlock);
    int positionFromStart = clickedPosition - cursor.position();
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    QString selectedText = cursor.selectedText();

    // find out which url in the selected text was clicked
    QString urlString = getMarkdownUrlAtPosition(selectedText,
                                                 positionFromStart);
    QUrl url = QUrl(urlString);
    bool isRelativeFileUrl = urlString.startsWith("file://..");

    qDebug() << __func__ << " - 'emit urlClicked( urlString )': "
             << urlString;

    emit urlClicked(urlString);

    if ((url.isValid() && isValidUrl(urlString)) || isRelativeFileUrl) {
        // ignore some schemata
        if (!(_ignoredClickUrlSchemata.contains(url.scheme()) ||
                isRelativeFileUrl)) {
            // open the url
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
bool QMarkdownTextEdit::isValidUrl(const QString& urlString) {
    QRegularExpressionMatch match =
            QRegularExpression(R"(^\w+:\/\/.+)").match(urlString);
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
void QMarkdownTextEdit::openUrl(QString urlString) {
    qDebug() << "QMarkdownTextEdit " << __func__ << " - 'urlString': "
        << urlString;

    QDesktopServices::openUrl(QUrl(urlString));
}

/**
 * @brief Returns the highlighter instance
 * @return
 */
MarkdownHighlighter *QMarkdownTextEdit::highlighter() {
    return _highlighter;
}

/**
 * @brief Returns the searchWidget instance
 * @return
 */
QPlainTextEditSearchWidget *QMarkdownTextEdit::searchWidget() {
    return _searchWidget;
}

/**
 * @brief Sets url schemata that will be ignored when clicked on
 * @param urlSchemes
 */
void QMarkdownTextEdit::setIgnoredClickUrlSchemata(
        QStringList ignoredUrlSchemata) {
    _ignoredClickUrlSchemata = std::move(ignoredUrlSchemata);
}

/**
 * @brief Returns a map of parsed markdown urls with their link texts as key
 *
 * @param text
 * @return parsed urls
 */
QMap<QString, QString> QMarkdownTextEdit::parseMarkdownUrlsFromText(
        const QString& text) {
    QMap<QString, QString> urlMap;
    QRegularExpression regex;
    QRegularExpressionMatchIterator iterator;

    // match urls like this: <http://mylink>
//    re = QRegularExpression("(<(.+?:\\/\\/.+?)>)");
    regex = QRegularExpression("(<(.+?)>)");
    iterator = regex.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString linkText = match.captured(1);
        QString url = match.captured(2);
        urlMap[linkText] = url;
    }

    // match urls like this: [this url](http://mylink)
//    QRegularExpression re("(\\[.*?\\]\\((.+?:\\/\\/.+?)\\))");
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

    // match reference urls like this: [this url][1] with this later:
    // [1]: http://domain
    regex = QRegularExpression(R"(\[(.*?)\]\[(.+?)\])");
    iterator = regex.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString linkText = match.captured(1);
        QString referenceId = match.captured(2);

        // search for the referenced url in the whole text edit
//        QRegularExpression refRegExp(
//                "\\[" + QRegularExpression::escape(referenceId) +
//                        "\\]: (.+?:\\/\\/.+)");
        QRegularExpression refRegExp(
                "\\[" + QRegularExpression::escape(referenceId) + "\\]: (.+?)");
        QRegularExpressionMatch urlMatch = refRegExp.match(toPlainText());

        if (urlMatch.hasMatch()) {
            QString url = urlMatch.captured(1);
            urlMap[linkText] = url;
        }
    }

    return urlMap;
}

/**
 * @brief Returns the markdown url at position
 * @param text
 * @param position
 * @return url string
 */
QString QMarkdownTextEdit::getMarkdownUrlAtPosition(
        const QString& text, int position) {
    QString url;

    // get a map of parsed markdown urls with their link texts as key
    QMap<QString, QString> urlMap = parseMarkdownUrlsFromText(text);

    QMapIterator<QString, QString> iterator(urlMap);
    while (iterator.hasNext()) {
        iterator.next();
        QString linkText = iterator.key();
        QString urlString = iterator.value();

        int foundPositionStart = text.indexOf(linkText);

        if (foundPositionStart >= 0) {
            // calculate end position of found linkText
            int foundPositionEnd = foundPositionStart + linkText.size();

            // check if position is in found string range
            if ((position >= foundPositionStart) &&
                (position <= foundPositionEnd)) {
                url = urlString;
                break;
            }
        }
    }

    return url;
}

/**
 * @brief Duplicates the text in the text edit
 */
void QMarkdownTextEdit::duplicateText() {
    QTextCursor cursor = this->textCursor();
    QString selectedText = cursor.selectedText();

    // duplicate line if no text was selected
    if (selectedText == "") {
        int position = cursor.position();

        // select the whole line
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

        int positionDiff = cursor.position() - position;
        selectedText = "\n" + cursor.selectedText();

        // insert text with new line at end of the selected line
        cursor.setPosition(cursor.selectionEnd());
        cursor.insertText(selectedText);

        // set the position to same position it was in the duplicated line
        cursor.setPosition(cursor.position() - positionDiff);
    } else {
        // duplicate selected text
        cursor.setPosition(cursor.selectionEnd());
        int selectionStart = cursor.position();

        // insert selected text
        cursor.insertText(selectedText);
        int selectionEnd = cursor.position();

        // select the inserted text
        cursor.setPosition(selectionStart);
        cursor.setPosition(selectionEnd, QTextCursor::KeepAnchor);
    }

    this->setTextCursor(cursor);
}

void QMarkdownTextEdit::setText(const QString & text) {
    setPlainText(text);
}

void QMarkdownTextEdit::setPlainText(const QString & text) {
    // clear the dirty blocks vector to increase performance and prevent
    // a possible crash in QSyntaxHighlighter::rehighlightBlock
    _highlighter->clearDirtyBlocks();

    QPlainTextEdit::setPlainText(text);
    adjustRightMargin();
}

/**
 * Uses another widget as parent for the search widget
 */
void QMarkdownTextEdit::initSearchFrame(QWidget *searchFrame, bool darkMode) {
    _searchFrame = searchFrame;

    // remove the search widget from our layout
    layout()->removeWidget(_searchWidget);

    QLayout *layout = _searchFrame->layout();

    // create a grid layout for the frame and add the search widget to it
    if (layout == nullptr) {
        layout = new QVBoxLayout(_searchFrame);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    _searchWidget->setDarkMode(darkMode);
    layout->addWidget(_searchWidget);
    _searchFrame->setLayout(layout);
}

/**
 * Hides the text edit and the search widget
 */
void QMarkdownTextEdit::hide() {
    _searchWidget->hide();
    QWidget::hide();
}

/**
 * Handles an entered return key
 */
bool QMarkdownTextEdit::handleReturnEntered() {
    if (isReadOnly()) {
        return true;
    }

    QTextCursor cursor = this->textCursor();
    int position = cursor.position();

    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    QString currentLineText = cursor.selectedText();

    // if return is pressed and there is just an unordered list symbol then we want to
    // remove the list symbol
    // Valid listCharacters: '+ ', '-' , '* ', '+ [ ] ', '+ [x] ', '- [ ] ', '- [x] ', '* [ ] ', '* [x] '.
    QRegularExpression regex(R"(^(\s*)([+|\-|\*] \[(x| |)\]|[+\-\*])(\s+)$)");
    QRegularExpressionMatchIterator iterator = regex.globalMatch(currentLineText);
    if (iterator.hasNext()) {
        cursor.removeSelectedText();
        return true;
    }

    // if return is pressed and there is just an ordered list symbol then we want to
    // remove the list symbol
    regex = QRegularExpression(R"(^(\s*)(\d+\.)(\s+)$)");
    iterator = regex.globalMatch(currentLineText);
    if (iterator.hasNext()) {
        cursor.removeSelectedText();
        return true;
    }

    // Check if we are in an unordered list.
    // We are in a list when we have '* ', '- ' or '+ ', possibly with preceding
    // whitespace. If e.g. user has entered '**text**' and pressed enter - we
    // don't want do anymore list-stuff.
    QChar char0 = currentLineText.trimmed()[0];
    QChar char1 = currentLineText.trimmed()[1];
    bool inList = ((char0 == '*' || char0 == '-' || char0 == '+') && char1 == ' ');

    if (inList) {
        // if the current line starts with a list character (possibly after
        // whitespaces) add the whitespaces at the next line too
        // Valid listCharacters: '+ ', '-' , '* ', '+ [ ] ', '+ [x] ', '- [ ] ', '- [x] ', '* [ ] ', '* [x] '.
        regex = QRegularExpression(R"(^(\s*)([+|\-|\*] \[(x| |)\]|[+\-\*])(\s+))");
        iterator = regex.globalMatch(currentLineText);
        if (iterator.hasNext()) {
            QRegularExpressionMatch match = iterator.next();
            QString whitespaces = match.captured(1);
            QString listCharacter = match.captured(2);
            QString whitespaceCharacter = match.captured(4);

            // start new checkbox list item with an unchecked checkbox
            iterator = QRegularExpression(R"(^([+|\-|\*]) \[(x| |)\])").globalMatch(listCharacter);
            if (iterator.hasNext()) {
                QRegularExpressionMatch match = iterator.next();
                QString realListCharacter = match.captured(1);
                listCharacter = realListCharacter + " [ ]";
            }

            cursor.setPosition(position);
            cursor.insertText("\n" + whitespaces + listCharacter + whitespaceCharacter);

            // scroll to the cursor if we are at the bottom of the document
            ensureCursorVisible();
            return true;
        }
    }

    // check for ordered lists and increment the list number in the next line
    regex = QRegularExpression(R"(^(\s*)(\d+)\.(\s+))");
    iterator = regex.globalMatch(currentLineText);
    if (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString whitespaces = match.captured(1);
        uint listNumber = match.captured(2).toUInt();
        QString whitespaceCharacter = match.captured(3);

        cursor.setPosition(position);
        cursor.insertText("\n" + whitespaces + QString::number(listNumber + 1) +
        "." + whitespaceCharacter);

        // scroll to the cursor if we are at the bottom of the document
        ensureCursorVisible();
        return true;
    }

    // intent next line with same whitespaces as in current line
    regex = QRegularExpression(R"(^(\s+))");
    iterator = regex.globalMatch(currentLineText);
    if (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString whitespaces = match.captured(1);

        cursor.setPosition(position);
        cursor.insertText("\n" + whitespaces);

        // scroll to the cursor if we are at the bottom of the document
        ensureCursorVisible();
        return true;
    }

    return false;
}

/**
 * Handles entered tab or reverse tab keys
 */
bool QMarkdownTextEdit::handleTabEntered(bool reverse, QString indentCharacters) {
    if (isReadOnly()) {
        return true;
    }

    QTextCursor cursor = this->textCursor();

    // only check for lists if we haven't a text selected
    if (cursor.selectedText().isEmpty()) {
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        QString currentLineText = cursor.selectedText();

        // check if we want to indent or un-indent an ordered list
        // Valid listCharacters: '+ ', '-' , '* ', '+ [ ] ', '+ [x] ', '- [ ] ', '- [x] ', '* [ ] ', '* [x] '.
        QRegularExpression re(R"(^(\s*)([+|\-|\*] \[(x| )\]|[+\-\*])(\s+)$)");
        QRegularExpressionMatchIterator i = re.globalMatch(currentLineText);

        if (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString whitespaces = match.captured(1);
            QString listCharacter = match.captured(2);
            QString whitespaceCharacter = match.captured(4);

            // add or remove one tabulator key
            if (reverse) {
                whitespaces.chop(1);
            } else {
                whitespaces += indentCharacters;
            }

            cursor.insertText(whitespaces + listCharacter + whitespaceCharacter);
            return true;
        }

        // check if we want to indent or un-indent an ordered list
        re = QRegularExpression(R"(^(\s*)(\d+)\.(\s+)$)");
        i = re.globalMatch(currentLineText);

        if (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString whitespaces = match.captured(1);
            QString listCharacter = match.captured(2);
            QString whitespaceCharacter = match.captured(3);

            // add or remove one tabulator key
            if (reverse) {
                whitespaces.chop(1);
            } else {
                whitespaces += indentCharacters;
            }

            cursor.insertText(whitespaces + listCharacter + "." +
            whitespaceCharacter);
            return true;
        }
    }

    // check if we want to intent the whole text
    return increaseSelectedTextIndention(reverse, indentCharacters);
}

/**
 * Sets the auto text options
 */
void QMarkdownTextEdit::setAutoTextOptions(AutoTextOptions options) {
    _autoTextOptions = options;
}

/**
 * Overrides QPlainTextEdit::paintEvent to fix the RTL bug of QPlainTextEdit
 *
 * @param e
 */
void QMarkdownTextEdit::paintEvent(QPaintEvent *e) {
    QTextBlock block = firstVisibleBlock();

    while (block.isValid()) {
        QTextLayout *layout = block.layout();

        // this fixes the RTL bug of QPlainTextEdit
        // https://bugreports.qt.io/browse/QTBUG-7516
        if (block.text().isRightToLeft())
        {
            QTextOption opt = document()->defaultTextOption();
            opt = QTextOption(Qt::AlignRight);
            opt.setTextDirection(Qt::RightToLeft);
            layout->setTextOption(opt);
        }

        block = block.next();
    }

    QPlainTextEdit::paintEvent(e);
}

/**
 * Overrides QPlainTextEdit::setReadOnly to fix a problem with Chinese and
 * Japanese input methods
 *
 * @param ro
 */
void QMarkdownTextEdit::setReadOnly(bool ro) {
    QPlainTextEdit::setReadOnly(ro);

    // attempted to fix a problem with Chinese and Japanese input methods
    // @see https://github.com/pbek/QOwnNotes/issues/976
    setAttribute(Qt::WA_InputMethodEnabled, !isReadOnly());
}

void QMarkdownTextEdit::doSearch(
        QString &searchText,
        QPlainTextEditSearchWidget::SearchMode searchMode) {
    _searchWidget->setSearchText(searchText);
    _searchWidget->setSearchMode(searchMode);
    _searchWidget->doSearchCount();
    _searchWidget->activate(false);
}

void QMarkdownTextEdit::hideSearchWidget(bool reset) {
    _searchWidget->deactivate();

    if (reset) {
        _searchWidget->reset();
    }
}

void QMarkdownTextEdit::updateSettings() {
    // if true: centers the screen if cursor reaches bottom (but not top)
    setCenterOnScroll(_centerCursor);
}

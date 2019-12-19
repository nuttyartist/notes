
/*
 * Copyright (c) 2014-2019 Patrizio Bekerle -- http://www.bekerle.com
 * Copyright (c) 2019      Waqar Ahmed      -- <waqar.17a@gmail.com>
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
 * QPlainTextEdit markdown highlighter
 */

#include <QTimer>
#include <QDebug>
#include <QTextDocument>
#include "markdownhighlighter.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <utility>
#include "qownlanguagedata.h"

QHash<QString, MarkdownHighlighter::HighlighterState> MarkdownHighlighter::_langStringToEnum;

/**
 * Markdown syntax highlighting
 *
 * markdown syntax:
 * http://daringfireball.net/projects/markdown/syntax
 *
 * @param parent
 * @return
 */
MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent,
                                         HighlightingOptions highlightingOptions)
        : QSyntaxHighlighter(parent),
          _highlightingOptions(highlightingOptions) {
   // _highlightingOptions = highlightingOptions;
    _timer = new QTimer(this);
    QObject::connect(_timer, SIGNAL(timeout()),
                     this, SLOT(timerTick()));
    _timer->start(1000);

    // initialize the highlighting rules
    initHighlightingRules();

    // initialize the text formats
    initTextFormats();

    //initialize code langs
    initCodeLangs();
}

/**
 * Does jobs every second
 */
void MarkdownHighlighter::timerTick() {
    // qDebug() << "timerTick: " << this << ", " << this->parent()->parent()->parent()->objectName();

    // re-highlight all dirty blocks
    reHighlightDirtyBlocks();

    // emit a signal every second if there was some highlighting done
    if (_highlightingFinished) {
        _highlightingFinished = false;
        emit(highlightingFinished());
    }
}

/**
 * Re-highlights all dirty blocks
 */
void MarkdownHighlighter::reHighlightDirtyBlocks() {
    while (_dirtyTextBlocks.count() > 0) {
        QTextBlock block = _dirtyTextBlocks.at(0);
        rehighlightBlock(block);
        _dirtyTextBlocks.removeFirst();
    }
}

/**
 * Clears the dirty blocks vector
 */
void MarkdownHighlighter::clearDirtyBlocks() {
    _dirtyTextBlocks.clear();
}

/**
 * Adds a dirty block to the list if it doesn't already exist
 *
 * @param block
 */
void MarkdownHighlighter::addDirtyBlock(const QTextBlock& block) {
    if (!_dirtyTextBlocks.contains(block)) {
        _dirtyTextBlocks.append(block);
    }
}

/**
 * Initializes the highlighting rules
 *
 * regexp tester:
 * https://regex101.com
 *
 * other examples:
 * /usr/share/kde4/apps/katepart/syntax/markdown.xml
 */
void MarkdownHighlighter::initHighlightingRules() {
    // highlight the reference of reference links
    {
        HighlightingRule rule(HighlighterState::MaskedSyntax);
        rule.pattern = QRegularExpression(QStringLiteral(R"(^\[.+?\]: \w+://.+$)"));
        _highlightingRulesPre.append(rule);
    }

    // highlight unordered lists
    {
        HighlightingRule rule(HighlighterState::List);
        rule.pattern = QRegularExpression(QStringLiteral("^\\s*[-*+]\\s"));
        rule.useStateAsCurrentBlockState = true;
        _highlightingRulesPre.append(rule);

        // highlight ordered lists
        rule.pattern = QRegularExpression(QStringLiteral(R"(^\s*\d+\.\s)"));
        _highlightingRulesPre.append(rule);
    }

    // highlight block quotes
    {
        HighlightingRule rule(HighlighterState::BlockQuote);
        rule.pattern = QRegularExpression(
                    _highlightingOptions.testFlag(
                        HighlightingOption::FullyHighlightedBlockQuote) ?
                        QStringLiteral("^\\s*(>\\s*.+)") : QStringLiteral("^\\s*(>\\s*)+"));
        _highlightingRulesPre.append(rule);
    }

    // highlight horizontal rulers
    {
        HighlightingRule rule(HighlighterState::HorizontalRuler);
        rule.pattern = QRegularExpression(QStringLiteral("^([*\\-_]\\s?){3,}$"));
        _highlightingRulesPre.append(rule);
    }

    // highlight tables without starting |
    // we drop that for now, it's far too messy to deal with
//    rule = HighlightingRule();
//    rule.pattern = QRegularExpression("^.+? \\| .+? \\| .+$");
//    rule.state = HighlighterState::Table;
//    _highlightingRulesPre.append(rule);

    /*
     * highlight italic
     * this goes before bold so that bold can overwrite italic
     *
     * text to test:
     * **bold** normal **bold**
     * *start of line* normal
     * normal *end of line*
     * * list item *italic*
     */
    {
        HighlightingRule rule(HighlighterState::Italic);
        // we don't allow a space after the starting * to prevent problems with
        // unordered lists starting with a *
        rule.pattern = QRegularExpression(
                    QStringLiteral(R"((?:^|[^\*\b])(?:\*([^\* ][^\*]*?)\*)(?:[^\*\b]|$))"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        rule.pattern = QRegularExpression(QStringLiteral("\\b_([^_]+)_\\b"));
        _highlightingRulesAfter.append(rule);
    }

    {
        HighlightingRule rule(HighlighterState::Bold);
        // highlight bold
        rule.pattern = QRegularExpression(QStringLiteral(R"(\B\*{2}(.+?)\*{2}\B)"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);
        rule.pattern = QRegularExpression(QStringLiteral("\\b__(.+?)__\\b"));
        _highlightingRulesAfter.append(rule);
    }

    {
        HighlightingRule rule(HighlighterState::MaskedSyntax);
        // highlight strike through
        rule.pattern = QRegularExpression(QStringLiteral(R"(\~{2}(.+?)\~{2})"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);
    }

    // highlight urls
    {
        HighlightingRule rule(HighlighterState::Link);

        // highlight urls without any other markup
        rule.pattern = QRegularExpression(QStringLiteral(R"(\b\w+?:\/\/[^\s>]+)"));
        rule.capturingGroup = 0;
        _highlightingRulesAfter.append(rule);

        // highlight urls with <> but without any . in it
        rule.pattern = QRegularExpression(QStringLiteral(R"(<(\w+?:\/\/[^\s]+)>)"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight links with <> that have a .in it
        //    rule.pattern = QRegularExpression("<(.+?:\\/\\/.+?)>");
        rule.pattern = QRegularExpression(QStringLiteral("<([^\\s`][^`]*?\\.[^`]*?[^\\s`])>"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight urls with title
        //    rule.pattern = QRegularExpression("\\[(.+?)\\]\\(.+?://.+?\\)");
        //    rule.pattern = QRegularExpression("\\[(.+?)\\]\\(.+\\)\\B");
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[([^\[\]]+)\]\((\S+|.+?)\)\B)"));
        _highlightingRulesAfter.append(rule);

        // highlight urls with empty title
        //    rule.pattern = QRegularExpression("\\[\\]\\((.+?://.+?)\\)");
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[\]\((.+?)\))"));
        _highlightingRulesAfter.append(rule);

        // highlight email links
        rule.pattern = QRegularExpression(QStringLiteral("<(.+?@.+?)>"));
        _highlightingRulesAfter.append(rule);

        // highlight reference links
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[(.+?)\]\[.+?\])"));
        _highlightingRulesAfter.append(rule);
    }

    // Images
    {
        // highlight images with text
        HighlightingRule rule(HighlighterState::Image);
        rule.pattern = QRegularExpression(QStringLiteral(R"(!\[(.+?)\]\(.+?\))"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight images without text
        rule.pattern = QRegularExpression(QStringLiteral(R"(!\[\]\((.+?)\))"));
        _highlightingRulesAfter.append(rule);
    }

    // highlight images links
    {
//        HighlightingRule rule;
        HighlightingRule rule(HighlighterState::Link);
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[!\[(.+?)\]\(.+?\)\]\(.+?\))"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight images links without text
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[!\[\]\(.+?\)\]\((.+?)\))"));
        _highlightingRulesAfter.append(rule);
    }

    // highlight trailing spaces
    {
        HighlightingRule rule(HighlighterState::TrailingSpace);
        rule.pattern = QRegularExpression(QStringLiteral("( +)$"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);
    }

    // highlight inline code
    {
        HighlightingRule rule(HighlighterState::InlineCodeBlock);
//        HighlightingRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("`(.+?)`"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);
    }

    // highlight code blocks with four spaces or tabs in front of them
    // and no list character after that
    {
        HighlightingRule rule(HighlighterState::CodeBlock);
//        HighlightingRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("^((\\t)|( {4,})).+$"));
        rule.disableIfCurrentStateIsSet = true;
        _highlightingRulesAfter.append(rule);
    }

    // highlight inline comments
    {
        HighlightingRule rule(HighlighterState::Comment);
        rule.pattern = QRegularExpression(QStringLiteral(R"(<!\-\-(.+?)\-\->)"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight comments for Rmarkdown for academic papers
        rule.pattern = QRegularExpression(QStringLiteral(R"(^\[.+?\]: # \(.+?\)$)"));
        _highlightingRulesAfter.append(rule);
    }

    // highlight tables with starting |
    {
        HighlightingRule rule(HighlighterState::Table);
        rule.pattern = QRegularExpression(QStringLiteral("^\\|.+?\\|$"));
        _highlightingRulesAfter.append(rule);
    }
}

/**
 * Initializes the text formats
 *
 * @param defaultFontSize
 */
void MarkdownHighlighter::initTextFormats(int defaultFontSize) {
    QTextCharFormat format;

    // set character formats for headlines
    format = QTextCharFormat();
    format.setForeground(QBrush(QColor(2, 69, 150)));
    format.setFontWeight(QFont::Bold);
    format.setFontPointSize(defaultFontSize * 1.6);
    _formats[H1] = format;
    format.setFontPointSize(defaultFontSize * 1.5);
    _formats[H2] = format;
    format.setFontPointSize(defaultFontSize * 1.4);
    _formats[H3] = format;
    format.setFontPointSize(defaultFontSize * 1.3);
    _formats[H4] = format;
    format.setFontPointSize(defaultFontSize * 1.2);
    _formats[H5] = format;
    format.setFontPointSize(defaultFontSize * 1.1);
    _formats[H6] = format;
    format.setFontPointSize(defaultFontSize);

    // set character format for horizontal rulers
    format = QTextCharFormat();
    format.setForeground(QBrush(Qt::darkGray));
    format.setBackground(QBrush(Qt::lightGray));
    _formats[HorizontalRuler] = format;

    // set character format for lists
    format = QTextCharFormat();
    format.setForeground(QBrush(QColor(163, 0, 123)));
    _formats[List] = format;

    // set character format for links
    format = QTextCharFormat();
    format.setForeground(QBrush(QColor(0, 128, 255)));
    format.setFontUnderline(true);
    _formats[Link] = format;

    // set character format for images
    format = QTextCharFormat();
    format.setForeground(QBrush(QColor(0, 191, 0)));
    format.setBackground(QBrush(QColor(228, 255, 228)));
    _formats[Image] = format;

    // set character format for code blocks
    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    //format.setBackground(QColor(220, 220, 220));
    _formats[CodeBlock] = format;
    _formats[InlineCodeBlock] = format;

    // set character format for italic
    format = QTextCharFormat();
    format.setFontWeight(QFont::StyleItalic);
    format.setFontItalic(true);
    _formats[Italic] = format;

    // set character format for bold
    format = QTextCharFormat();
    format.setFontWeight(QFont::Bold);
    _formats[Bold] = format;

    // set character format for comments
    format = QTextCharFormat();
    format.setForeground(QBrush(Qt::gray));
    _formats[Comment] = format;

    // set character format for masked syntax
    format = QTextCharFormat();
    format.setForeground(QBrush("#cccccc"));
    _formats[MaskedSyntax] = format;

    // set character format for tables
    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QBrush(QColor("#649449")));
    _formats[Table] = format;

    // set character format for block quotes
    format = QTextCharFormat();
    format.setForeground(QBrush(QColor(Qt::darkRed)));
    _formats[BlockQuote] = format;

    format = QTextCharFormat();
    _formats[HeadlineEnd] = format;

    format = QTextCharFormat();
    _formats[NoState] = format;

    /****************************************
     * Formats for syntax highlighting
     ***************************************/

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#F92672"));
    _formats[CodeKeyWord] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#a39b4e"));
    _formats[CodeString] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#75715E"));
    _formats[CodeComment] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#54aebf"));
    _formats[CodeType] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#db8744"));
    _formats[CodeOther] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#AE81FF"));
    _formats[CodeNumLiteral] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#018a0f"));
    _formats[CodeBuiltIn] = format;
}

/**
 * @brief initializes the langStringToEnum
 */
void MarkdownHighlighter::initCodeLangs()
{
    MarkdownHighlighter::_langStringToEnum =
            QHash<QString, MarkdownHighlighter::HighlighterState> {
        {QLatin1String("bash"),        MarkdownHighlighter::CodeBash},
        {QLatin1String("c"),           MarkdownHighlighter::CodeC},
        {QLatin1String("cpp"),         MarkdownHighlighter::CodeCpp},
        {QLatin1String("cxx"),         MarkdownHighlighter::CodeCpp},
        {QLatin1String("c++"),         MarkdownHighlighter::CodeCpp},
        {QLatin1String("c#"),          MarkdownHighlighter::CodeCSharp},
        {QLatin1String("csharp"),      MarkdownHighlighter::CodeCSharp},
        {QLatin1String("css"),         MarkdownHighlighter::CodeCSS},
        {QLatin1String("go"),          MarkdownHighlighter::CodeCSharp},
        {QLatin1String("html"),        MarkdownHighlighter::CodeXML},
        {QLatin1String("java"),        MarkdownHighlighter::CodeJava},
        {QLatin1String("javascript"),  MarkdownHighlighter::CodeJava},
        {QLatin1String("js"),          MarkdownHighlighter::CodeJs},
        {QLatin1String("json"),        MarkdownHighlighter::CodeJSON},
        {QLatin1String("php"),         MarkdownHighlighter::CodePHP},
        {QLatin1String("py"),          MarkdownHighlighter::CodePython},
        {QLatin1String("python"),      MarkdownHighlighter::CodePython},
        {QLatin1String("qml"),         MarkdownHighlighter::CodeQML},
        {QLatin1String("rust"),        MarkdownHighlighter::CodeRust},
        {QLatin1String("sh"),          MarkdownHighlighter::CodeBash},
        {QLatin1String("sql"),         MarkdownHighlighter::CodeSQL},
        {QLatin1String("SQL"),         MarkdownHighlighter::CodeSQL},
        {QLatin1String("ts"),          MarkdownHighlighter::CodeTypeScript},
        {QLatin1String("typescript"),  MarkdownHighlighter::CodeTypeScript},
        {QLatin1String("v"),           MarkdownHighlighter::CodeV},
        {QLatin1String("xml"),         MarkdownHighlighter::CodeXML}
    };

}

/**
 * Sets the text formats
 *
 * @param formats
 */
void MarkdownHighlighter::setTextFormats(
        QHash<HighlighterState, QTextCharFormat> formats) {
    _formats = std::move(formats);
}

/**
 * Sets a text format
 *
 * @param formats
 */
void MarkdownHighlighter::setTextFormat(HighlighterState state,
                                        QTextCharFormat format) {
    _formats[state] = std::move(format);
}

/**
 * Does the markdown highlighting
 *
 * @param text
 */
void MarkdownHighlighter::highlightBlock(const QString &text) {
    setCurrentBlockState(HighlighterState::NoState);
    currentBlock().setUserState(HighlighterState::NoState);
    highlightMarkdown(text);
    _highlightingFinished = true;
}

void MarkdownHighlighter::highlightMarkdown(const QString& text) {
    if (!text.isEmpty()) {
        highlightAdditionalRules(_highlightingRulesPre, text);

        // needs to be called after the horizontal ruler highlighting
        highlightHeadline(text);

        highlightAdditionalRules(_highlightingRulesAfter, text);
    }

    highlightCommentBlock(text);
    highlightCodeBlock(text);
    highlightFrontmatterBlock(text);
}

/**
 * Highlight headlines
 *
 * @param text
 */
void MarkdownHighlighter::highlightHeadline(const QString& text) {
    bool headingFound = text.startsWith(QLatin1String("# ")) ||
                        text.startsWith(QLatin1String("## ")) ||
                        text.startsWith(QLatin1String("### ")) ||
                        text.startsWith(QLatin1String("#### ")) ||
                        text.startsWith(QLatin1String("##### ")) ||
                        text.startsWith(QLatin1String("###### "));

    const QTextCharFormat &maskedFormat = _formats[HighlighterState::MaskedSyntax];

    if (headingFound) {
        int count = 0;
        int len = text.length() > 6 ? 6 : text.length();
        //check only first 6 chars of text
        for (int i = 0; i < len; ++i) {
            if (text.at(i) == QLatin1Char('#')) {
                ++count;
            }
        }

        const auto state = HighlighterState(HighlighterState::H1 + count - 1);

        QTextCharFormat &format = _formats[state];
        QTextCharFormat currentMaskedFormat = maskedFormat;

        // set the font size from the current rule's font format
        currentMaskedFormat.setFontPointSize(format.fontPointSize());

        // first highlight everything as MaskedSyntax
        setFormat(0, text.length(), currentMaskedFormat);

        //const int length = text.length() - count;
        // then highlight with the real format
        setFormat(0, text.length(), _formats[state]);

        // set a margin for the current block
        setCurrentBlockMargin(state);

        setCurrentBlockState(state);
        currentBlock().setUserState(state);
        return;
    }

    auto hasOnlyHeadChars = [](const QString &txt, const QChar c) -> bool {
        if (txt.isEmpty()) return false;
        for (int i = 0; i < txt.length(); ++i) {
            if (txt.at(i) != c)
                return false;
        }
        return true;
    };

    // take care of ==== and ---- headlines

    QTextBlock previousBlock = currentBlock().previous();
    const QString &previousText = previousBlock.text();

    const bool pattern1 = hasOnlyHeadChars(text, QLatin1Char('='));
    if (pattern1) {

        if (( (previousBlockState() == HighlighterState::H1) ||
               previousBlockState() == HighlighterState::NoState) &&
               previousText.length() > 0) {
            QTextCharFormat currentMaskedFormat = maskedFormat;
            // set the font size from the current rule's font format
            currentMaskedFormat.setFontPointSize(
                        _formats[HighlighterState::H1].fontPointSize());

            setFormat(0, text.length(), currentMaskedFormat);
            setCurrentBlockState(HighlighterState::HeadlineEnd);
            previousBlock.setUserState(HighlighterState::H1);

            // set a margin for the current block
            setCurrentBlockMargin(HighlighterState::H1);

            // we want to re-highlight the previous block
            // this must not done directly, but with a queue, otherwise it
            // will crash
            // setting the character format of the previous text, because this
            // causes text to be formatted the same way when writing after
            // the text
            addDirtyBlock(previousBlock);
        }
        return;
    }

    const bool pattern2 = hasOnlyHeadChars(text, QLatin1Char('-'));
    if (pattern2) {

        if (( (previousBlockState() == HighlighterState::H2) ||
               previousBlockState() == HighlighterState::NoState) &&
               previousText.length() > 0) {
            // set the font size from the current rule's font format
            QTextCharFormat currentMaskedFormat = maskedFormat;
            currentMaskedFormat.setFontPointSize(
                        _formats[HighlighterState::H2].fontPointSize());

            setFormat(0, text.length(), currentMaskedFormat);
            setCurrentBlockState(HighlighterState::HeadlineEnd);
            previousBlock.setUserState(HighlighterState::H2);

            // set a margin for the current block
            setCurrentBlockMargin(HighlighterState::H2);

            // we want to re-highlight the previous block
            addDirtyBlock(previousBlock);
        }
        return;
    }

    //check next block for ====
    QTextBlock nextBlock = currentBlock().next();
    const QString &nextBlockText = nextBlock.text();
    const bool nextHasEqualChars = hasOnlyHeadChars(nextBlockText, QLatin1Char('='));
    if (nextHasEqualChars) {
        setFormat(0, text.length(), _formats[HighlighterState::H1]);
        setCurrentBlockState(HighlighterState::H1);
        currentBlock().setUserState(HighlighterState::H1);
    }
    //check next block for ----
    const bool nextHasMinusChars = hasOnlyHeadChars(nextBlockText, QLatin1Char('-'));
    if (nextHasMinusChars) {
        setFormat(0, text.length(), _formats[HighlighterState::H2]);
        setCurrentBlockState(HighlighterState::H2);
        currentBlock().setUserState(HighlighterState::H2);
    }
}


/**
 * Sets a margin for the current block
 *
 * @param state
 */
void MarkdownHighlighter::setCurrentBlockMargin(
        MarkdownHighlighter::HighlighterState state) {
    // this is currently disabled because it causes multiple problems:
    // - it prevents "undo" in headlines
    //   https://github.com/pbek/QOwnNotes/issues/520
    // - invisible lines at the end of a note
    //   https://github.com/pbek/QOwnNotes/issues/667
    // - a crash when reaching the invisible lines when the current line is
    //   highlighted
    //   https://github.com/pbek/QOwnNotes/issues/701
    return;

    qreal margin;

    switch (state) {
        case HighlighterState::H1:
            margin = 5;
            break;
        case HighlighterState::H2:
        case HighlighterState::H3:
        case HighlighterState::H4:
        case HighlighterState::H5:
        case HighlighterState::H6:
            margin = 3;
            break;
        default:
            return;
    }

    QTextBlockFormat blockFormat = currentBlock().blockFormat();
    blockFormat.setTopMargin(2);
    blockFormat.setBottomMargin(margin);

    // this prevents "undo" in headlines!
    QTextCursor* myCursor = new QTextCursor(currentBlock());
    myCursor->setBlockFormat(blockFormat);
}

/**
 * Highlight multi-line code blocks
 *
 * @param text
 */
void MarkdownHighlighter::highlightCodeBlock(const QString& text) {

    if (text.startsWith(QLatin1String("```"))) {
        if (previousBlockState() != HighlighterState::CodeBlock &&
            previousBlockState() < HighlighterState::CodeCpp) {
            QString lang = text.mid(3, text.length());
            MarkdownHighlighter::HighlighterState progLang = _langStringToEnum.value(lang);

            if (progLang >= HighlighterState::CodeCpp) {
                setCurrentBlockState(progLang);
            } else {
                previousBlockState() == HighlighterState::CodeBlock ?
                            setCurrentBlockState(CodeBlockEnd) : setCurrentBlockState(CodeBlock);
            }
        } else if (previousBlockState() == HighlighterState::CodeBlock ||
                   previousBlockState() >= HighlighterState::CodeCpp) {
            setCurrentBlockState(HighlighterState::CodeBlockEnd);
        }

        // set the font size from the current rule's font format
        QTextCharFormat &maskedFormat =
                _formats[HighlighterState::MaskedSyntax];
        maskedFormat.setFontPointSize(
                    _formats[HighlighterState::CodeBlock].fontPointSize());

        setFormat(0, text.length(), maskedFormat);
    } else if (previousBlockState() == HighlighterState::CodeBlock ||
               previousBlockState() >= HighlighterState::CodeCpp) {

        if (previousBlockState() >= HighlighterState::CodeCpp) {
            setCurrentBlockState(previousBlockState());
            highlightSyntax(text);
        } else {
            setFormat(0, text.length(), _formats[HighlighterState::CodeBlock]);
            setCurrentBlockState(HighlighterState::CodeBlock);
        }
    }
}



/**
 * @brief Does the code syntax highlighting
 * @param text
 */
void MarkdownHighlighter::highlightSyntax(const QString &text)
{
    if (text.isEmpty()) return;

    const auto textLen = text.length();

    QChar comment;
    bool isCSS = false;

    QMultiHash<char, QLatin1String> keywords{};
    QMultiHash<char, QLatin1String> others{};
    QMultiHash<char, QLatin1String> types{};
    QMultiHash<char, QLatin1String> builtin{};
    QMultiHash<char, QLatin1String> literals{};

    QList<QLatin1String> wordList;

    switch (currentBlockState()) {
        case HighlighterState::CodeCpp :
        case HighlighterState::CodeCppComment :
            loadCppData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeJs :
        case HighlighterState::CodeJsComment :
            loadJSData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeC :
        case HighlighterState::CodeCComment :
            loadCppData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeBash :
            loadShellData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        case HighlighterState::CodePHP :
        case HighlighterState::CodePHPComment :
            loadPHPData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeQML :
        case HighlighterState::CodeQMLComment :
            loadQMLData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodePython :
            loadPythonData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        case HighlighterState::CodeRust :
        case HighlighterState::CodeRustComment :
            loadRustData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeJava :
        case HighlighterState::CodeJavaComment :
            loadJavaData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeCSharp :
        case HighlighterState::CodeCSharpComment :
            loadCSharpData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeGo :
        case HighlighterState::CodeGoComment :
            loadGoData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeV :
        case HighlighterState::CodeVComment :
            loadVData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeSQL :
            loadSQLData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeJSON :
            loadJSONData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeXML :
            xmlHighlighter(text);
            return;
        case HighlighterState::CodeCSS :
        case HighlighterState::CodeCSSComment :
            isCSS = true;
            loadCSSData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeTypeScript:
        case HighlighterState::CodeTypeScriptComment:
            loadTypescriptData(types, keywords, builtin, literals, others);
            break;
    default:
        break;
    }

    // keep the default code block format
    // this statement is very slow
    // TODO: do this formatting when necessary instead of
    // applying it to the whole block in the beginning
    setFormat(0, textLen, _formats[CodeBlock]);

    auto applyCodeFormat = [this, &wordList](int i, const QMultiHash<char, QLatin1String> &data,
                        const QString &text, const QTextCharFormat &fmt) -> int {
        // check if we are at the beginning OR if this is the start of a word
        // AND the current char is present in the data structure
        if ( ( i == 0 || !text[i-1].isLetter()) && data.contains(text[i].toLatin1())) {
            wordList = data.values(text[i].toLatin1());
#if QT_VERSION >= 0x050700
            for(const QLatin1String &word : qAsConst(wordList)) {
#else
            for(const QLatin1String &word : wordList) {
#endif
                if (word == text.midRef(i, word.size())) {
                    //check if we are at the end of text OR if we have a complete word
                    if ( i + word.size() == text.length() ||
                         !text.at(i + word.size()).isLetter()) {
                        setFormat(i, word.size(), fmt);
                        i += word.size();
                    }
                }
            }
        }
        return i;
    };

    const QTextCharFormat &formatType = _formats[CodeType];
    const QTextCharFormat &formatKeyword = _formats[CodeKeyWord];
    const QTextCharFormat &formatComment = _formats[CodeComment];
    const QTextCharFormat &formatNumLit = _formats[CodeNumLiteral];
    const QTextCharFormat &formatBuiltIn = _formats[CodeBuiltIn];
    const QTextCharFormat &formatOther = _formats[CodeOther];

    for (int i=0; i< textLen; ++i) {

        if (currentBlockState() % 2 != 0) goto Comment;

        while (i < textLen && !text[i].isLetter()) {
            if (text[i].isSpace()) {
                ++i;
                //make sure we don't cross the bound
                if (i == textLen) return;
                if (text[i].isLetter()) break;
                else continue;
            }
            //inline comment
            if (text[i] == QLatin1Char('/')) {
                if((i+1) < textLen){
                    if(text[i+1] == QLatin1Char('/')) {
                        setFormat(i, textLen, formatComment);
                        return;
                    } else if(text[i+1] == QLatin1Char('*')) {
                        Comment:
                        int next = text.indexOf(QLatin1String("*/"));
                        if (next == -1) {
                            //we didn't find a comment end.
                            //Check if we are already in a comment block
                            if (currentBlockState() % 2 == 0)
                                setCurrentBlockState(currentBlockState() + 1);
                            setFormat(i, textLen,  formatComment);
                            return;
                        } else {
                            //we found a comment end
                            //mark this block as code if it was previously comment
                            //first check if the comment ended on the same line
                            //if modulo 2 is not equal to zero, it means we are in a comment
                            //-1 will set this block's state as language
                            if (currentBlockState() % 2 != 0) {
                                setCurrentBlockState(currentBlockState() - 1);
                            }
                            next += 2;
                            setFormat(i, next - i,  formatComment);
                            i = next;
                            if (i >= textLen) return;
                        }
                    }
                }
            } else if (text[i] == comment) {
                setFormat(i, textLen, formatComment);
                return;
            //integer literal
            } else if (text[i].isNumber()) {
               i = highlightIntegerLiterals(text, i);
            //string literals
            } else if (text[i] == QLatin1Char('\"')) {
               i = highlightStringLiterals('\"', text, i);
            }  else if (text[i] == QLatin1Char('\'')) {
               i = highlightStringLiterals('\'', text, i);
            }
            if (i >= textLen) {
                if (isCSS) cssHighlighter(text);
                return;
            }
            ++i;
        }

        int pos = i;

        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Types */
        i = applyCodeFormat(i, types, text, formatType);
        /************************************************
         next letter is usually a space, in that case
         going forward is useless, so continue;
         We can ++i here and go to the beginning of the next word
         so that the next formatter can check for formatting but this will
         cause problems in case the next word is also of 'Type' or the current
         type(keyword/builtin). We can work around it and reset the value of i
         in the beginning of the loop to the word's first letter but I am not
         sure about its efficiency yet.
         ************************************************/
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Keywords */
        i = applyCodeFormat(i, keywords, text, formatKeyword);
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Literals (true/false/NULL,nullptr) */
        i = applyCodeFormat(i, literals, text, formatNumLit);
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Builtin library stuff */
        i = applyCodeFormat(i, builtin, text, formatBuiltIn);
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight other stuff (preprocessor etc.) */
        if (( i == 0 || !text[i-1].isLetter()) && others.contains(text[i].toLatin1())) {
            wordList = others.values(text[i].toLatin1());
#if QT_VERSION >= 0x050700
            for(const QLatin1String &word : qAsConst(wordList)) {
#else
            for(const QLatin1String &word : wordList) {
#endif
                if (word == text.midRef(i, word.size()).toLatin1()) {
                    if ( i + word.size() == textLen ||
                         !text.at(i + word.size()).isLetter()) {
                        currentBlockState() == HighlighterState::CodeCpp ?
                        setFormat(i-1, word.size()+1, formatOther) :
                                    setFormat(i, word.size(), formatOther);
                        i += word.size();
                    }
                }
            }
        }

        //we were unable to find any match, lets skip this word
        if (pos == i) {
            int cnt = i;
            while (cnt < textLen) {
                if (!text[cnt].isLetter()) break;
                ++cnt;
            }
            i = cnt;
        }
    }

    if (isCSS) cssHighlighter(text);
}

/**
 * @brief Highlight string literals in code
 * @param strType str type i.e., ' or "
 * @param text the text being scanned
 * @param i pos of i in loop
 * @return pos of i after the string
 */
int MarkdownHighlighter::highlightStringLiterals(QChar strType, const QString &text, int i) {
    setFormat(i, 1,  _formats[CodeString]);
    ++i;

    while (i < text.length()) {
        //make sure it's not an escape seq
        if (text.at(i) == strType && text.at(i-1) != '\\') {
            setFormat(i, 1,  _formats[CodeString]);
            ++i;
            break;
        }
        //look for escape sequence
        if (text.at(i) == '\\') {
            //look for space
            int spacePos = text.indexOf(' ', i);
            //if space not found, look for the string end
            //this may present problems in very special cases for e.g \"hello\"
            if (spacePos == -1) {
                spacePos = text.indexOf(strType, i);
            }
            setFormat(i, spacePos - i, _formats[CodeNumLiteral]);
            i = spacePos;
        }
        setFormat(i, 1,  _formats[CodeString]);
        ++i;
    }
    return i;
}

/**
 * @brief Highlight number literals in code
 * @param text the text being scanned
 * @param i pos of i in loop
 * @return pos of i after the number
 */
int MarkdownHighlighter::highlightIntegerLiterals(const QString &text, int i)
{
    bool isPreNum = false;
    if (i == 0) isPreNum = true;
    else {
        switch(text[i - 1].toLatin1()) {
        case '[':
        case '(':
        case '{':
        case ' ':
        case ',':
        case '=':
        case '<':
        case '>':
            isPreNum = true;
            break;
        }
    }
    int start = i;

    if ((i+1) >= text.length()) {
        if (isPreNum) setFormat(i, 1, _formats[CodeNumLiteral]);
        return ++i;
    }

    ++i;
    if (text[i] == 'x') ++i;

    if (isPreNum) {
        while (i < text.length()) {
            if (!text[i].isNumber() && text[i] != '.') break;
            ++i;
        }
    } else {
        return i;
    }

    i--;

    bool isPostNum = false;
    if (i+1 == text.length()) isPostNum = true;
    else {
        switch(text[i + 1].toLatin1()) {
        case ']':
        case ')':
        case '}':
        case ' ':
        case ',':
        case '=':
        case '>':
        case '<':
        case ';':
            isPostNum = true;
            break;
        }
    }
    if (isPostNum) {
        int end = ++i;
        setFormat(start, end - start, _formats[CodeNumLiteral]);
    }
    return i;
}

void MarkdownHighlighter::cssHighlighter(const QString &text)
{
    if (text.isEmpty()) return;
    const auto textLen = text.length();
    for (int i = 0; i<textLen; ++i) {
        if (text[i] == QLatin1Char('.') || text[i] == QLatin1Char('#')) {
            if (i+1 >= textLen) return;
            if (text[i + 1].isSpace() || text[i+1].isNumber()) continue;
            int space = text.indexOf(QLatin1Char(' '), i);
            if (space < 0) {
                space = text.indexOf('{');
                if (space < 0) {
                    space = textLen;
                }
            }
            setFormat(i, space - i, _formats[CodeKeyWord]);
            i = space;
        } else if (text[i] == QLatin1Char('c')) {
            if (text.midRef(i, 5) == QLatin1String("color")) {
                i += 5;
                int colon = text.indexOf(QLatin1Char(':'), i);
                if (colon < 0) continue;
                i = colon;
                i++;
                while(i < textLen) {
                    if (!text[i].isSpace()) break;
                    i++;
                }
                int semicolon = text.indexOf(QLatin1Char(';'));
                if (semicolon < 0) semicolon = textLen;
                QString color = text.mid(i, semicolon-i);
                QTextCharFormat f = _formats[CodeBlock];
                QColor c(color);
                if (color.startsWith(QLatin1String("rgb"))) {
                    int t = text.indexOf('(', i);
                    int rPos = text.indexOf(',', t);
                    int gPos = text.indexOf(',', rPos+1);
                    int bPos = text.indexOf(')', gPos);
                    if (rPos > -1 && gPos > -1 && bPos > -1) {
                        const QStringRef r = text.midRef(t+1, rPos - (t+1));
                        const QStringRef g = text.midRef(rPos+1, gPos - (rPos + 1));
                        const QStringRef b = text.midRef(gPos+1, bPos - (gPos+1));
                        c.setRgb(r.toInt(), g.toInt(), b.toInt());
                    } else {
                        c = _formats[HighlighterState::NoState].background().color();
                    }
                }

                if (!c.isValid()) {
                    continue;
                }

                int lightness{};
                QColor foreground;
                //really dark
                if (c.lightness() <= 20) {
                    foreground = Qt::white;
                } else if (c.lightness() > 20 && c.lightness() <= 51){
                    foreground = QColor("#ccc");
                } else if (c.lightness() > 51 && c.lightness() <= 78){
                    foreground = QColor("#bbb");
                } else if (c.lightness() > 78 && c.lightness() <= 110){
                    foreground = QColor("#bbb");
                } else if (c.lightness() > 127) {
                    lightness = c.lightness() + 100;
                    foreground = c.darker(lightness);
                }
                else {
                    lightness = c.lightness() + 100;
                    foreground = c.lighter(lightness);
                }

                f.setBackground(c);
                f.setForeground(foreground);
                setFormat(i, semicolon - i, QTextCharFormat()); //clear prev format
                setFormat(i, semicolon - i, f);
                i = semicolon;
            }
        }
    }
}


void MarkdownHighlighter::xmlHighlighter(const QString &text) {
    if (text.isEmpty()) return;
    const auto textLen = text.length();

    setFormat(0, textLen, _formats[CodeBlock]);

    for (int i = 0; i < textLen; ++i) {
        if (text[i] == QLatin1Char('<') && text[i+1] != QLatin1Char('!')) {

            int found = text.indexOf(QLatin1Char('>'), i);
            if (found > 0) {
                ++i;
                if (text[i] == QLatin1Char('/')) ++i;
                setFormat(i, found - i, _formats[CodeKeyWord]);
            }
        }

        if (text[i] == QLatin1Char('=')) {
            int lastSpace = text.lastIndexOf(QLatin1Char(' '), i);
            if (lastSpace == i-1) lastSpace = text.lastIndexOf(QLatin1Char(' '), i-2);
            if (lastSpace > 0) {
                setFormat(lastSpace, i - lastSpace, _formats[CodeBuiltIn]);
            }
        }

        if (text[i] == QLatin1Char('\"')) {
            int pos = i;
            int cnt = 1;
            ++i;
            //bound check
            if ( (i+1) >= textLen) return;
            while (i < textLen) {
                if (text[i] == QLatin1Char('\"')) {
                    ++cnt;
                    ++i;
                    break;
                }
                ++i; ++cnt;
                //bound check
                if ( (i+1) >= textLen) {
                    ++cnt;
                    break;
                }
            }
            setFormat(pos, cnt, _formats[CodeString]);
        }
    }
}

/**
 * Highlight multi-line frontmatter blocks
 *
 * @param text
 */
void MarkdownHighlighter::highlightFrontmatterBlock(const QString& text) {
    // return if there is no frontmatter in this document
    if (document()->firstBlock().text() != "---") {
        return;
    }

    if (text == "---") {
        bool foundEnd = previousBlockState() == HighlighterState::FrontmatterBlock;

        // return if the frontmatter block was already highlighted in previous blocks,
        // there just can be one frontmatter block
        if (!foundEnd && document()->firstBlock() != currentBlock()) {
            return;
        }

        setCurrentBlockState(foundEnd ? HighlighterState::FrontmatterBlockEnd : HighlighterState::FrontmatterBlock);

        QTextCharFormat &maskedFormat =
                _formats[HighlighterState::MaskedSyntax];
        setFormat(0, text.length(), maskedFormat);
    } else if (previousBlockState() == HighlighterState::FrontmatterBlock) {
        setCurrentBlockState(HighlighterState::FrontmatterBlock);
        setFormat(0, text.length(), _formats[HighlighterState::MaskedSyntax]);
    }
}

/**
 * Highlight multi-line comments
 *
 * @param text
 */
void MarkdownHighlighter::highlightCommentBlock(QString text) {
    bool highlight = false;
    text = text.trimmed();
    QString startText = QStringLiteral("<!--");
    QString endText = QStringLiteral("-->");

    // we will skip this case because that is an inline comment and causes
    // troubles here
    if (text.startsWith(startText) && text.contains(endText)) {
        return;
    }

    if (text.startsWith(startText) ||
            (!text.endsWith(endText) &&
                    (previousBlockState() == HighlighterState::Comment))) {
        setCurrentBlockState(HighlighterState::Comment);
        highlight = true;
    } else if (text.endsWith(endText)) {
        highlight = true;
    }

    if (highlight) {
        setFormat(0, text.length(), _formats[HighlighterState::Comment]);
    }
}

/**
 * Format italics, bolds and links in headings(h1-h6)
 *
 * @param format The format that is being applied
 * @param match The regex match
 * @param capturedGroup The captured group
*/
void MarkdownHighlighter::setHeadingStyles(const QTextCharFormat &format,
                                           const QRegularExpressionMatch &match,
                                           const int capturedGroup) {
    QTextCharFormat f;
    int state = currentBlockState();
    if (state == HighlighterState::H1) f = _formats[H1];
    else if (state == HighlighterState::H2) f = _formats[H2];
    else if (state == HighlighterState::H3) f = _formats[H3];
    else if (state == HighlighterState::H4) f = _formats[H4];
    else if (state == HighlighterState::H5) f = _formats[H5];
    else f = _formats[H6];

    if (format == _formats[HighlighterState::Italic]) {
        f.setFontItalic(true);
        setFormat(match.capturedStart(capturedGroup),
                  match.capturedLength(capturedGroup),
                  f);
        return;
    } else if (format == _formats[HighlighterState::Bold]) {
        setFormat(match.capturedStart(capturedGroup),
                  match.capturedLength(capturedGroup),
                  f);
        return;
    }  else if (format == _formats[HighlighterState::Link]) {
        QTextCharFormat link = _formats[Link];
        link.setFontPointSize(f.fontPointSize());
        if (capturedGroup == 1) {
            setFormat(match.capturedStart(capturedGroup),
                  match.capturedLength(capturedGroup),
                  link);
        }
        return;
    }
/**
 * Waqar144
 * TODO: Test this again and make it work correctly
 * Q: Do we even need this in headings?
 */
//disabling these, as these work, but not as good I think.
//    else if (format == _formats[HighlighterState::InlineCodeBlock]) {
//        QTextCharFormat ff;
//        f.setFontPointSize(1.6);
//        f.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
//        f.setBackground(QColor(220, 220, 220));
//        setFormat(match.capturedStart(capturedGroup),
//                  match.capturedEnd(capturedGroup) - 18,
//                  f);
//        return;
//    }
}

/**
 * Highlights the rules from the _highlightingRules list
 *
 * @param text
 */
void MarkdownHighlighter::highlightAdditionalRules(
        const QVector<HighlightingRule> &rules, const QString& text) {
    const QTextCharFormat &maskedFormat = _formats[HighlighterState::MaskedSyntax];

    for(const HighlightingRule &rule : rules) {
            // continue if another current block state was already set if
            // disableIfCurrentStateIsSet is set
            if (rule.disableIfCurrentStateIsSet &&
                    (currentBlockState() != HighlighterState::NoState)) {
                continue;
            }

            QRegularExpression expression(rule.pattern);
            QRegularExpressionMatchIterator iterator = expression.globalMatch(text);
            int capturingGroup = rule.capturingGroup;
            int maskedGroup = rule.maskedGroup;
            QTextCharFormat &format = _formats[rule.state];

            // store the current block state if useStateAsCurrentBlockState
            // is set
            if (iterator.hasNext() && rule.useStateAsCurrentBlockState) {
                setCurrentBlockState(rule.state);
            }

            // find and format all occurrences
            while (iterator.hasNext()) {
                QRegularExpressionMatch match = iterator.next();

                // if there is a capturingGroup set then first highlight
                // everything as MaskedSyntax and highlight capturingGroup
                // with the real format
                if (capturingGroup > 0) {
                    QTextCharFormat currentMaskedFormat = maskedFormat;
                    // set the font size from the current rule's font format
                    if (format.fontPointSize() > 0) {
                        currentMaskedFormat.setFontPointSize(format.fontPointSize());
                    }

                    if ((currentBlockState() == HighlighterState::H1 ||
                        currentBlockState() == HighlighterState::H2 ||
                        currentBlockState() == HighlighterState::H3 ||
                        currentBlockState() == HighlighterState::H4 ||
                        currentBlockState() == HighlighterState::H5 ||
                        currentBlockState() == HighlighterState::H6) &&
                        format != _formats[HighlighterState::InlineCodeBlock]) {
                        //setHeadingStyles(format, match, maskedGroup);

                    } else {

                        setFormat(match.capturedStart(maskedGroup),
                              match.capturedLength(maskedGroup),
                              currentMaskedFormat);
                    }
                }

                if ((currentBlockState() == HighlighterState::H1 ||
                    currentBlockState() == HighlighterState::H2 ||
                    currentBlockState() == HighlighterState::H3 ||
                    currentBlockState() == HighlighterState::H4 ||
                    currentBlockState() == HighlighterState::H5 ||
                    currentBlockState() == HighlighterState::H6) &&
                    format != _formats[HighlighterState::InlineCodeBlock]) {
                    setHeadingStyles(format, match, capturingGroup);

                } else {

                setFormat(match.capturedStart(capturingGroup),
                          match.capturedLength(capturingGroup),
                          format);
                }
            }
        }
}

void MarkdownHighlighter::setHighlightingOptions(const HighlightingOptions options) {
    _highlightingOptions = options;
}

#include "customMarkdownHighlighter.h"
#include "editorsettingsoptions.h"

CustomMarkdownHighlighter::CustomMarkdownHighlighter(QTextDocument *parent,
                                                     HighlightingOptions highlightingOptions)
    : MarkdownHighlighter(parent, highlightingOptions)
{
    setListsColor(QColor(35, 131, 226)); // accent color

    _formats[static_cast<HighlighterState>(HighlighterState::HorizontalRuler)].clearBackground();
}

void CustomMarkdownHighlighter::setHeaderColors(QColor color)
{
    // set all header colors to the same color
    for (int i = HighlighterState::H1; i <= HighlighterState::H6; ++i) {
        _formats[static_cast<HighlighterState>(i)].setForeground(color);
    }
}

void CustomMarkdownHighlighter::setFontSize(qreal fontSize)
{
    // change font size for all formats
    for (auto &format : _formats) {
        format.setFontPointSize(fontSize);
    }

    // change header sizes
    for (int i = HighlighterState::H1; i <= HighlighterState::H6; ++i) {
        // H1 => size = fontSize * 1.6
        // H6 => size = fontSize * 1.1
        qreal size = fontSize * (1.6 - (i - HighlighterState::H1) * 0.1);
        _formats[static_cast<HighlighterState>(i)].setFontPointSize(size);
    }

    qreal codeBlockFontSize = fontSize - 4;
    _formats[static_cast<HighlighterState>(HighlighterState::InlineCodeBlock)].setFontPointSize(
            codeBlockFontSize);
}

void CustomMarkdownHighlighter::setListsColor(QColor color)
{
    _formats[static_cast<HighlighterState>(HighlighterState::CheckBoxUnChecked)].setForeground(
            color);
    _formats[static_cast<HighlighterState>(HighlighterState::CheckBoxChecked)].setForeground(
            QColor(90, 113, 140));
    _formats[static_cast<HighlighterState>(HighlighterState::List)].setForeground(color);
    _formats[static_cast<HighlighterState>(HighlighterState::BlockQuote)].setForeground(color);
}

void CustomMarkdownHighlighter::setTheme(Theme::Value theme, QColor textColor, qreal fontSize)
{
    setHeaderColors(textColor);
    setFontSize(fontSize);

    switch (theme) {
    case Theme::Light:
        _formats[static_cast<HighlighterState>(HighlighterState::InlineCodeBlock)].setBackground(
                QColor(239, 241, 243));
        _formats[static_cast<HighlighterState>(HighlighterState::InlineCodeBlock)].setForeground(
                QColor(42, 46, 51));
        break;
    case Theme::Dark:
        _formats[static_cast<HighlighterState>(HighlighterState::InlineCodeBlock)].setBackground(
                QColor(52, 57, 66));
        _formats[static_cast<HighlighterState>(HighlighterState::InlineCodeBlock)].setForeground(
                QColor(230, 237, 243));
        break;
    case Theme::Sepia:
        _formats[static_cast<HighlighterState>(HighlighterState::InlineCodeBlock)].setBackground(
                QColor(239, 241, 243));
        _formats[static_cast<HighlighterState>(HighlighterState::InlineCodeBlock)].setForeground(
                QColor(42, 46, 51));
        break;
        break;
    }
}

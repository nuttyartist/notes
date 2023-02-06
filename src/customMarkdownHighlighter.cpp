#include "customMarkdownHighlighter.h"

CustomMarkdownHighlighter::CustomMarkdownHighlighter(QTextDocument *parent,
                                                     HighlightingOptions highlightingOptions)
    : MarkdownHighlighter(parent, highlightingOptions)
{
}

void CustomMarkdownHighlighter::setHeaderColors(QColor color)
{
    // set all header colors to the same color
    for (int i = HighlighterState::H1; i <= HighlighterState::H6; ++i) {
        _formats[static_cast<HighlighterState>(i)].setForeground(color);
    }
}

void CustomMarkdownHighlighter::setTheme(Theme theme, QColor textColor)
{
    setHeaderColors(textColor);

    switch (theme) {
    case Theme::Light:
    case Theme::Dark:
    case Theme::Sepia:
        break;
    }
}

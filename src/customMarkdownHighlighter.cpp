#include "customMarkdownHighlighter.h"

CustomMarkdownHighlighter::CustomMarkdownHighlighter(QTextDocument *parent,
                                                     HighlightingOptions highlightingOptions)
    : MarkdownHighlighter(parent, highlightingOptions)
{
    // set all header colors
    for (int i = HighlighterState::H1; i <= HighlighterState::H6; ++i) {
        _formats[static_cast<HighlighterState>(i)].setForeground(QColor(0, 0, 0));
    }
}
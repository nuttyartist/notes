#pragma once

#include "3rdParty/qmarkdowntextedit/markdownhighlighter.h"
#include "editorsettingsoptions.h"

class CustomMarkdownHighlighter : public MarkdownHighlighter
{
public:
    explicit CustomMarkdownHighlighter(QTextDocument *parent, HighlightingOptions highlightingOptions = HighlightingOption::None);
    ~CustomMarkdownHighlighter() override = default;

    void setFontSize(qreal fontSize);
    void setHeaderColors(QColor color);
    void setListsColor(QColor color);

    void setTheme(Theme::Value theme, QColor textColor, qreal fontSize);
};

#pragma once

#include "3rdParty/qmarkdowntextedit/markdownhighlighter.h"

class CustomMarkdownHighlighter : public MarkdownHighlighter
{
	public:
		CustomMarkdownHighlighter(QTextDocument *parent = nullptr,
			HighlightingOptions highlightingOptions = HighlightingOption::None);
		~CustomMarkdownHighlighter() override = default;
};
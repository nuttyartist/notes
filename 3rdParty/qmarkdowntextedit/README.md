# [QMarkdownTextEdit](https://github.com/pbek/qmarkdowntextedit)
[![Build Status Linux/OS X](https://travis-ci.org/pbek/qmarkdowntextedit.svg?branch=develop)](https://travis-ci.org/pbek/qmarkdowntextedit)
[![Build Status Windows](https://ci.appveyor.com/api/projects/status/github/pbek/qmarkdowntextedit)](https://ci.appveyor.com/project/pbek/qmarkdowntextedit)

QMarkdownTextEdit is a C++ Qt [QPlainTextEdit](http://doc.qt.io/qt-5/qplaintextedit.html) widget with [markdown](https://en.wikipedia.org/wiki/Markdown) highlighting and some other goodies.

## Features
- markdown highlighting
- syntax highlighting
- clickable links with `Ctrl + Click`
- block indent with `Tab` and `Shift + Tab`
- duplicate text with `Ctrl + Alt + Down`
- searching of text with `Ctrl + F`
    - jump between search results with `Up` and `Down`
    - close search field with `Escape`
- replacing of text with `Ctrl + R`
    - you can also replace text with regular expressions or whole words
- and much more...

## Screenshot
![Screenhot](screenshot.png)

## How to use this widget
- include [qmarkdowntextedit.pri](https://github.com/pbek/qmarkdowntextedit/blob/develop/qmarkdowntextedit.pri) 
  to your project like this `include (qmarkdowntextedit/qmarkdowntextedit.pri)`
- add a normal `QPlainTextEdit` to your UI and promote it to `QMarkdownTextEdit` (base class `QPlainTextEdit`)

## References
- [QOwnNotes - cross-platform open source plain-text file markdown note taking](http://www.qownnotes.org)

## Disclaimer
This SOFTWARE PRODUCT is provided by THE PROVIDER "as is" and "with all faults." THE PROVIDER makes no representations or warranties of any kind concerning the safety, suitability, lack of viruses, inaccuracies, typographical errors, or other harmful components of this SOFTWARE PRODUCT. 

There are inherent dangers in the use of any software, and you are solely responsible for determining whether this SOFTWARE PRODUCT is compatible with your equipment and other software installed on your equipment. You are also solely responsible for the protection of your equipment and backup of your data, and THE PROVIDER will not be liable for any damages you may suffer in connection with using, modifying, or distributing this SOFTWARE PRODUCT.

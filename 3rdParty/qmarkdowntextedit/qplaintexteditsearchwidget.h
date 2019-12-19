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

#pragma once

#include <QWidget>
#include <QPlainTextEdit>

namespace Ui {
class QPlainTextEditSearchWidget;
}

class QPlainTextEditSearchWidget : public QWidget
{
    Q_OBJECT

public:
    enum SearchMode {
        PlainTextMode,
        WholeWordsMode,
        RegularExpressionMode
    };

    explicit QPlainTextEditSearchWidget(QPlainTextEdit *parent = 0);
    bool doSearch(bool searchDown = true, bool allowRestartAtTop = true);
    void setDarkMode(bool enabled);
    ~QPlainTextEditSearchWidget();
    void setSearchText(QString &searchText);
    void setSearchMode(SearchMode searchMode);
    void activate(bool focus);

private:
    Ui::QPlainTextEditSearchWidget *ui;
    int _searchResultCount;
    int _currentSearchResult;

protected:
    QPlainTextEdit *_textEdit;
    bool _darkMode;
    bool eventFilter(QObject *obj, QEvent *event);

public slots:
    void activate();
    void deactivate();
    void doSearchDown();
    void doSearchUp();
    void setReplaceMode(bool enabled);
    void activateReplace();
    bool doReplace(bool forAll = false);
    void doReplaceAll();
    void reset();
    void doSearchCount();

protected slots:
    void searchLineEditTextChanged(const QString &arg1);
    void updateSearchCountLabelText();
private slots:
    void on_modeComboBox_currentIndexChanged(int index);
    void on_matchCaseSensitiveButton_toggled(bool checked);
};

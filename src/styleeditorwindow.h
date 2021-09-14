/*********************************************************************************************
* Mozila License
* Just a meantime project to see the ability of qt, the framework that my OS might be based on
* And for those linux users that beleive in the power of notes
*********************************************************************************************/

#ifndef STYLEEDITORWINDOW_H
#define STYLEEDITORWINDOW_H

#include <QWidget>
#include<QPushButton>

namespace Ui {
class StyleEditorWindow;
}

enum class FontTypeface {
    Mono,
    Serif,
    SansSerif
};

enum class FontSizeAction {
    Increase,
    Decrease
};

enum class EditorTextWidth {
    FullWidth,
    Increase,
    Decrease
};

enum class Theme {
    Light,
    Dark,
    Sepia
};

enum class ButtonState {
    Normal,
    Hovered,
    Clicked
};

class StyleEditorWindow : public QWidget
{
    Q_OBJECT

public:
    explicit StyleEditorWindow (QWidget *parent = 0);
    ~StyleEditorWindow();
    void changeSelectedFont(FontTypeface selectedFontType, QString selectedFontName);
    void setTheme(Theme theme, QColor themeColor, QColor textColor);
    void restoreSelectedOptions(bool isTextFullWidth, FontTypeface selectedFontTypeface, Theme selectedTheme);

public slots:
    void toggleWindowVisibility();

signals:
    void changeFontType(FontTypeface fontType);
    void changeFontSize(FontSizeAction fontSizeAction);
    void changeEditorTextWidth(EditorTextWidth editorTextWidth);
    void changeTheme(Theme theme);
    void resetEditorToDefaultSettings();

private slots:

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    QString getStyleSheetForButton(ButtonState buttonState);
    void buttonClicked(QPushButton* button);
    bool isSelectedButton(QPushButton *button);

    Ui::StyleEditorWindow *m_ui;
    QColor m_currentFontColor;
    Theme m_currentTheme;
    QColor m_currentThemeColor;
    QString m_selectedMonoFontFamilyName;
    QString m_selectedSerifFontFamilyName;
    QString m_selectedSansSerifFontFamilyName;

    QPushButton *m_currentlyClickedButton;
    QPushButton *m_currentSelectedFontButton;
    QPushButton *m_currentSelectedThemeButton;
    bool m_isFullWidthClicked;
};

#endif // STYLEEDITORWINDOW_H

/*********************************************************************************************
 * Mozilla License
 * Just a meantime project to see the ability of qt, the framework that my OS might be based on
 * And for those linux users that believe in the power of notes
 *********************************************************************************************/

#ifndef STYLEEDITORWINDOW_H
#define STYLEEDITORWINDOW_H

#include <QDialog>
#include <QPushButton>
#include "theme.h"

namespace Ui {
class StyleEditorWindow;
}

enum class FontTypeface { Mono, Serif, SansSerif };
std::ostream &operator<<(std::ostream &os, const FontTypeface &fontTypeface);
std::string to_string(FontTypeface fontTypeface);

enum class FontSizeAction { Increase, Decrease };

enum class EditorTextWidth { FullWidth, Increase, Decrease };

enum class ButtonState { Normal, Clicked };

class StyleEditorWindow : public QDialog
{
    Q_OBJECT

public:
    explicit StyleEditorWindow(QWidget *parent = 0);
    ~StyleEditorWindow();
    void changeSelectedFont(FontTypeface selectedFontType, const QString &selectedFontName);
    void setTheme(Theme theme);
    void restoreSelectedOptions(bool isTextFullWidth, FontTypeface selectedFontTypeface,
                                Theme selectedTheme);

public slots:
    void toggleWindowVisibility();

signals:
    void changeFontType(FontTypeface fontType);
    void changeFontSize(FontSizeAction fontSizeAction);
    void changeEditorTextWidth(EditorTextWidth editorTextWidth);
    void changeTheme(Theme theme);
    void resetEditorToDefaultSettings();

private slots:

private:
    void setButtonStyle(QPushButton *button, ButtonState buttonState, Theme theme);
    void buttonClicked(QPushButton *button);
    bool isSelectedButton(QPushButton *button);

    Ui::StyleEditorWindow *m_ui;
    Theme m_currentTheme;
    QString m_selectedMonoFontFamilyName;
    QString m_selectedSerifFontFamilyName;
    QString m_selectedSansSerifFontFamilyName;

    QPushButton *m_currentlyClickedButton;
    QPushButton *m_currentSelectedFontButton;
    QPushButton *m_currentSelectedThemeButton;
    bool m_isFullWidthClicked;
};

#endif // STYLEEDITORWINDOW_H

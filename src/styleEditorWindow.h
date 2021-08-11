/*********************************************************************************************
* Mozila License
* Just a meantime project to see the ability of qt, the framework that my OS might be based on
* And for those linux users that beleive in the power of notes
*********************************************************************************************/

#ifndef STYLEEDITORWINDUpdaterWindowOW_H
#define STYLEEDITORWINDOW_H

#include <QWidget>

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

class StyleEditorWindow : public QWidget
{
    Q_OBJECT

public:
    explicit StyleEditorWindow (QWidget *parent = 0);
    ~StyleEditorWindow();
    void changeSelectedFontLabel(FontTypeface selectedFontType, QString selectedFontName);

public slots:


signals:
    void changeFontType(FontTypeface fontType);
    void changeFontSize(FontSizeAction fontSizeAction);
    void changeEditorTextWidth(EditorTextWidth editorTextWidth);
    void changeTheme(Theme theme);
    void resetEditorToDefaultSettings();

private slots:

protected:

private:
    Ui::StyleEditorWindow *m_ui;
};

#endif // STYLEEDITORWINDOW_H

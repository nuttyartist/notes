#ifndef EDITORSETTINGSBUTTON_H
#define EDITORSETTINGSBUTTON_H

#include <QPushButton>
#include "styleeditorwindow.h"

class EditorSettingsButton : public QPushButton
{
    Q_OBJECT
public:
    EditorSettingsButton(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *p);
    void changeFont(const QString &fontName, const QString &fontTypeface, QColor fontColor);
    void setTheme(Theme theme);

private:
    QString m_currentFontName;
    QString m_currentFontTypeface;
    QColor m_currentFontColor;
    Theme m_currentTheme;
};

#endif // EDITORSETTINGSBUTTON_H

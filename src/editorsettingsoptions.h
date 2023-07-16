#ifndef EDITORSETTINGSOPTIONS_H
#define EDITORSETTINGSOPTIONS_H

#include <QObject>
#include <QtQml/qqml.h>
#include <sstream>
#include <QString>
#include <QWidget>
#include "lqtutils_enum.h"

L_DECLARE_ENUM(FontTypeface, SansSerif, Serif, Mono)
L_DECLARE_ENUM(FontSizeAction, FontSizeIncrease, FontSizeDecrease)
L_DECLARE_ENUM(EditorTextWidth, TextWidthIncrease, TextWidthDecrease, TextWidthFullWidth)
L_DECLARE_ENUM(Theme, Light, Dark, Sepia)
L_DECLARE_ENUM(View, TextView, KanbanView)

class EditorSettingsOptions : public QObject
{
    Q_OBJECT

public:
    explicit EditorSettingsOptions(QObject *parent = nullptr);
};

std::ostream &operator<<(std::ostream &os, const Theme::Value &theme);
std::ostream &operator<<(std::ostream &os, const FontTypeface::Value &fontTypeface);
std::string to_string(FontTypeface::Value fontTypeface);
std::string to_string(Theme::Value theme);

void setCSSThemeAndUpdate(QWidget *obj, Theme::Value theme);
void setCSSClassesAndUpdate(QWidget *obj, std::string classNames);

#endif

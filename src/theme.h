#pragma once

#include <sstream>
#include <QWidget>
#include <QString>

enum class Theme { Light, Dark, Sepia };

std::ostream &operator<<(std::ostream &os, const Theme &theme);
std::string to_string(Theme theme);

void setCSSThemeAndUpdate(QWidget *obj, Theme theme);
void setCSSClassesAndUpdate(QWidget *obj, std::string classNames);

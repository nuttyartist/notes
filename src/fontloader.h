#pragma once

#include <QFont>
#include <QString>
#include <QFontDatabase>

namespace font_loader {

inline QFont loadFont(const QString &family, const QString &style, int pointSize)
{
    return QFontDatabase::font(family, style, pointSize);
}
} // namespace font_loader
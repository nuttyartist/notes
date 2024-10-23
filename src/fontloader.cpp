#include "fontloader.h"

QFont FontLoader::loadFont(const QString &family, const QString &style, int pointSize)
{
    return QFontDatabase::font(family, style, pointSize);
}

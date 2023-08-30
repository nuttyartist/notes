#include "fontloader.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
FontLoader::FontLoader() : m_fontDatabase() { }
#endif

QFont FontLoader::loadFont(const QString &family, const QString &style, int pointSize)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return m_fontDatabase.font(family, style, pointSize);
#else
    return QFontDatabase::font(family, style, pointSize);
#endif
}

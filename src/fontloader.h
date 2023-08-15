#pragma once

#include <QFont>
#include <QString>
#include <QFontDatabase>

class FontLoader
{
public:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QFontDatabase m_fontDatabase;
    FontLoader();
#else
    FontLoader() = default;
#endif

    ~FontLoader() = default;

    static FontLoader &getInstance()
    {
        static FontLoader instance;
        return instance;
    }

    QFont loadFont(const QString &family, const QString &style, int pointSize);
};
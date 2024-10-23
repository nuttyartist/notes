#pragma once

#include <QFont>
#include <QString>
#include <QFontDatabase>

class FontLoader
{
public:
    FontLoader() = default;
    ~FontLoader() = default;

    static FontLoader &getInstance()
    {
        static FontLoader instance;
        return instance;
    }

    QFont loadFont(const QString &family, const QString &style, int pointSize);
};
#pragma once

#include <QFont>
#include <QString>
#include <QFontDatabase>

class FontLoader
{
public:
    static FontLoader &getInstance()
    {
        static FontLoader instance;
        return instance;
    }

    FontLoader(const FontLoader &) = delete;
    void operator=(const FontLoader &) = delete;
    QFont loadFont(const QString &family, const QString &style, int pointSize);

private:
    FontLoader() = default;
    ~FontLoader() = default;
};
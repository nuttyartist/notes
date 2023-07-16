#include "editorsettingsoptions.h"
#include <QStyle>
#include <QVariant>
#include <qdebug.h>

EditorSettingsOptions::EditorSettingsOptions(QObject *) { }

std::ostream &operator<<(std::ostream &os, const FontTypeface::Value &fontTypeface)
{
    switch (fontTypeface) {
    case FontTypeface::SansSerif:
        os << "SansSerif";
        break;
    case FontTypeface::Serif:
        os << "Serif";
        break;
    case FontTypeface::Mono:
        os << "Mono";
        break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const Theme::Value &theme)
{
    switch (theme) {
    case Theme::Light:
        os << "Light";
        break;
    case Theme::Dark:
        os << "Dark";
        break;
    case Theme::Sepia:
        os << "Sepia";
        break;
    }
    return os;
}

std::string to_string(FontTypeface::Value fontTypeface)
{
    std::ostringstream oss;
    oss << fontTypeface;
    return oss.str();
}

std::string to_string(Theme::Value theme)
{
    std::ostringstream oss;
    oss << theme;
    return oss.str();
}

void setCSSThemeAndUpdate(QWidget *obj, Theme::Value theme)
{
    setCSSClassesAndUpdate(obj, QString::fromStdString(to_string(theme)).toLower().toStdString());
}

void setCSSClassesAndUpdate(QWidget *obj, std::string classNames)
{
    if (obj->styleSheet().isEmpty()) {
        qWarning() << "setCSSClassesAndUpdate: styleSheet is empty for widget with name "
                   << obj->objectName();
    }
    // set the class
    obj->setProperty("class", classNames.c_str());
    // update the widget
    obj->style()->polish(obj);
    obj->update();
}

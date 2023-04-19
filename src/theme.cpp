#include "theme.h"
#include <QStyle>
#include <QVariant>
#include <qdebug.h>

// ostream operator for Theme
std::ostream &operator<<(std::ostream &os, const Theme &theme)
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

std::string to_string(Theme theme)
{
    std::ostringstream oss;
    oss << theme;
    return oss.str();
}

void setCSSThemeAndUpdate(QWidget *obj, Theme theme)
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
    obj->style()->unpolish(obj);
    obj->style()->polish(obj);
    obj->update();
}

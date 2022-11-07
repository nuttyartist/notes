#include "pushbuttontype.h"
#include <QEvent>

PushButtonType::PushButtonType(QWidget *parent) : QPushButton(parent) { }

bool PushButtonType::event(QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        setIcon(pressedIcon);
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        if (underMouse()) {
            setIcon(hoveredIcon);
        } else {
            setIcon(normalIcon);
        }
    }
    if (event->type() == QEvent::Enter) {
        setIcon(hoveredIcon);
    }

    if (event->type() == QEvent::Leave) {
        setIcon(normalIcon);
    }
    return QPushButton::event(event);
}

void PushButtonType::setPressedIcon(const QIcon &newPressedIcon)
{
    pressedIcon = newPressedIcon;
}

void PushButtonType::setHoveredIcon(const QIcon &newHoveredIcon)
{
    hoveredIcon = newHoveredIcon;
}

void PushButtonType::setNormalIcon(const QIcon &newNormalIcon)
{
    normalIcon = newNormalIcon;
    setIcon(newNormalIcon);
}

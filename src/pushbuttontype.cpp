#include "pushbuttontype.h"
#include <QEvent>

PushButtonType::PushButtonType(QWidget *parent) : QPushButton(parent) { }

bool PushButtonType::event(QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        setIcon(m_pressedIcon);
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        if (underMouse()) {
            setIcon(m_hoveredIcon);
        } else {
            setIcon(m_normalIcon);
        }
    }
    if (event->type() == QEvent::Enter) {
        setIcon(m_hoveredIcon);
    }

    if (event->type() == QEvent::Leave) {
        setIcon(m_normalIcon);
    }
    return QPushButton::event(event);
}

void PushButtonType::setPressedIcon(const QIcon &newPressedIcon)
{
    m_pressedIcon = newPressedIcon;
}

void PushButtonType::setHoveredIcon(const QIcon &newHoveredIcon)
{
    m_hoveredIcon = newHoveredIcon;
}

void PushButtonType::setNormalIcon(const QIcon &newNormalIcon)
{
    m_normalIcon = newNormalIcon;
    setIcon(newNormalIcon);
}

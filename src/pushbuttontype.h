#ifndef PUSHBUTTONTYPE_H
#define PUSHBUTTONTYPE_H

#include <QPushButton>

class PushButtonType : public QPushButton
{
    Q_OBJECT
public:
    explicit PushButtonType(QWidget *parent = nullptr);

    void setNormalIcon(const QIcon &newNormalIcon);
    void setHoveredIcon(const QIcon &newHoveredIcon);
    void setPressedIcon(const QIcon &newPressedIcon);

protected:
    bool event(QEvent *event) override;

private:
    QIcon m_normalIcon;
    QIcon m_hoveredIcon;
    QIcon m_pressedIcon;
};

#endif // PUSHBUTTONTYPE_H

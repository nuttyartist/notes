#include "editorsettingsbutton.h"

#include <QPainter>

EditorSettingsButton::EditorSettingsButton(QWidget *parent)
    : QPushButton(parent), m_currentFontName(QStringLiteral("Roboto"))
{
}

/*!
 * \brief EditorSettingsButton::paintEvent
 * We custom paint the style editor window font buttons
 * so we can design them as we wish, creating multiple labels
 * in different sizes
 * \param p
 */
void EditorSettingsButton::paintEvent(QPaintEvent *p)
{
    QPushButton::paintEvent(p);

    int rowPosX = 0;
    int rowPosY = 0;
    int rowWidth = width();
    int rowHeight = height();
    int currentY = rowPosY;

    QPainter painter(this);
    painter.setPen(m_currentFontColor);

#ifdef __APPLE__
    currentY += 10;
    painter.setFont(QFont(m_currentFontName, 40, QFont::Bold));
#else
    currentY += 6;
    painter.setFont(QFont(m_currentFontName, 36, QFont::Bold));
#endif
    painter.drawText(rowPosX, currentY, rowWidth, rowHeight, Qt::AlignHCenter,
                     QStringLiteral("Aa"));

    QString arrowImagePath;
    switch (m_currentTheme) {
    case Theme::Dark:
        arrowImagePath = QStringLiteral(":images/arrow-right-dark.png");
        break;
    default:
        arrowImagePath = QStringLiteral(":images/arrow-right.png");
        break;
    }

#ifdef __APPLE__
    painter.drawImage(rowPosX + 115, currentY + 30, QImage(arrowImagePath));
    painter.setFont(QFont(QStringLiteral("Roboto"), 9, QFont::Normal));
    painter.drawText(rowPosX + 52, currentY + 42, rowWidth, rowHeight, Qt::AlignHCenter,
                     QStringLiteral("Next"));
#else
    painter.drawImage(rowPosX + 109, currentY + 24, QImage(arrowImagePath));
    painter.setFont(QFont(QStringLiteral("Roboto"), 5, QFont::Normal));
    painter.drawText(rowPosX + 46, currentY + 36, rowWidth, rowHeight, Qt::AlignHCenter,
                     QStringLiteral("Next"));
#endif

#ifdef __APPLE__
    currentY += 48;
    painter.setFont(QFont(m_currentFontName, 14, QFont::Normal));
#else
    currentY += 54;
    painter.setFont(QFont(m_currentFontName, 10, QFont::Normal));
#endif
    painter.drawText(rowPosX, currentY, rowWidth, rowHeight, Qt::AlignHCenter,
                     m_currentFontTypeface);
    currentY += 16;

#ifdef __APPLE__
    painter.setFont(QFont(m_currentFontName, 13, QFont::Normal));
#else
    painter.setFont(QFont(m_currentFontName, 9, QFont::Normal));
#endif
    painter.drawText(rowPosX, currentY, rowWidth, rowHeight, Qt::AlignHCenter, m_currentFontName);
}

/*!
 * \brief EditorSettingsButton::changeFont
 * Change the font used for the button
 * \param fontName
 * \param fontTypeface
 * \param fontColor
 */
void EditorSettingsButton::changeFont(const QString &fontName, const QString &fontTypeface,
                                      QColor fontColor)
{
    m_currentFontName = fontName;
    m_currentFontTypeface = fontTypeface;
    m_currentFontColor = fontColor;
}

/*!
 * \brief EditorSettingsButton::changeFont
 * Change the font used for the button
 * \param fontName
 * \param fontTypeface
 * \param fontColor
 */
void EditorSettingsButton::setTheme(Theme theme)
{
    m_currentTheme = theme;
}

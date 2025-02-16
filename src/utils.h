#pragma once

#include <cmath>
#include <QString>
#include <QDateTime>

namespace utils {

/**
 * @brief
 *
 * @param x
 * @param center
 * @param sigma
 * @return double
 */
inline double gaussianDist(double x, double center, double sigma)
{
    return (1.0 / (2 * M_PI * pow(sigma, 2)) * exp(-pow(x - center, 2) / (2 * pow(sigma, 2))));
}

/**
 * @brief
 *
 * @param dateTime
 * @return QString
 */
inline QString parseDateTime(const QDateTime &dateTime)
{
    QLocale usLocale(QLocale("en_US"));

    auto currDateTime = QDateTime::currentDateTime();

    if (dateTime.date() == currDateTime.date()) {
        return usLocale.toString(dateTime.time(), "h:mm A");
    }
    if (dateTime.daysTo(currDateTime) == 1) {
        return "Yesterday";
    }
    if (dateTime.daysTo(currDateTime) >= 2 && dateTime.daysTo(currDateTime) <= 7) {
        return usLocale.toString(dateTime.date(), "dddd");
    }

    return dateTime.date().toString("M/d/yy");
}

} // namespace utils
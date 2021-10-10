#ifndef TAGDATA_H
#define TAGDATA_H

#include <QString>
#include <QMetaClassInfo>

class TagData
{
public:
    TagData();

    int id() const;
    void setId(int newId);

    const QString &name() const;
    void setName(const QString &newName);

    const QString &color() const;
    void setColor(const QString &newColor);

    int relativePosition() const;
    void setRelativePosition(int newRelativePosition);

private:
    int m_id;
    QString m_name;
    QString m_color;
    int m_relativePosition;
};

Q_DECLARE_METATYPE(TagData)

#endif // TAGDATA_H

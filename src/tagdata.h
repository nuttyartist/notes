#ifndef TAGDATA_H
#define TAGDATA_H

#include <QString>
#include <QMetaClassInfo>

namespace SpecialTagID {
enum Value {
    InvalidTagId = -1,
};
}

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

    int childNotesCount() const;
    void setChildNotesCount(int newChildCount);

private:
    int m_id;
    QString m_name;
    QString m_color;
    int m_relativePosition;
    int m_childNotesCount;
};

Q_DECLARE_METATYPE(TagData)

#endif // TAGDATA_H

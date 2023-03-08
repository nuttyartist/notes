#include "tagdata.h"

TagData::TagData()
    : m_id{ SpecialTagID::InvalidTagId }, m_relativePosition{ -1 }, m_childNotesCount{ 0 }
{
}

int TagData::id() const
{
    return m_id;
}

void TagData::setId(int newId)
{
    m_id = newId;
}

const QString &TagData::name() const
{
    return m_name;
}

void TagData::setName(const QString &newName)
{
    m_name = newName;
}

const QString &TagData::color() const
{
    return m_color;
}

void TagData::setColor(const QString &newColor)
{
    m_color = newColor;
}

int TagData::relativePosition() const
{
    return m_relativePosition;
}

void TagData::setRelativePosition(int newRelativePosition)
{
    m_relativePosition = newRelativePosition;
}

int TagData::childNotesCount() const
{
    return m_childNotesCount;
}

void TagData::setChildNotesCount(int newChildCount)
{
    m_childNotesCount = newChildCount;
}

#include "notedata.h"

NoteData::NoteData(QObject *parent)
    : QObject(parent),
      m_isModified(false),
      m_isSelected(false),
      m_scrollBarPosition(0)
{

}

QString NoteData::id() const
{
    return m_id;
}

void NoteData::setId(const QString &id)
{
    m_id = id;
}

QString NoteData::fullTitle() const
{
    return m_fullTitle;
}

void NoteData::setFullTitle(const QString &fullTitle)
{
    m_fullTitle = fullTitle;
}

QDateTime NoteData::dateTime() const
{
    return m_dateTime;
}

void NoteData::setDateTime(const QDateTime &dateTime)
{
    m_dateTime = dateTime;
}

QString NoteData::content() const
{
    return m_content;
}

void NoteData::setContent(const QString &content)
{
    m_content = content;
}

bool NoteData::isModified() const
{
    return m_isModified;
}

void NoteData::setModified(bool isModified)
{
    m_isModified = isModified;
}

bool NoteData::isSelected() const
{
    return m_isSelected;
}

void NoteData::setSelected(bool isSelected)
{
    m_isSelected = isSelected;
}

int NoteData::scrollBarPosition() const
{
    return m_scrollBarPosition;
}

void NoteData::setScrollBarPosition(int scrollBarPosition)
{
    m_scrollBarPosition = scrollBarPosition;
}

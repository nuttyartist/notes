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

QDateTime NoteData::lastModificationdateTime() const
{
    return m_lastModificationDateTime;
}

void NoteData::setLastModificationDateTime(const QDateTime &lastModificationdateTime)
{
    m_lastModificationDateTime = lastModificationdateTime;
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

QDateTime NoteData::deletionDateTime() const
{
    return m_deletionDateTime;
}

void NoteData::setDeletionDateTime(const QDateTime& deletionDateTime)
{
    m_deletionDateTime = deletionDateTime;
}

QDateTime NoteData::creationDateTime() const
{
    return m_creationDateTime;
}

void NoteData::setCreationDateTime(const QDateTime&creationDateTime)
{
    m_creationDateTime = creationDateTime;
}

NoteExport NoteData::exportNote() {
    NoteExport noteExport;
    noteExport.id = m_id;
    noteExport.fullTitle = m_fullTitle;
    noteExport.creationDateTime = m_creationDateTime;
    noteExport.lastModificationDateTime = m_lastModificationDateTime;
    noteExport.content = m_content;
    return noteExport;
}

QDataStream &operator<<(QDataStream &stream, const NoteExport &noteExport) {
    return stream << noteExport.id << noteExport.fullTitle << noteExport.creationDateTime << noteExport.lastModificationDateTime<< noteExport.content;
}

QDataStream &operator>>(QDataStream &stream, NoteExport &noteExport) {
    return stream >> noteExport.id >> noteExport.fullTitle >> noteExport.creationDateTime >> noteExport.lastModificationDateTime >> noteExport.content;
}

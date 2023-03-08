#include "nodedata.h"
#include <QDataStream>

NodeData::NodeData()
    : m_id{ SpecialNodeID::InvalidNodeId },
      m_isModified(false),
      m_isSelected(false),
      m_scrollBarPosition(0),
      m_isTempNote{ false },
      m_isPinnedNote{ false },
      m_tagListScrollBarPos{ 0 },
      m_relativePosAN{ 0 },
      m_childNotesCount{ 0 }
{
}

int NodeData::id() const
{
    return m_id;
}

void NodeData::setId(int id)
{
    m_id = id;
}

QString NodeData::fullTitle() const
{
    return m_fullTitle;
}

void NodeData::setFullTitle(const QString &fullTitle)
{
    m_fullTitle = fullTitle;
}

QDateTime NodeData::lastModificationdateTime() const
{
    return m_lastModificationDateTime;
}

void NodeData::setLastModificationDateTime(const QDateTime &lastModificationdateTime)
{
    m_lastModificationDateTime = lastModificationdateTime;
}

QString NodeData::content() const
{
    return m_content;
}

void NodeData::setContent(const QString &content)
{
    m_content = content;
}

bool NodeData::isModified() const
{
    return m_isModified;
}

void NodeData::setModified(bool isModified)
{
    m_isModified = isModified;
}

bool NodeData::isSelected() const
{
    return m_isSelected;
}

void NodeData::setSelected(bool isSelected)
{
    m_isSelected = isSelected;
}

int NodeData::scrollBarPosition() const
{
    return m_scrollBarPosition;
}

void NodeData::setScrollBarPosition(int scrollBarPosition)
{
    m_scrollBarPosition = scrollBarPosition;
}

QDateTime NodeData::deletionDateTime() const
{
    return m_deletionDateTime;
}

void NodeData::setDeletionDateTime(const QDateTime &deletionDateTime)
{
    m_deletionDateTime = deletionDateTime;
}

NodeData::Type NodeData::nodeType() const
{
    return m_nodeType;
}

void NodeData::setNodeType(NodeData::Type newNodeType)
{
    m_nodeType = newNodeType;
}

int NodeData::parentId() const
{
    return m_parentId;
}

void NodeData::setParentId(int newParentId)
{
    m_parentId = newParentId;
}

int NodeData::relativePosition() const
{
    return m_relativePosition;
}

void NodeData::setRelativePosition(int newRelativePosition)
{
    m_relativePosition = newRelativePosition;
}

const QString &NodeData::absolutePath() const
{
    return m_absolutePath;
}

void NodeData::setAbsolutePath(const QString &newAbsolutePath)
{
    m_absolutePath = newAbsolutePath;
}

const QSet<int> &NodeData::tagIds() const
{
    return m_tagIds;
}

void NodeData::setTagIds(const QSet<int> &newTagIds)
{
    m_tagIds = newTagIds;
}

bool NodeData::isTempNote() const
{
    return m_isTempNote;
}

void NodeData::setIsTempNote(bool newIsTempNote)
{
    m_isTempNote = newIsTempNote;
}

const QString &NodeData::parentName() const
{
    return m_parentName;
}

void NodeData::setParentName(const QString &newParentName)
{
    m_parentName = newParentName;
}

bool NodeData::isPinnedNote() const
{
    return m_isPinnedNote;
}

void NodeData::setIsPinnedNote(bool newIsPinnedNote)
{
    m_isPinnedNote = newIsPinnedNote;
}

int NodeData::tagListScrollBarPos() const
{
    return m_tagListScrollBarPos;
}

void NodeData::setTagListScrollBarPos(int newTagListScrollBarPos)
{
    m_tagListScrollBarPos = newTagListScrollBarPos;
}

int NodeData::relativePosAN() const
{
    return m_relativePosAN;
}

void NodeData::setRelativePosAN(int newRelativePosAN)
{
    m_relativePosAN = newRelativePosAN;
}

int NodeData::childNotesCount() const
{
    return m_childNotesCount;
}

void NodeData::setChildNotesCount(int newChildCount)
{
    m_childNotesCount = newChildCount;
}

QDateTime NodeData::creationDateTime() const
{
    return m_creationDateTime;
}

void NodeData::setCreationDateTime(const QDateTime &creationDateTime)
{
    m_creationDateTime = creationDateTime;
}

QDataStream &operator>>(QDataStream &stream, NodeData &nodeData)
{
    int id;
    QString fullTitle;
    QDateTime lastModificationDateTime;
    QDateTime creationDateTime;
    QString content;
    stream >> id >> fullTitle >> creationDateTime >> lastModificationDateTime >> content;
    nodeData.setId(id);
    nodeData.setFullTitle(fullTitle);
    nodeData.setLastModificationDateTime(lastModificationDateTime);
    nodeData.setCreationDateTime(creationDateTime);
    nodeData.setContent(content);
    return stream;
}

QDataStream &operator>>(QDataStream &stream, NodeData *&nodeData)
{
    nodeData = new NodeData();
    QString id;
    QString fullTitle;
    QDateTime lastModificationDateTime;
    QDateTime creationDateTime;
    QString content;
    stream >> id >> fullTitle >> creationDateTime >> lastModificationDateTime >> content;
    nodeData->setId(SpecialNodeID::InvalidNodeId);
    nodeData->setFullTitle(fullTitle);
    nodeData->setLastModificationDateTime(lastModificationDateTime);
    nodeData->setCreationDateTime(creationDateTime);
    nodeData->setContent(content);
    return stream;
}

#include "nodedata.h"
#include <QDataStream>

NodeData::NodeData():
    m_id{SpecialNodeID::InvalidNoteId},
    m_isModified(false),
    m_isSelected(false),
    m_scrollBarPosition(0),
    m_isTempNote{false}
{

}

int NodeData::id() const
{
    return m_id;
}

void NodeData::setId(const int &id)
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

void NodeData::setDeletionDateTime(const QDateTime& deletionDateTime)
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

QDateTime NodeData::creationDateTime() const
{
    return m_creationDateTime;
}

void NodeData::setCreationDateTime(const QDateTime&creationDateTime)
{
    m_creationDateTime = creationDateTime;
}

QDataStream &operator<<(QDataStream &stream, const NodeData* nodeData) {
    return stream << nodeData->id() << nodeData->fullTitle() << nodeData->creationDateTime() << nodeData->lastModificationdateTime() << nodeData->content();
}

QDataStream &operator>>(QDataStream &stream, NodeData* &nodeData){
    nodeData = new NodeData();
    int id;
    QString fullTitle;
    QDateTime lastModificationDateTime;
    QDateTime creationDateTime;
    QString content;
    stream >> id >> fullTitle >> creationDateTime >> lastModificationDateTime >> content;
    nodeData->setId(id);
    nodeData->setFullTitle(fullTitle);
    nodeData->setLastModificationDateTime(lastModificationDateTime);
    nodeData->setCreationDateTime(creationDateTime);
    nodeData->setContent(content);
    return stream;
}

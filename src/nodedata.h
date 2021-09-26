#ifndef NODEDATA_H
#define NODEDATA_H

#include <QObject>
#include <QDateTime>
#include <QList>

namespace SpecialNodeID {
    enum Value {
        RootFolder = 0,
        TrashFolder = 1,
        DefaultNotesFolder = 2,
    };
}

class NodeData
{
public:
    explicit NodeData();

    enum Type {
        Note = 0,
        Folder
    };

    int id() const;
    void setId(const int& id);

    QString fullTitle() const;
    void setFullTitle(const QString &fullTitle);

    QDateTime lastModificationdateTime() const;
    void setLastModificationDateTime(const QDateTime &lastModificationdateTime);

    QDateTime creationDateTime() const;
    void setCreationDateTime(const QDateTime& creationDateTime);

    QString content() const;
    void setContent(const QString &content);

    bool isModified() const;
    void setModified(bool isModified);

    bool isSelected() const;
    void setSelected(bool isSelected);

    int scrollBarPosition() const;
    void setScrollBarPosition(int scrollBarPosition);

    QDateTime deletionDateTime() const;
    void setDeletionDateTime(const QDateTime& deletionDateTime);

    NodeData::Type nodeType() const;
    void setNodeType(NodeData::Type newNodeType);

    int parentId() const;
    void setParentId(int newParentId);

    int relativePosition() const;
    void setRelativePosition(int newRelativePosition);

private:
    int m_id;
    QString m_fullTitle;
    QDateTime m_lastModificationDateTime;
    QDateTime m_creationDateTime;
    QDateTime m_deletionDateTime;
    QString m_content;
    bool m_isModified;
    bool m_isSelected;
    int m_scrollBarPosition;
    NodeData::Type m_nodeType;
    int m_parentId;
    int m_relativePosition;
};

Q_DECLARE_METATYPE(NodeData)

QDataStream &operator<<(QDataStream &stream, const NodeData* nodeData);
QDataStream &operator>>(QDataStream &stream, NodeData *&nodeData);

#endif // NODEDATA_H

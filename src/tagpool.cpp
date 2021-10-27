#include "tagpool.h"
#include "dbmanager.h"

TagPool::TagPool(DBManager *dbManager, QObject *parent) :
    QObject(parent),
    m_dbManager{dbManager}
{
    connect(m_dbManager, &DBManager::nodesTagTreeReceived,
            this, [this] (const NodeTagTreeData& treeData) {
        QMap<int, TagData> newPool;
        for (const auto& tag: treeData.tagTreeData) {
            newPool[tag.id()] = tag;
        }
        setTagPool(newPool);
    }, Qt::QueuedConnection);
    connect(m_dbManager, &DBManager::tagAdded,
            this, &TagPool::onTagAdded, Qt::QueuedConnection);
}

void TagPool::setTagPool(const QMap<int, TagData> &newPool)
{
    m_pool = newPool;
    emit dataReseted();
}

void TagPool::onTagDeleted(int id)
{
    m_pool.remove(id);
}

void TagPool::onTagAdded(const TagData &tag)
{
    m_pool[tag.id()] = tag;
}

void TagPool::onTagChanged(const TagData &tag)
{
    m_pool[tag.id()] = tag;
}

TagData TagPool::getTag(int id) const
{
    return m_pool[id];
}

bool TagPool::contains(int id) const
{
    return m_pool.contains(id);
}

QList<int> TagPool::tagIds() const
{
    return m_pool.keys();
}

#include "taglistmodel.h"
#include "tagpool.h"
#include <QDebug>
#include "nodepath.h"

TagListModel::TagListModel(QObject *parent) : QAbstractListModel(parent), m_tagPool{ nullptr } { }

void TagListModel::setTagPool(TagPool *tagPool)
{
    beginResetModel();
    m_tagPool = tagPool;
    connect(tagPool, &TagPool::dataReset, this, [this] {
        beginResetModel();
        updateTagData();
        endResetModel();
    });
    endResetModel();
}

void TagListModel::setModelData(const QSet<int> &data)
{
    if (!m_tagPool) {
        qDebug() << __FUNCTION__ << "Tag pool is not init yet";
        return;
    }
    beginResetModel();
    m_ids = data;
    updateTagData();
    endResetModel();
}

void TagListModel::addTag(int tagId)
{
    if (!m_tagPool) {
        qDebug() << __FUNCTION__ << "Tag pool is not init yet";
        return;
    }
    if (m_tagPool->contains(tagId)) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_data.append(m_tagPool->getTag(tagId));
        endInsertRows();
    } else {
        qDebug() << __FUNCTION__ << "Tag is not in pool:" << tagId;
    }
}

int TagListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.count();
}

QVariant TagListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    const TagData &tag = m_data[index.row()];
    if (role == IdRole) {
        return tag.id();
    } else if (role == NameRole) {
        return tag.name();
    } else if (role == ColorRole) {
        return tag.color();
    }
    return QVariant();
}

void TagListModel::updateTagData()
{
    m_data.clear();
    for (const auto &id : qAsConst(m_ids)) {
        if (m_tagPool->contains(id)) {
            m_data.append(m_tagPool->getTag(id));
        } else {
            qDebug() << __FUNCTION__ << "Tag is not in pool:" << id;
        }
    }
}

#ifndef TAGLISTMODEL_H
#define TAGLISTMODEL_H

#include <QAbstractListModel>
#include <QSet>
#include "tagdata.h"

class TagPool;
class TagListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum TagListRoles { IdRole = Qt::UserRole + 1, NameRole, ColorRole };
    TagListModel(QObject *parent = nullptr);
    void setTagPool(TagPool *tagPool);
    void setModelData(const QSet<int> &data);
    void addTag(int tagId);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
    TagPool *m_tagPool;
    QVector<TagData> m_data;
    QSet<int> m_ids;

    void updateTagData();
};

#endif // TAGLISTMODEL_H

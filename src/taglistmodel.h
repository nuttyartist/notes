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
    enum class TagListRole : std::uint16_t {
        IdRole = Qt::UserRole + 1,
        NameRole,
        ColorRole,
    };

    explicit TagListModel(QObject *parent);
    void setTagPool(TagPool *tagPool);
    void setModelData(const QSet<int> &data);
    void addTag(int tagId);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    TagPool *m_tagPool;
    QVector<TagData> m_data;
    QSet<int> m_ids;

    void updateTagData();
};

#endif // TAGLISTMODEL_H

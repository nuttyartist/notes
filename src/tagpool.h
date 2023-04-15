#ifndef TAGPOOL_H
#define TAGPOOL_H

#include <QObject>
#include <QMap>
#include "tagdata.h"

class DBManager;

class TagPool : public QObject
{
    Q_OBJECT
public:
    explicit TagPool(DBManager *dbManager, QObject *parent = nullptr);
    TagData getTag(int id) const;
    bool contains(int id) const;
    QList<int> tagIds() const;

signals:
    void dataReset();
    void dataUpdated(int tagId);
    void tagDeleted(int tagId);

private slots:
    void onTagDeleted(int id);
    void onTagAdded(const TagData &tag);
    void onTagRenamed(int id, const QString &newName);
    void onTagColorChanged(int id, const QString &newColor);

private:
    QMap<int, TagData> m_pool;
    DBManager *m_dbManager;

    void setTagPool(const QMap<int, TagData> &newPool);
};

#endif // TAGPOOL_H

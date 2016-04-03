#ifndef NOTEMODEL_H
#define NOTEMODEL_H

#include <QAbstractListModel>
#include "notedata.h"

class NoteModel : public QAbstractListModel
{
public:

    enum NoteRoles{
        NoteID = Qt::UserRole + 1,
        NoteFullTitle,
        NoteDateTime,
        NoteContent
    };

    explicit NoteModel(QObject *parent = Q_NULLPTR);
    ~NoteModel();

    void addNote(NoteData* note);
    void insertNote(NoteData* note, int row);
    void removeNote(NoteData* note);
    bool moveRow(const QModelIndex &sourceParent,
                 int sourceRow,
                 const QModelIndex &destinationParent,
                 int destinationChild);

    void clearNotes();
    void copyFromModel(NoteModel *model);
    NoteData* getNote(const QModelIndex &index) const;
    QModelIndex getNoteIndex(NoteData* note) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    void sort(int column, Qt::SortOrder order) override;

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<NoteData *> m_noteList;
};

#endif // NOTEMODEL_H

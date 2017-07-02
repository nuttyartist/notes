#ifndef NOTEMODEL_H
#define NOTEMODEL_H

#include <QAbstractListModel>
#include "notedata.h"

class NoteModel : public QAbstractListModel
{

    friend class tst_NoteModel;

public:

    enum NoteRoles{
        NoteID = Qt::UserRole + 1,
        NoteFullTitle,
        NoteCreationDateTime,
        NoteLastModificationDateTime,
        NoteDeletionDateTime,
        NoteContent,
        NoteScrollbarPos
    };

    explicit NoteModel(QObject *parent = Q_NULLPTR);
    ~NoteModel();

    QModelIndex addNote(NoteData* note);
    QModelIndex insertNote(NoteData* note, int row);
    NoteData* getNote(const QModelIndex& index);
    void addListNote(QList<NoteData*> noteList);
    NoteData* removeNote(const QModelIndex& noteIndex);
    bool moveRow(const QModelIndex& sourceParent,
                 int sourceRow,
                 const QModelIndex& destinationParent,
                 int destinationChild);

    void clearNotes();
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    void sort(int column, Qt::SortOrder order) Q_DECL_OVERRIDE;

private:
    QList<NoteData *> m_noteList;

signals:
    void noteRemoved();
};

#endif // NOTEMODEL_H

#ifndef NOTELISTMODEL_H
#define NOTELISTMODEL_H

#include <QAbstractListModel>
#include "nodedata.h"

class NoteListModel : public QAbstractListModel
{

public:

    enum NoteRoles{
        NoteID = Qt::UserRole + 1,
        NoteFullTitle,
        NoteCreationDateTime,
        NoteLastModificationDateTime,
        NoteDeletionDateTime,
        NoteContent,
        NoteScrollbarPos,
        NoteTagsList,
        NoteIsTemp,
    };

    explicit NoteListModel(QObject *parent = Q_NULLPTR);
    ~NoteListModel();

    QModelIndex addNote(const NodeData& note);
    QModelIndex insertNote(const NodeData& note, int row);
    NodeData getNote(const QModelIndex& index) const;
    QModelIndex getNoteIndex(int id) const;
    void setListNote(const QVector<NodeData> notes);
    void removeNote(const QModelIndex& noteIndex);
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
    QVector<NodeData> m_noteList;

signals:
    void noteRemoved();
};

#endif // NOTELISTMODEL_H

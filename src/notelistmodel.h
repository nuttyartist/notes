#ifndef NOTELISTMODEL_H
#define NOTELISTMODEL_H

#include <QAbstractListModel>
#include "nodedata.h"
#include "dbmanager.h"

class NoteListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum NoteRoles {
        NoteID = Qt::UserRole + 1,
        NoteFullTitle,
        NoteCreationDateTime,
        NoteLastModificationDateTime,
        NoteDeletionDateTime,
        NoteContent,
        NoteScrollbarPos,
        NoteTagsList,
        NoteIsTemp,
        NoteParentName,
        NoteTagListScrollbarPos,
        NoteIsPinned,
    };

    explicit NoteListModel(QObject *parent = nullptr);
    ~NoteListModel();

    QModelIndex addNote(const NodeData &note);
    QModelIndex insertNote(const NodeData &note, int row);
    const NodeData &getNote(const QModelIndex &index) const;
    QModelIndex getNoteIndex(int id) const;
    void setListNote(const QVector<NodeData> &notes, const ListViewInfo &inf);
    void removeNotes(const QModelIndexList &noteIndexes);
    bool moveRow(const QModelIndex &sourceParent, int sourceRow,
                 const QModelIndex &destinationParent, int destinationChild);

    void clearNotes();
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    void sort(int column, Qt::SortOrder order) override;
    void setNoteData(const QModelIndex &index, const NodeData &note);

    virtual Qt::DropActions supportedDropActions() const override;
    virtual Qt::DropActions supportedDragActions() const override;
    virtual QStringList mimeTypes() const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
    virtual bool dropMimeData(const QMimeData *mime, Qt::DropAction action, int row, int column,
                              const QModelIndex &parent) override;

    bool noteIsHaveTag(const QModelIndex &index) const;
    bool isFirstPinnedNote(const QModelIndex &index) const;
    bool isFirstUnpinnedNote(const QModelIndex &index) const;
    QModelIndex getFirstPinnedNote() const;
    QModelIndex getFirstUnpinnedNote() const;
    bool hasPinnedNote() const;
    void setNotesIsPinned(const QModelIndexList &indexes, bool isPinned);

private:
    QVector<NodeData> m_noteList;
    QVector<NodeData> m_pinnedList;
    ListViewInfo m_listViewInfo;
    void updatePinnedRelativePosition();
    bool isInAllNote() const;
    NodeData &getRef(int row);
    const NodeData &getRef(int row) const;

signals:
    void rowCountChanged();
    void requestUpdatePinned(int noteId, bool isPinned);
    void requestUpdatePinnedRelPos(int noteId, int pos);
    void requestUpdatePinnedRelPosAN(int noteId, int pos);
    void requestRemoveNotes(QModelIndexList index);
    void rowsInsertedC(const QModelIndexList &rows);
    void rowsAboutToBeMovedC(const QModelIndexList &source);
    void rowsMovedC(const QModelIndexList &dest);
    void requestCloseNoteEditor(const QModelIndexList &indexes);
    void requestOpenNoteEditor(const QModelIndexList &indexes);
    void selectNotes(const QModelIndexList &indexes);

    // QAbstractItemModel interface
public:
    bool removeRows(int row, int count, const QModelIndex &parent) override;
};

#endif // NOTELISTMODEL_H

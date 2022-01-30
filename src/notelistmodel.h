#ifndef NOTELISTMODEL_H
#define NOTELISTMODEL_H

#include <QAbstractListModel>
#include "nodedata.h"
#include "dbmanager.h"

class NoteListModel : public QAbstractListModel
{
    Q_OBJECT
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
        NoteParentName,
        NoteTagListScrollbarPos,
        NoteIsPinned,
    };

    explicit NoteListModel(QObject *parent = Q_NULLPTR);
    ~NoteListModel();

    QModelIndex addNote(const NodeData& note);
    QModelIndex insertNote(const NodeData& note, int row);
    NodeData getNote(const QModelIndex& index) const;
    QModelIndex getNoteIndex(int id) const;
    void setListNote(const QVector<NodeData> notes, const ListViewInfo& inf);
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
    void setNoteData(const QModelIndex& index, const NodeData& note);

    virtual Qt::DropActions supportedDropActions() const override;
    virtual Qt::DropActions supportedDragActions() const override;
    virtual QStringList mimeTypes() const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
    virtual bool dropMimeData(const QMimeData *mime, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    QModelIndex firstUnpinnedIndex() const;
    bool noteIsHaveTag(const QModelIndex index) const;
private slots:
    void onPinnedChanged(const QModelIndex& index, bool isPinned);

private:
    QVector<NodeData> m_noteList;
    QVector<NodeData> m_pinnedList;
    ListViewInfo m_listViewInfo;
    void updatePinnedRelativePosition();
    bool isInAllNote() const;
    bool noteIsPinned(const NodeData& note) const;

signals:
    void noteRemoved();
    void rowCountChanged();
    void pinnedChanged(const QModelIndex& index, bool isPinned);
    void requestUpdatePinned(int noteId, bool isPinned);
    void requestUpdatePinnedAN(int noteId, bool isPinned);
    void requestUpdatePinnedRelPos(int noteId, int pos);
    void requestUpdatePinnedRelPosAN(int noteId, int pos);
    void setCurrentIndex(const QModelIndex& index);
};

#endif // NOTELISTMODEL_H

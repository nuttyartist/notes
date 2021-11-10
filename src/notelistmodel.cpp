#include "notelistmodel.h"
#include <QDebug>

NoteListModel::NoteListModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

NoteListModel::~NoteListModel()
{

}

QModelIndex NoteListModel::addNote(const NodeData& note)
{
    const int rowCnt = rowCount();
    beginInsertRows(QModelIndex(), rowCnt, rowCnt);
    m_noteList << note;
    endInsertRows();

    return createIndex(rowCnt, 0);
}

QModelIndex NoteListModel::insertNote(const NodeData &note, int row)
{
    if(row >= rowCount()){
        return addNote(note);
    } else {
        beginInsertRows(QModelIndex(), row, row);
        m_noteList.insert(row, note);
        endInsertRows();
    }

    return createIndex(row, 0);
}

NodeData NoteListModel::getNote(const QModelIndex& index) const
{
    return m_noteList.at(index.row());
}

QModelIndex NoteListModel::getNoteIndex(int id) const
{
    for (int i = 0; i < m_noteList.size(); ++i) {
        if (m_noteList[i].id() == id) {
            return createIndex(i, 0);
        }
    }
    return QModelIndex{};
}

void NoteListModel::setListNote(const QVector<NodeData> notes)
{
    beginResetModel();
    m_noteList = notes;
    endResetModel();
}

void NoteListModel::removeNote(const QModelIndex &noteIndex)
{
    if (noteIndex.isValid()) {
        int row = noteIndex.row();
        beginRemoveRows(QModelIndex(), row, row);
        m_noteList.takeAt(row);
        endRemoveRows();
    }
}

bool NoteListModel::moveRow(const QModelIndex &sourceParent, int sourceRow, const QModelIndex &destinationParent, int destinationChild)
{
    if(sourceRow<0
            || sourceRow >= m_noteList.count()
            || destinationChild <0
            || destinationChild >= m_noteList.count()){

        return false;
    }

    if (beginMoveRows(sourceParent,sourceRow,sourceRow,destinationParent,destinationChild)) {
        m_noteList.move(sourceRow,destinationChild);
        endMoveRows();
        return true;
    }
    return false;
}

void NoteListModel::clearNotes()
{
    beginResetModel();
    m_noteList.clear();
    endResetModel();
}

QVariant NoteListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_noteList.count())
        return QVariant();

    const auto& note = m_noteList[index.row()];
    if(role == NoteID){
        return note.id();
    }else if(role == NoteFullTitle){
        return note.fullTitle();
    }else if(role == NoteCreationDateTime){
        return note.creationDateTime();
    }else if(role == NoteLastModificationDateTime){
        return note.lastModificationdateTime();
    }else if(role == NoteDeletionDateTime){
        return note.deletionDateTime();
    }else if(role == NoteContent){
        return note.content();
    }else if(role == NoteScrollbarPos){
        return note.scrollBarPosition();
    }else if(role == NoteTagsList){
        return QVariant::fromValue(note.tagIds());
    }else if(role == NoteIsTemp){
        return note.isTempNote();
    }else if(role == NodeParentName){
        return note.parentName();
    }

    return QVariant();
}

bool NoteListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    NodeData& note = m_noteList[index.row()];

    if(role == NoteID){
        note.setId(value.toInt());
    }else if(role == NoteFullTitle){
        note.setFullTitle(value.toString());
    }else if(role == NoteCreationDateTime){
        note.setCreationDateTime(value.toDateTime());
    }else if(role == NoteLastModificationDateTime){
        note.setLastModificationDateTime(value.toDateTime());
    }else if(role == NoteDeletionDateTime){
        note.setDeletionDateTime(value.toDateTime());
    }else if(role == NoteContent){
        note.setContent(value.toString());
    }else if(role == NoteScrollbarPos){
        note.setScrollBarPosition(value.toInt());
    }else if(role == NoteTagsList) {
        note.setTagIds(value.value<QSet<int>>());
    }else if(role == NoteIsTemp) {
        note.setIsTempNote(value.toBool());
    }else if(role == NodeParentName) {
        note.setParentName(value.toString());
    } else {
        return false;
    }

    emit dataChanged(this->index(index.row()),
                     this->index(index.row()),
                     QVector<int>(1,role));

    return true;
}

Qt::ItemFlags NoteListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsEditable ;
}

int NoteListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_noteList.count();
}

void NoteListModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column)
    Q_UNUSED(order)

    std::stable_sort(m_noteList.begin(), m_noteList.end(), [](const NodeData& lhs, const NodeData& rhs){
        return lhs.lastModificationdateTime() > rhs.lastModificationdateTime();
    });

    emit dataChanged(index(0), index(rowCount()-1));
}

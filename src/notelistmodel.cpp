#include "notelistmodel.h"
#include <QDebug>
#include "nodepath.h"
#include <QTimer>
#include <QMimeData>

NoteListModel::NoteListModel(QObject *parent)
    : QAbstractListModel(parent),
      m_isPinnedList{false}
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
    emit rowCountChanged();
    return createIndex(rowCnt, 0);
}

QModelIndex NoteListModel::insertNote(const NodeData &note, int row)
{
    beginInsertRows(QModelIndex(), row, row);
    m_noteList.insert(row, note);
    endInsertRows();
    emit rowCountChanged();
    return createIndex(row, 0);
}

NodeData NoteListModel::getNote(const QModelIndex& index) const
{
    auto row = index.row();
    return m_noteList.at(row);
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

void NoteListModel::setListNote(const QVector<NodeData> notes, const ListViewInfo& inf)
{
    beginResetModel();
    m_listViewInfo = inf;
    m_noteList = notes;
    sort(0, Qt::AscendingOrder);
    endResetModel();
    emit rowCountChanged();
}

void NoteListModel::removeNote(const QModelIndex &noteIndex)
{
    if (noteIndex.isValid()) {
        int row = noteIndex.row();
        beginRemoveRows(QModelIndex(), row, row);
        m_noteList.takeAt(row);
        endRemoveRows();
        emit rowCountChanged();
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
        vector_move(m_noteList, sourceRow, destinationChild);
        endMoveRows();
        emit rowCountChanged();
        return true;
    }
    return false;
}

void NoteListModel::clearNotes()
{
    beginResetModel();
    m_noteList.clear();
    endResetModel();
    emit rowCountChanged();
}

QVariant NoteListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
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
    }else if(role == NoteParentName){
        return note.parentName();
    } else if (role == NoteTagListScrollbarPos) {
        return note.tagListScrollBarPos();
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
    }else if(role == NoteParentName) {
        note.setParentName(value.toString());
    } else if (role == NoteTagListScrollbarPos) {
        note.setTagListScrollBarPos(value.toInt());
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
    if (!index.isValid()) {
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
    }

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

int NoteListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_noteList.size();
}

void NoteListModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column)
    Q_UNUSED(order)
    if (m_isPinnedList) {
        std::stable_sort(m_noteList.begin(), m_noteList.end(), [this](const NodeData& lhs, const NodeData& rhs) {
            if (isInAllNote()) {
                return lhs.relativePosAN() < rhs.relativePosAN();
            } else {
                return lhs.relativePosition() < rhs.relativePosition();
            }
        });
    } else {
        std::stable_sort(m_noteList.begin(), m_noteList.end(), [](const NodeData& lhs, const NodeData& rhs){
            return lhs.lastModificationdateTime() > rhs.lastModificationdateTime();
        });
    }

    emit dataChanged(index(0), index(rowCount()-1));
}

void NoteListModel::setNoteData(const QModelIndex &index, const NodeData &note)
{
    if (!index.isValid()) {
        return;
    }

    m_noteList[index.row()] = note;
    emit dataChanged(this->index(index.row()),
                     this->index(index.row()));
}

void NoteListModel::setIsPinnedList(bool newIsPinnedList)
{
    m_isPinnedList = newIsPinnedList;
}

void NoteListModel::updatePinnedRelativePosition()
{
    if (m_isPinnedList) {
        for (int i = 0; i < m_noteList.size(); ++i) {
            if (!isInAllNote()) {
                emit requestUpdatePinnedRelPos(m_noteList[i].id(), i);
            } else {
                emit requestUpdatePinnedRelPosAN(m_noteList[i].id(), i);
            }
        }
    }
}

bool NoteListModel::isInAllNote() const
{
    return (!m_listViewInfo.isInTag) && (m_listViewInfo.parentFolderId == SpecialNodeID::RootFolder);
}

Qt::DropActions NoteListModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions NoteListModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

QStringList NoteListModel::mimeTypes() const
{
    return QStringList() << NOTE_MIME;
}

QMimeData *NoteListModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty()) {
        return nullptr;
    }
    QMimeData *mimeData = new QMimeData;
    const auto& current = indexes[0];
    mimeData->setData(NOTE_MIME, QString::number(
                          current.data(NoteListModel::NoteID).toInt()).toUtf8());
    return mimeData;
}

bool NoteListModel::dropMimeData(const QMimeData *mime,
                                 Qt::DropAction action,
                                 int row, int column,
                                 const QModelIndex &parent)
{
    Q_UNUSED(column);
    if (!m_isPinnedList) {
        return false;
    }
    if (!(mime->hasFormat(NOTE_MIME) &&
          action == Qt::MoveAction)) {
        return false;
    }
    if (row == -1) {
        // valid index: drop onto item
        if (parent.isValid()) {
            row = parent.row();
        } else {
            // invalid index: append at bottom, after last toplevel
            row = rowCount(parent);
        }
    }
    if (row >= m_noteList.size()) {
        return false;
    }
    bool ok = false;
    auto nodeId = QString::fromUtf8(mime->data(NOTE_MIME)).toInt(&ok);
    if (ok) {
        beginResetModel();
        for (int i = 0; i < m_noteList.size(); ++i) {
            if (m_noteList[i].id() == nodeId) {
                vector_move(m_noteList, i, row);
                break;
            }
        }

        endResetModel();
        emit setCurrentIndex(getNoteIndex(nodeId));
        updatePinnedRelativePosition();
        return true;
    }

    return false;
}

bool NoteListModel::noteIsHaveTag(const QModelIndex index) const
{
    if (index.row() < 0 || index.row() >= m_noteList.count()) {
        return false;
    }
    auto row = index.row();
    const auto& note = m_noteList[row];
    return !note.tagIds().empty();
}

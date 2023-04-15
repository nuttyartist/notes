#include "notelistmodel.h"
#include <QDebug>
#include "nodepath.h"
#include <QTimer>
#include <QMimeData>

NoteListModel::NoteListModel(QObject *parent) : QAbstractListModel(parent) { }

NoteListModel::~NoteListModel() { }

QModelIndex NoteListModel::addNote(const NodeData &note)
{
    if (!note.isPinnedNote()) {
        const int rowCnt = rowCount();
        beginInsertRows(QModelIndex(), rowCnt, rowCnt);
        m_noteList << note;
        endInsertRows();
        emit rowsInsertedC({ createIndex(rowCnt, 0) });
        emit rowCountChanged();
        return createIndex(rowCnt, 0);
    } else {
        const int rowCnt = m_pinnedList.size();
        beginInsertRows(QModelIndex(), rowCnt, rowCnt);
        m_pinnedList << note;
        endInsertRows();
        emit rowsInsertedC({ createIndex(rowCnt, 0) });
        emit rowCountChanged();
        return createIndex(rowCnt, 0);
    }
}

QModelIndex NoteListModel::insertNote(const NodeData &note, int row)
{
    if (note.isPinnedNote()) {
        if (row > m_pinnedList.size()) {
            row = m_pinnedList.size();
        } else if (row < 0) {
            row = 0;
        }
        beginInsertRows(QModelIndex(), row, row);
        m_pinnedList.insert(row, note);
        endInsertRows();
        emit rowsInsertedC({ createIndex(row, 0) });
        emit rowCountChanged();
        return createIndex(row, 0);
    } else {
        if (row < m_pinnedList.size()) {
            row = m_pinnedList.size();
        } else if (row > (m_pinnedList.size() + m_noteList.size())) {
            row = m_pinnedList.size() + m_noteList.size();
        }
        beginInsertRows(QModelIndex(), row, row);
        m_noteList.insert(row - m_pinnedList.size(), note);
        endInsertRows();
        emit rowsInsertedC({ createIndex(row, 0) });
        emit rowCountChanged();
        return createIndex(row, 0);
    }
}

const NodeData &NoteListModel::getNote(const QModelIndex &index) const
{
    auto row = index.row();
    if (row < m_pinnedList.size()) {
        return m_pinnedList.at(row);
    } else {
        row = row - m_pinnedList.size();
        return m_noteList.at(row);
    }
}

QModelIndex NoteListModel::getNoteIndex(int id) const
{
    for (int i = 0; i < m_pinnedList.size(); ++i) {
        if (m_pinnedList[i].id() == id) {
            return createIndex(i, 0);
        }
    }

    for (int i = 0; i < m_noteList.size(); ++i) {
        if (m_noteList[i].id() == id) {
            return createIndex(i + m_pinnedList.size(), 0);
        }
    }
    return QModelIndex{};
}

void NoteListModel::setListNote(const QVector<NodeData> &notes, const ListViewInfo &inf)
{
    beginResetModel();
    m_pinnedList.clear();
    m_noteList.clear();
    m_listViewInfo = inf;
    if ((!m_listViewInfo.isInTag)
        && (m_listViewInfo.parentFolderId != SpecialNodeID::TrashFolder)) {
        for (const auto &note : qAsConst(notes)) {
            if (note.isPinnedNote()) {
                m_pinnedList.append(note);
            } else {
                m_noteList.append(note);
            }
        }
    } else {
        m_noteList = notes;
    }
    sort(0, Qt::AscendingOrder);
    endResetModel();
    emit rowCountChanged();
}

void NoteListModel::removeNotes(const QModelIndexList &noteIndexes)
{
    emit requestRemoveNotes(noteIndexes);
}

bool NoteListModel::moveRow(const QModelIndex &sourceParent, int sourceRow,
                            const QModelIndex &destinationParent, int destinationChild)
{
    if (sourceRow < 0 || sourceRow >= rowCount() || destinationChild < 0
        || destinationChild >= rowCount()) {
        return false;
    }
    if (sourceRow < m_pinnedList.size() && destinationChild < m_pinnedList.size()) {
        if (beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent,
                          destinationChild)) {
            m_pinnedList.move(sourceRow, destinationChild);
            endMoveRows();
            emit rowsAboutToBeMovedC({ createIndex(sourceRow, 0) });
            emit rowsMovedC({ createIndex(destinationChild, 0) });
            emit rowCountChanged();
            return true;
        }
    }
    if (sourceRow >= m_pinnedList.size() && destinationChild >= m_pinnedList.size()) {
        sourceRow = sourceRow - m_pinnedList.size();
        destinationChild = destinationChild - m_pinnedList.size();
        if (beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent,
                          destinationChild)) {
            m_noteList.move(sourceRow, destinationChild);
            endMoveRows();
            emit rowsAboutToBeMovedC({ createIndex(sourceRow, 0) });
            emit rowsMovedC({ createIndex(destinationChild + 1, 0) });
            emit rowCountChanged();
            return true;
        }
    }
    return false;
}

void NoteListModel::clearNotes()
{
    beginResetModel();
    m_pinnedList.clear();
    m_noteList.clear();
    endResetModel();
    emit rowCountChanged();
}

QVariant NoteListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= (m_noteList.count() + m_pinnedList.count())) {
        return QVariant();
    }
    if (role < NoteID || role > NoteIsPinned) {
        return QVariant();
    }
    const NodeData &note = getRef(index.row());
    if (role == NoteID) {
        return note.id();
    } else if (role == NoteFullTitle) {
        auto text = note.fullTitle().trimmed();
        if (text.startsWith("#")) {
            text.remove(0, 1);
            text = text.trimmed();
        }
        return text;
    } else if (role == NoteCreationDateTime) {
        return note.creationDateTime();
    } else if (role == NoteLastModificationDateTime) {
        return note.lastModificationdateTime();
    } else if (role == NoteDeletionDateTime) {
        return note.deletionDateTime();
    } else if (role == NoteContent) {
        return note.content();
    } else if (role == NoteScrollbarPos) {
        return note.scrollBarPosition();
    } else if (role == NoteTagsList) {
        return QVariant::fromValue(note.tagIds());
    } else if (role == NoteIsTemp) {
        return note.isTempNote();
    } else if (role == NoteParentName) {
        return note.parentName();
    } else if (role == NoteTagListScrollbarPos) {
        return note.tagListScrollBarPos();
    } else if (role == NoteIsPinned) {
        return note.isPinnedNote();
    }

    return QVariant();
}

bool NoteListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() < 0 || index.row() >= (m_noteList.count() + m_pinnedList.count())) {
        return false;
    }

    NodeData &note = getRef(index.row());
    if (role == NoteID) {
        note.setId(value.toInt());
    } else if (role == NoteFullTitle) {
        note.setFullTitle(value.toString());
    } else if (role == NoteCreationDateTime) {
        note.setCreationDateTime(value.toDateTime());
    } else if (role == NoteLastModificationDateTime) {
        note.setLastModificationDateTime(value.toDateTime());
    } else if (role == NoteDeletionDateTime) {
        note.setDeletionDateTime(value.toDateTime());
    } else if (role == NoteContent) {
        note.setContent(value.toString());
    } else if (role == NoteScrollbarPos) {
        note.setScrollBarPosition(value.toInt());
    } else if (role == NoteTagsList) {
        note.setTagIds(value.value<QSet<int>>());
    } else if (role == NoteIsTemp) {
        note.setIsTempNote(value.toBool());
    } else if (role == NoteParentName) {
        note.setParentName(value.toString());
    } else if (role == NoteTagListScrollbarPos) {
        note.setTagListScrollBarPos(value.toInt());
    } else {
        return false;
    }

    emit dataChanged(this->index(index.row()), this->index(index.row()), QVector<int>(1, role));
    return true;
}

Qt::ItemFlags NoteListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
    }

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled
            | Qt::ItemIsDropEnabled;
}

int NoteListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_noteList.size() + m_pinnedList.size();
}

void NoteListModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column)
    Q_UNUSED(order)
    if (m_listViewInfo.parentFolderId == SpecialNodeID::TrashFolder) {
        std::stable_sort(m_noteList.begin(), m_noteList.end(),
                         [](const NodeData &lhs, const NodeData &rhs) {
                             return lhs.deletionDateTime() > rhs.deletionDateTime();
                         });
    } else {
        std::stable_sort(m_pinnedList.begin(), m_pinnedList.end(),
                         [this](const NodeData &lhs, const NodeData &rhs) {
                             if (isInAllNote()) {
                                 return lhs.relativePosAN() < rhs.relativePosAN();
                             } else {
                                 return lhs.relativePosition() < rhs.relativePosition();
                             }
                         });

        std::stable_sort(m_noteList.begin(), m_noteList.end(),
                         [](const NodeData &lhs, const NodeData &rhs) {
                             return lhs.lastModificationdateTime() > rhs.lastModificationdateTime();
                         });
    }

    emit dataChanged(index(0), index(rowCount() - 1));
}

void NoteListModel::setNoteData(const QModelIndex &index, const NodeData &note)
{
    if (!index.isValid()) {
        return;
    }
    auto row = index.row();
    if (row < m_pinnedList.size()) {
        m_pinnedList[row] = note;
    } else {
        row = row - m_pinnedList.size();
        m_noteList[row] = note;
    }
    emit dataChanged(this->index(index.row()), this->index(index.row()));
}

void NoteListModel::updatePinnedRelativePosition()
{
    for (int i = 0; i < m_pinnedList.size(); ++i) {
        if (!isInAllNote()) {
            emit requestUpdatePinnedRelPos(m_pinnedList[i].id(), i);
        } else {
            emit requestUpdatePinnedRelPosAN(m_pinnedList[i].id(), i);
        }
    }
}

bool NoteListModel::isInAllNote() const
{
    return (!m_listViewInfo.isInTag)
            && (m_listViewInfo.parentFolderId == SpecialNodeID::RootFolder);
}

NodeData &NoteListModel::getRef(int row)
{
    if (row < m_pinnedList.size())
        return m_pinnedList[row];
    return m_noteList[row - m_pinnedList.size()];
}

const NodeData &NoteListModel::getRef(int row) const
{
    if (row < m_pinnedList.size())
        return m_pinnedList[row];
    return m_noteList[row - m_pinnedList.size()];
}

bool NoteListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || (row + count) > (m_pinnedList.size() + m_noteList.size())) {
        return false;
    }
    beginRemoveRows(parent, row, row + count - 1);
    for (int r = row; r < row + count; ++r) {
        if (r < m_pinnedList.size()) {
            m_pinnedList.takeAt(r);
        } else {
            auto rr = r - m_pinnedList.size();
            m_noteList.takeAt(rr);
        }
    }
    endRemoveRows();
    emit rowCountChanged();
    return true;
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
    QStringList d;
    for (const auto &index : indexes) {
        auto id = index.data(NoteListModel::NoteID).toInt();
        d.append(QString::number(id));
    }
    if (d.isEmpty()) {
        return nullptr;
    }
    QMimeData *mimeData = new QMimeData;
    mimeData->setData(NOTE_MIME, d.join(QStringLiteral(PATH_SEPARATOR)).toUtf8());
    return mimeData;
}

bool NoteListModel::dropMimeData(const QMimeData *mime, Qt::DropAction action, int row, int column,
                                 const QModelIndex &parent)
{
    Q_UNUSED(column);

    if (!(mime->hasFormat(NOTE_MIME) && action == Qt::MoveAction)) {
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
    bool toPinned = false;
    if (row >= m_pinnedList.size()) {
        toPinned = false;
    } else {
        toPinned = true;
    }
    auto idl = QString::fromUtf8(mime->data(NOTE_MIME)).split(QStringLiteral(PATH_SEPARATOR));
    QSet<int> movedIds;
    QModelIndexList idxe;
    for (const auto &id_s : qAsConst(idl)) {
        auto nodeId = id_s.toInt();
        idxe.append(getNoteIndex(nodeId));
    }
    emit rowsAboutToBeMovedC(idxe);
    beginResetModel();
    if (toPinned) {
        for (const auto &index : qAsConst(idxe)) {
            auto &note = getRef(index.row());
            if (!note.isPinnedNote()) {
                note.setIsPinnedNote(true);
                emit requestUpdatePinned(note.id(), true);
                m_pinnedList.prepend(m_noteList.takeAt(index.row() - m_pinnedList.size()));
            }
        }
        for (const auto &id_s : qAsConst(idl)) {
            auto nodeId = id_s.toInt();
            for (int i = 0; i < m_pinnedList.size(); ++i) {
                if (m_pinnedList[i].id() == nodeId) {
                    m_pinnedList.move(i, row);
                    break;
                }
            }
            movedIds.insert(nodeId);
        }
    } else {
        for (const auto &index : qAsConst(idxe)) {
            auto &note = getRef(index.row());
            movedIds.insert(note.id());
            if (!note.isPinnedNote()) {
                continue;
            }
            note.setIsPinnedNote(false);
            emit requestUpdatePinned(note.id(), false);
            int destinationChild = 0;
            if (m_listViewInfo.parentFolderId == SpecialNodeID::TrashFolder) {
                auto lastMod = index.data(NoteDeletionDateTime).toDateTime();
                for (destinationChild = 0; destinationChild < m_noteList.size();
                     ++destinationChild) {
                    if (m_noteList[destinationChild].deletionDateTime() <= lastMod) {
                        break;
                    }
                }
            } else {
                auto lastMod = index.data(NoteLastModificationDateTime).toDateTime();
                for (destinationChild = 0; destinationChild < m_noteList.size();
                     ++destinationChild) {
                    if (m_noteList[destinationChild].deletionDateTime() <= lastMod) {
                        break;
                    }
                }
            }
            m_noteList.insert(destinationChild, m_pinnedList.takeAt(index.row()));
        }
    }

    endResetModel();
    QModelIndexList destinations;
    for (const auto &id : movedIds) {
        auto index = getNoteIndex(id);
        if (!index.isValid()) {
            continue;
        }
        destinations.append(index);
    }
    emit selectNotes(destinations);
    emit rowsMovedC(destinations);
    updatePinnedRelativePosition();
    return true;
}

QModelIndex NoteListModel::getFirstUnpinnedNote() const
{
    if (!m_noteList.isEmpty()) {
        return createIndex(m_pinnedList.size(), 0);
    } else {
        return QModelIndex();
    }
}

bool NoteListModel::hasPinnedNote() const
{
    if ((!m_listViewInfo.isInTag)
        && (m_listViewInfo.parentFolderId == SpecialNodeID::TrashFolder)) {
        // Trash don't have pinned note
        return false;
    }
    return !m_pinnedList.isEmpty();
}

void NoteListModel::setNotesIsPinned(const QModelIndexList &indexes, bool isPinned)
{
    emit requestCloseNoteEditor(indexes);
    QSet<int> needMovingIds;
    QModelIndexList needMovingIndexes;
    for (const auto &index : indexes) {
        if (index.isValid()) {
            NodeData &note = getRef(index.row());
            if (note.isPinnedNote() != isPinned) {
                needMovingIds.insert(note.id());
                needMovingIndexes.append(index);
                note.setIsPinnedNote(isPinned);
                emit requestUpdatePinned(note.id(), isPinned);
            }
        }
    }

    if (isPinned) {
        emit rowsAboutToBeMovedC(needMovingIndexes);
        beginResetModel();
        for (const auto &id : qAsConst(needMovingIds)) {
            auto index = getNoteIndex(id);
            if (!index.isValid()) {
                continue;
            }
            int sourceRow = index.row();
            if (sourceRow < 0 || sourceRow >= rowCount()) {
                continue;
            }
            m_pinnedList.prepend(m_noteList.takeAt(sourceRow - m_pinnedList.size()));
        }
        endResetModel();
        QModelIndexList destinations;
        for (const auto &id : needMovingIds) {
            auto index = getNoteIndex(id);
            if (!index.isValid()) {
                continue;
            }
            destinations.append(index);
        }
        emit selectNotes(destinations);
        emit rowsMovedC(destinations);
        emit rowCountChanged();
        updatePinnedRelativePosition();
    } else {
        emit rowsAboutToBeMovedC(needMovingIndexes);
        beginResetModel();
        for (const auto &id : qAsConst(needMovingIds)) {
            auto index = getNoteIndex(id);
            if (!index.isValid()) {
                continue;
            }
            int destinationChild = 0;
            if (m_listViewInfo.parentFolderId == SpecialNodeID::TrashFolder) {
                auto lastMod = index.data(NoteDeletionDateTime).toDateTime();
                for (destinationChild = 0; destinationChild < m_noteList.size();
                     ++destinationChild) {
                    const auto &note = m_noteList[destinationChild];
                    if (note.deletionDateTime() <= lastMod) {
                        break;
                    }
                }
            } else {
                auto lastMod = index.data(NoteLastModificationDateTime).toDateTime();
                for (destinationChild = 0; destinationChild < m_noteList.size();
                     ++destinationChild) {
                    const auto &note = m_noteList[destinationChild];
                    if (note.lastModificationdateTime() <= lastMod) {
                        break;
                    }
                }
            }
            m_noteList.insert(destinationChild, m_pinnedList.takeAt(index.row()));
        }
        endResetModel();
        QModelIndexList destinations;
        for (const auto &id : needMovingIds) {
            auto index = getNoteIndex(id);
            if (!index.isValid()) {
                continue;
            }
            destinations.append(index);
        }
        emit selectNotes(destinations);
        emit rowsMovedC(destinations);
        emit rowCountChanged();
        updatePinnedRelativePosition();
    }
}

bool NoteListModel::noteIsHaveTag(const QModelIndex &index) const
{
    if (index.row() < 0 || index.row() >= (m_noteList.count() + m_pinnedList.count())) {
        return false;
    }
    auto row = index.row();
    NodeData note;
    if (row < m_pinnedList.size()) {
        note = m_pinnedList[row];
    } else {
        row = row - m_pinnedList.size();
        note = m_noteList[row];
    }
    return !note.tagIds().empty();
}

bool NoteListModel::isFirstPinnedNote(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return false;
    }
    if (index.row() > 0) {
        return false;
    }
    const NodeData &note = getRef(index.row());
    if (index.row() == 0 && note.isPinnedNote()) {
        return true;
    }
    return false;
}

bool NoteListModel::isFirstUnpinnedNote(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return false;
    }
    const NodeData &note = getRef(index.row());
    if ((index.row() - m_pinnedList.size()) == 0 && !note.isPinnedNote()) {
        return true;
    }
    return false;
}

QModelIndex NoteListModel::getFirstPinnedNote() const
{
    if (m_pinnedList.isEmpty()) {
        return QModelIndex();
    }
    return createIndex(0, 0);
}

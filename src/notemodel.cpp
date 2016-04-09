#include "notemodel.h"
#include <QDebug>

NoteModel::NoteModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

NoteModel::~NoteModel()
{

}

QModelIndex NoteModel::addNote(NoteData* note)
{
    const int rowCnt = rowCount();
    beginInsertRows(QModelIndex(), rowCnt, rowCnt);
    note->setParent(this);
    m_noteList << note;
    endInsertRows();

    return createIndex(rowCnt, 0);
}

QModelIndex NoteModel::insertNote(NoteData *note, int row)
{
    if(row >= rowCount()){
        return addNote(note);
    }else{
        beginInsertRows(QModelIndex(), row, row);
        m_noteList.insert(row, note);
        endInsertRows();
    }

    return createIndex(row,0);
}

void NoteModel::addListNote(QList<NoteData *> noteList)
{
    int start = rowCount();
    int end = start + noteList.count()-1;
    beginInsertRows(QModelIndex(), start, end);
    m_noteList << noteList;
    endInsertRows();
}

void NoteModel::removeNote(const QModelIndex &noteIndex)
{
    int row = noteIndex.row();
    beginRemoveRows(QModelIndex(), row, row);
    m_noteList.removeAt(row);
    endRemoveRows();

}

bool NoteModel::moveRow(const QModelIndex &sourceParent, int sourceRow, const QModelIndex &destinationParent, int destinationChild)
{
    if(sourceRow<0
            || sourceRow >= m_noteList.count()
            || destinationChild <0
            || destinationChild >= m_noteList.count()){

        return false;
    }

    beginMoveRows(sourceParent,sourceRow,sourceRow,destinationParent,destinationChild);
    m_noteList.move(sourceRow,destinationChild);
    endMoveRows();

    return true;
}

void NoteModel::clearNotes()
{
    beginResetModel();
    m_noteList.clear();
    endResetModel();
}

QVariant NoteModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_noteList.count())
        return QVariant();

    NoteData* note = m_noteList[index.row()];
    if(role == NoteID){
        return note->id();
    }else if(role == NoteFullTitle){
        return note->fullTitle();
    }else if(role == NoteDateTime){
        return note->dateTime();
    }else if(role == NoteContent){
        return note->content();
    }else if(role == NoteScrollbarPos){
        return note->scrollBarPosition();
    }

    return QVariant();
}

bool NoteModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    NoteData* note = m_noteList[index.row()];


    if(role == NoteID){
        note->setId(value.toString());
    }else if(role == NoteFullTitle){
        note->setFullTitle(value.toString());
    }else if(role == NoteDateTime){
        note->setDateTime(value.toDateTime());
    }else if(role == NoteContent){
        note->setContent(value.toString());
    }else if(role == NoteScrollbarPos){
        note->setScrollBarPosition(value.toInt());
    }else{
        return false;
    }

    emit dataChanged(this->index(index.row()),
                     this->index(index.row()),
                     QVector<int>(1,role));

    return true;
}

Qt::ItemFlags NoteModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsEditable ;
}

int NoteModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_noteList.count();
}

void NoteModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column)
    Q_UNUSED(order)

    std::stable_sort(m_noteList.begin(), m_noteList.end(), [](NoteData* lhs, NoteData* rhs){
        return lhs->dateTime() > rhs->dateTime();
    });

    emit dataChanged(index(0), index(rowCount()-1));
}

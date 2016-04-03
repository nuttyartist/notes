#include "notemodel.h"
#include <QDebug>

NoteModel::NoteModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

NoteModel::~NoteModel()
{

}

void NoteModel::addNote(NoteData* note)
{
    const int rowCnt = rowCount();
    beginInsertRows(QModelIndex(), rowCnt, rowCnt);
    note->setParent(this);
    m_noteList << note;
    endInsertRows();
}

void NoteModel::insertNote(NoteData *note, int row)
{
    if(row >= rowCount()){
        addNote(note);
    }else{
        beginInsertRows(QModelIndex(), row, row);
        note->setParent(this);
        m_noteList.insert(row, note);
        endInsertRows();
    }
}

void NoteModel::removeNote(NoteData *note)
{
    int row = m_noteList.indexOf(note);
    beginRemoveRows(QModelIndex(), row, row);
    m_noteList.removeOne(note);
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

void NoteModel::copyFromModel(NoteModel *model)
{
    beginInsertRows(QModelIndex(),0,model->rowCount()-1);
    for(int i=0; i<model->rowCount(); i++){
        QModelIndex index = model->index(i,0);
        NoteData* noteToAdd = model->getNote(index);
        m_noteList.push_back(noteToAdd);
    }
    endInsertRows();
}

NoteData *NoteModel::getNote(const QModelIndex &index) const
{
    return m_noteList.at(index.row());
}

QModelIndex NoteModel::getNoteIndex(NoteData *note) const
{
    int row = m_noteList.indexOf(note);
    return createIndex(row, 0);
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

QHash<int, QByteArray> NoteModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NoteID] = "id";
    roles[NoteFullTitle] = "fullTitle";
    roles[NoteDateTime] = "dateTime";
    roles[NoteContent] = "content";

    return roles;
}

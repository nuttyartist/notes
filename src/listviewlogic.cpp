#include "listviewlogic.h"
#include "notelistview.h"
#include "notelistmodel.h"
#include "notelistdelegate.h"
#include "dbmanager.h"
#include <QDebug>

ListViewLogic::ListViewLogic(NoteListView* noteView,
                             NoteListModel* noteModel,
                             DBManager* dbManager,
                             QObject* parent) : QObject(parent),
    m_listView{noteView},
    m_listModel{noteModel},
    m_dbManager{dbManager}
{
    m_listView->setItemDelegate(new NoteListDelegate(m_listView));
    connect(m_dbManager, &DBManager::notesListReceived, this, &ListViewLogic::loadNoteListModel);
    // note model rows moved
    connect(m_listModel, &NoteListModel::rowsAboutToBeMoved, m_listView, &NoteListView::rowsAboutToBeMoved);
    connect(m_listModel, &NoteListModel::rowsMoved, m_listView, &NoteListView::rowsMoved);
}

void ListViewLogic::moveNoteToTop(const NodeData &note)
{
    QModelIndex noteIndex = m_listModel->getNoteIndex(note.id());
    if (noteIndex.isValid()){
        m_listView->scrollToTop();

        // move the current selected note to the top
        QModelIndex destinationIndex = m_listModel->index(0);
        m_listModel->moveRow(noteIndex, noteIndex.row(), destinationIndex, 0);

        // update the current item
        noteIndex = destinationIndex;
        m_listView->setCurrentIndex(noteIndex);
    } else {
        qDebug() << "ListViewLogic::moveNoteToTop : Note is not in list";
    }
}

void ListViewLogic::setNoteData(const NodeData &note)
{
    QModelIndex noteIndex = m_listModel->getNoteIndex(note.id());
    if (noteIndex.isValid()) {
        QMap<int, QVariant> dataValue;
        dataValue[NoteListModel::NoteContent] =  QVariant::fromValue(note.content());
        dataValue[NoteListModel::NoteFullTitle] =  QVariant::fromValue(note.fullTitle());
        dataValue[NoteListModel::NoteLastModificationDateTime] =
                QVariant::fromValue(note.lastModificationdateTime());

        m_listModel->setItemData(noteIndex, dataValue);
    } else {
        qDebug() << "ListViewLogic::moveNoteToTop : Note is not in list";
    }
}

void ListViewLogic::loadNoteListModel(QVector<NodeData> noteList)
{
    m_listModel->setListNote(noteList);
    //    setTheme(m_currentTheme);
    selectFirstNote();
}

void ListViewLogic::selectFirstNote()
{
    if(m_listModel->rowCount() > 0){
        QModelIndex index = m_listModel->index(0,0);
        m_listView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        m_listView->setCurrentIndex(index);
        auto firstNote = m_listModel->getNote(index);
        emit showNoteInEditor(firstNote);
    }
}

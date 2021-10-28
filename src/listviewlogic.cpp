#include "listviewlogic.h"
#include "notelistview.h"
#include "notelistmodel.h"
#include "notelistdelegate.h"
#include "dbmanager.h"
#include <QDebug>

ListViewLogic::ListViewLogic(NoteListView* noteView,
                             NoteListModel* noteModel,
                             TagPool *tagPool,
                             DBManager* dbManager,
                             QObject* parent) : QObject(parent),
    m_listView{noteView},
    m_listModel{noteModel},
    m_dbManager{dbManager}
{
    m_listView->setItemDelegate(new NoteListDelegate(tagPool, m_listView));
    connect(m_dbManager, &DBManager::notesListReceived, this, &ListViewLogic::loadNoteListModel);
    // note model rows moved
    connect(m_listModel, &NoteListModel::rowsAboutToBeMoved, m_listView, &NoteListView::rowsAboutToBeMoved);
    connect(m_listModel, &NoteListModel::rowsMoved, m_listView, &NoteListView::rowsMoved);
    // note pressed
    connect(m_listView, &NoteListView::pressed, this, &ListViewLogic::onNotePressed);

    connect(m_listView, &NoteListView::addTagRequested, this, &ListViewLogic::onAddTagRequest);
    connect(this, &ListViewLogic::requestAddTagDb, dbManager, &DBManager::addNoteToTag, Qt::QueuedConnection);
}

void ListViewLogic::selectNote(const QModelIndex &noteIndex)
{
    if(noteIndex.isValid()){
        auto note = m_listModel->getNote(noteIndex);
        m_listView->selectionModel()->select(noteIndex, QItemSelectionModel::ClearAndSelect);
        m_listView->setCurrentIndex(noteIndex);
        m_listView->scrollTo(noteIndex);
        emit showNoteInEditor(note);
    } else {
        qDebug() << "MainWindow::selectNote() : noteIndex is not valid";
    }
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

void ListViewLogic::onNoteEditClosed(const NodeData &note)
{
    if (note.isTempNote()) {
        QModelIndex noteIndex = m_listModel->getNoteIndex(note.id());
        if (noteIndex.isValid()) {
            m_listModel->removeNote(noteIndex);
        }
    }
}

void ListViewLogic::loadNoteListModel(QVector<NodeData> noteList)
{
    m_listModel->setListNote(noteList);
    //    setTheme(m_currentTheme);
    selectFirstNote();
}

void ListViewLogic::onAddTagRequest(const QModelIndex &index, int tagId)
{
    if (index.isValid()) {
        auto noteId = index.data(NoteListModel::NoteID).toInt();
        emit requestAddTagDb(noteId, tagId);
        auto tagIds = index.data(NoteListModel::NoteTagsList).value<QSet<int>>();
        tagIds.insert(tagId);
        m_listModel->setData(index, QVariant::fromValue(tagIds), NoteListModel::NoteTagsList);
    } else {
        qDebug() << __FUNCTION__ << "index is not valid";
    }
}

/*!
 * \brief MainWindow::onNotePressed
 * When clicking on a note in the scrollArea:
 * Unhighlight the previous selected note
 * If selecting a note when temporery note exist, delete the temp note
 * Highlight the selected note
 * Load the selected note content into textedit
 * \param index
 */

void ListViewLogic::onNotePressed(const QModelIndex &index)
{
    QModelIndex indexInProxy = m_listModel->index(index.row(), 0);
    selectNote(indexInProxy);
    m_listView->setCurrentRowActive(false);
}

void ListViewLogic::selectFirstNote()
{
    if (m_listModel->rowCount() > 0){
        QModelIndex index = m_listModel->index(0,0);
        m_listView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        m_listView->setCurrentIndex(index);
        auto firstNote = m_listModel->getNote(index);
        emit showNoteInEditor(firstNote);
    } else {
        emit closeNoteEditor();
    }
}

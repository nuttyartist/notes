#include "listviewlogic.h"
#include "notelistview.h"
#include "notelistmodel.h"
#include "notelistdelegate.h"
#include "dbmanager.h"
#include <QDebug>
#include <QMessageBox>
#include <QLineEdit>
#include <QToolButton>
#include "tagpool.h"

ListViewLogic::ListViewLogic(NoteListView* noteView,
                             NoteListModel* noteModel, QLineEdit *searchEdit, QToolButton *clearButton,
                             TagPool *tagPool,
                             DBManager* dbManager,
                             QObject* parent) : QObject(parent),
    m_listView{noteView},
    m_listModel{noteModel},
    m_searchEdit{searchEdit},
    m_clearButton{clearButton},
    m_dbManager{dbManager}
{
    m_listDelegate = new NoteListDelegate(tagPool, m_listView);
    m_listView->setItemDelegate(m_listDelegate);
    connect(m_dbManager, &DBManager::notesListReceived, this, &ListViewLogic::loadNoteListModel);
    // note model rows moved
    connect(m_listModel, &NoteListModel::rowsAboutToBeMoved, m_listView, &NoteListView::rowsAboutToBeMoved);
    connect(m_listModel, &NoteListModel::rowsMoved, m_listView, &NoteListView::rowsMoved);
    // note pressed
    connect(m_listView, &NoteListView::pressed, this, &ListViewLogic::onNotePressed);
    connect(m_listView, &NoteListView::addTagRequested, this, &ListViewLogic::onAddTagRequest);
    connect(m_listView, &NoteListView::removeTagRequested, this, &ListViewLogic::onRemoveTagRequest);
    connect(m_listModel, &NoteListModel::rowsRemoved, this, [this] (const QModelIndex &, int , int) {
        selectNote(m_listView->currentIndex());
    });

    connect(this, &ListViewLogic::requestAddTagDb, dbManager, &DBManager::addNoteToTag, Qt::QueuedConnection);
    connect(this, &ListViewLogic::requestRemoveTagDb, dbManager, &DBManager::removeNoteFromTag, Qt::QueuedConnection);
    connect(this, &ListViewLogic::requestRemoveNoteDb, dbManager, &DBManager::removeNote, Qt::QueuedConnection);
    connect(this, &ListViewLogic::requestMoveNoteDb, dbManager, &DBManager::moveNode, Qt::QueuedConnection);
    connect(this, &ListViewLogic::requestSearchInDb, dbManager, &DBManager::searchForNotes, Qt::QueuedConnection);
    connect(this, &ListViewLogic::requestClearSearchDb, dbManager, &DBManager::clearSearch, Qt::QueuedConnection);
    connect(m_listView, &NoteListView::deleteNoteRequested, this, &ListViewLogic::deleteNoteRequestedI);
    connect(m_listView, &NoteListView::restoreNoteRequested, this, &ListViewLogic::restoreNoteRequestedI);

    connect(tagPool, &TagPool::dataUpdated, this, [this] (int) {
        if (m_listModel->rowCount() > 0) {
            emit m_listModel->dataChanged(m_listModel->index(0,0),
                                          m_listModel->index(m_listModel->rowCount() - 1,0));
        }
    });
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
        auto wasTemp = noteIndex.data(NoteListModel::NoteIsTemp).toBool();
        dataValue[NoteListModel::NoteContent] =  QVariant::fromValue(note.content());
        dataValue[NoteListModel::NoteFullTitle] =  QVariant::fromValue(note.fullTitle());
        dataValue[NoteListModel::NoteLastModificationDateTime] =
                QVariant::fromValue(note.lastModificationdateTime());
        dataValue[NoteListModel::NoteIsTemp] =
                QVariant::fromValue(note.isTempNote());
        m_listModel->setItemData(noteIndex, dataValue);
        if (wasTemp) {
            auto tagIds = noteIndex.data(NoteListModel::NoteTagsList).value<QSet<int>>();
            for (auto tagId : tagIds) {
                emit requestAddTagDb(note.id(), tagId);
            }
        }
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

void ListViewLogic::deleteNoteRequested(const NodeData &note)
{
    auto index = m_listModel->getNoteIndex(note.id());
    deleteNoteRequestedI(index);
}

void ListViewLogic::selectNoteUp()
{
    auto currentIndex = m_listView->currentIndex();
    if(currentIndex.isValid()) {
        int currentRow = currentIndex.row();
        QModelIndex aboveIndex = m_listView->model()->index(currentRow - 1, 0);
        if(aboveIndex.isValid()) {
            selectNote(aboveIndex);
            m_listView->setCurrentRowActive(false);
        }
        if (!m_searchEdit->text().isEmpty()) {
            m_searchEdit->setFocus();
        } else {
            m_listView->setFocus();
        }
    } else {
        selectFirstNote();
    }
}

void ListViewLogic::selectNoteDown()
{
    auto currentIndex = m_listView->currentIndex();
    if(currentIndex.isValid()) {
        int currentRow = currentIndex.row();
        QModelIndex belowIndex = m_listView->model()->index(currentRow + 1, 0);
        if(belowIndex.isValid()){
            selectNote(belowIndex);
            m_listView->setCurrentRowActive(false);
        }

        //if the searchEdit is not empty, set the focus to it
        if (!m_searchEdit->text().isEmpty()) {
            m_searchEdit->setFocus();
        } else {
            m_listView->setFocus();
        }
    } else {
        selectFirstNote();
    }
}

/*!
 * \brief ListViewLogic::onSearchEditTextChanged
 * When text on searchEdit change:
 * If there is a temp note "New Note" while searching, we delete it
 * Saving the last selected note for recovery after searching
 * Clear all the notes from scrollArea and
 * If text is empty, reload all the notes from database
 * Else, load all the notes contain the string in searchEdit from database
 * \param keyword
 */

void ListViewLogic::onSearchEditTextChanged(const QString &keyword)
{
    if (keyword.isEmpty()) {
        clearSearch();
    } else {
        if (!m_listViewInfo.isInSearch) {
            auto index = m_listView->currentIndex();
            if (index.isValid()) {
                m_listViewInfo.currentNoteId = index.data(NoteListModel::NoteID).toInt();
            } else {
                m_listViewInfo.currentNoteId = SpecialNodeID::InvalidNoteId;
            }
        }
        m_clearButton->show();
        emit requestSearchInDb(keyword, m_listViewInfo);
    }
}

void ListViewLogic::clearSearch()
{
    emit requestClearSearchDb(m_listViewInfo);
    emit requestClearSearchUI();
}

void ListViewLogic::loadNoteListModel(const QVector<NodeData>& noteList, const ListViewInfo& inf)
{
    m_listModel->setListNote(noteList);
    auto currentNoteId = m_listViewInfo.currentNoteId;
    m_listViewInfo = inf;
    if ((!m_listViewInfo.isInTag) && m_listViewInfo.parentFolderId == SpecialNodeID::RootFolder) {
        m_listDelegate->setIsInAllNotes(true);
    } else {
        m_listDelegate->setIsInAllNotes(false);
    }
    if ((!m_listViewInfo.isInTag) && m_listViewInfo.parentFolderId == SpecialNodeID::TrashFolder) {
        m_listView->setIsInTrash(true);
    } else {
        m_listView->setIsInTrash(false);
    }

    //    setTheme(m_currentTheme);
    if (!m_listViewInfo.isInSearch && currentNoteId != SpecialNodeID::InvalidNoteId) {
        auto index = m_listModel->getNoteIndex(currentNoteId);
        if (index.isValid()) {
            m_listView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
            m_listView->setCurrentIndex(index);
            auto firstNote = m_listModel->getNote(index);
            emit showNoteInEditor(firstNote);
            return;
        }
    }
    selectFirstNote();
}

void ListViewLogic::onAddTagRequest(const QModelIndex &index, int tagId)
{
    if (index.isValid()) {
        auto noteId = index.data(NoteListModel::NoteID).toInt();
        auto isTemp = index.data(NoteListModel::NoteIsTemp).toBool();
        if (!isTemp) {
            emit requestAddTagDb(noteId, tagId);
        }
        auto tagIds = index.data(NoteListModel::NoteTagsList).value<QSet<int>>();
        tagIds.insert(tagId);
        m_listModel->setData(index, QVariant::fromValue(tagIds), NoteListModel::NoteTagsList);
        emit noteTagListChanged(noteId, tagIds);
    } else {
        qDebug() << __FUNCTION__ << "index is not valid";
    }
}

void ListViewLogic::onRemoveTagRequest(const QModelIndex &index, int tagId)
{
    if (index.isValid()) {
        auto noteId = index.data(NoteListModel::NoteID).toInt();
        auto isTemp = index.data(NoteListModel::NoteIsTemp).toBool();
        if (!isTemp) {
            emit requestRemoveTagDb(noteId, tagId);
        }
        auto tagIds = index.data(NoteListModel::NoteTagsList).value<QSet<int>>();
        tagIds.remove(tagId);
        m_listModel->setData(index, QVariant::fromValue(tagIds), NoteListModel::NoteTagsList);
        emit noteTagListChanged(noteId, tagIds);
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
    selectNote(index);
    m_listView->setCurrentRowActive(false);
}

void ListViewLogic::deleteNoteRequestedI(const QModelIndex &index)
{
    if (index.isValid()) {
        auto id = index.data(NoteListModel::NoteID).toInt();
        NodeData note;
        QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(NodeData, note),
                                  Q_ARG(int, id)
                                  );
        if (note.parentId() == SpecialNodeID::TrashFolder) {
            auto btn = QMessageBox::question(nullptr, "Are you sure you want to delete this note permanently",
                                             "Are you sure you want to delete this note permanently? It will not be recoverable.");
            if (btn == QMessageBox::Yes) {
                m_listModel->removeNote(index);
                emit requestRemoveNoteDb(note);
            }
        } else {
            m_listModel->removeNote(index);
            emit requestRemoveNoteDb(note);
        }
    }
}

void ListViewLogic::restoreNoteRequestedI(const QModelIndex &index)
{
    if (index.isValid()) {
        auto id = index.data(NoteListModel::NoteID).toInt();
        NodeData note;
        QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(NodeData, note),
                                  Q_ARG(int, id)
                                  );
        if (note.parentId() == SpecialNodeID::TrashFolder) {
            m_listModel->removeNote(index);
            NodeData defaultNotesFolder;
            QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                      Q_RETURN_ARG(NodeData, defaultNotesFolder),
                                      Q_ARG(int, SpecialNodeID::DefaultNotesFolder)
                                      );
            emit requestMoveNoteDb(note.id(), defaultNotesFolder);
        } else {
            qDebug() << "Note id" << id << "is currently not in Trash";
        }
    }
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

const ListViewInfo &ListViewLogic::listViewInfo() const
{
    return m_listViewInfo;
}

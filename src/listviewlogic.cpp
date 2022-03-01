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
#include <QTimer>

ListViewLogic::ListViewLogic(NoteListView* noteView,
                             NoteListModel* noteModel,
                             NoteListView *pinnedNoteView,
                             NoteListModel *pinnedNoteModel,
                             QLineEdit *searchEdit,
                             QToolButton *clearButton,
                             TagPool *tagPool,
                             DBManager* dbManager,
                             QObject* parent) : QObject(parent),
    m_listView{noteView},
    m_listModel{noteModel},
    m_pinnedNoteView{pinnedNoteView},
    m_pinnedNoteModel{pinnedNoteModel},
    m_searchEdit{searchEdit},
    m_clearButton{clearButton},
    m_dbManager{dbManager},
    m_tagPool{tagPool},
    m_needLoadSavedState{0},
    m_lastSelectedNote{SpecialNodeID::InvalidNodeId}
{
    m_noteListDelegate = new NoteListDelegate(m_listView, tagPool, m_listView);
    m_pinnedNoteListDelegate = new NoteListDelegate(m_pinnedNoteView, tagPool, m_listView);
    m_listView->setItemDelegate(m_noteListDelegate);
    m_listView->setDbManager(m_dbManager);
    m_pinnedNoteView->setItemDelegate(m_pinnedNoteListDelegate);
    m_pinnedNoteView->setDbManager(m_dbManager);
    m_pinnedNoteModel->setIsPinnedList(true);
    m_pinnedNoteView->setIsPinnedList(true);
    connect(m_dbManager, &DBManager::notesListReceived, this, &ListViewLogic::loadNoteListModel);
    // note model rows moved
    connect(m_listModel, &NoteListModel::rowsAboutToBeMoved, m_listView, &NoteListView::rowsAboutToBeMoved);
    connect(m_listModel, &NoteListModel::rowsMoved, m_listView, &NoteListView::rowsMoved);
    connect(m_pinnedNoteModel, &NoteListModel::rowsAboutToBeMoved, m_pinnedNoteView, &NoteListView::rowsAboutToBeMoved);
    connect(m_pinnedNoteModel, &NoteListModel::rowsMoved, m_pinnedNoteView, &NoteListView::rowsMoved);
    // note pressed
    connect(m_listView, &NoteListView::pressed, this, [this] (const QModelIndex &index) {
        onNotePressed(*m_listView, index);
    });
    connect(m_pinnedNoteView, &NoteListView::pressed, this, [this] (const QModelIndex &index) {
        onNotePressed(*m_pinnedNoteView, index);
    });

    connect(m_listView, &NoteListView::addTagRequested, this, [this] (const QModelIndex &index, int tagId) {
        onAddTagRequest(*m_listView, index, tagId);
    });
    connect(m_pinnedNoteView, &NoteListView::addTagRequested, this, [this] (const QModelIndex &index, int tagId) {
        onAddTagRequest(*m_pinnedNoteView, index, tagId);
    });

    connect(m_listView, &NoteListView::removeTagRequested, this, [this] (const QModelIndex &index, int tagId) {
        onRemoveTagRequest(*m_listView, index, tagId);
    });
    connect(m_pinnedNoteView, &NoteListView::removeTagRequested, this, [this] (const QModelIndex &index, int tagId) {
        onRemoveTagRequest(*m_pinnedNoteView, index, tagId);
    });

    connect(m_listModel, &NoteListModel::rowsRemoved, this, [this] (const QModelIndex &, int , int) {
        selectNote(*m_listView, m_listView->currentIndex());
    });
    connect(m_pinnedNoteModel, &NoteListModel::rowsRemoved, this, [this] (const QModelIndex &, int , int) {
        selectNote(*m_pinnedNoteView, m_pinnedNoteView->currentIndex());
    });

    connect(this, &ListViewLogic::requestAddTagDb, dbManager, &DBManager::addNoteToTag, Qt::QueuedConnection);
    connect(this, &ListViewLogic::requestRemoveTagDb, dbManager, &DBManager::removeNoteFromTag, Qt::QueuedConnection);
    connect(this, &ListViewLogic::requestRemoveNoteDb, dbManager, &DBManager::removeNote, Qt::QueuedConnection);
    connect(this, &ListViewLogic::requestMoveNoteDb, dbManager, &DBManager::moveNode, Qt::QueuedConnection);
    connect(this, &ListViewLogic::requestSearchInDb, dbManager, &DBManager::searchForNotes, Qt::QueuedConnection);
    connect(this, &ListViewLogic::requestClearSearchDb, dbManager, &DBManager::clearSearch, Qt::QueuedConnection);

    connect(m_pinnedNoteModel, &NoteListModel::requestUpdatePinnedRelPos,
            dbManager, &DBManager::updateRelPosPinnedNote, Qt::QueuedConnection);
    connect(m_pinnedNoteModel, &NoteListModel::requestUpdatePinnedRelPosAN,
            dbManager, &DBManager::updateRelPosPinnedNoteAN, Qt::QueuedConnection);
    connect(this, &ListViewLogic::requestUpdatePinnedDb,
            dbManager, &DBManager::setNoteIsPinned, Qt::QueuedConnection);

    connect(m_listView, &NoteListView::deleteNoteRequested, this, [this] (const QModelIndex &index) {
        deleteNoteRequestedI(*m_listView, index);
    });
    connect(m_pinnedNoteView, &NoteListView::deleteNoteRequested, this, [this] (const QModelIndex &index) {
        deleteNoteRequestedI(*m_pinnedNoteView, index);
    });
    connect(m_listView, &NoteListView::restoreNoteRequested, this, [this] (const QModelIndex &index) {
        restoreNoteRequestedI(*m_listView, index);
    });
    connect(m_pinnedNoteView, &NoteListView::restoreNoteRequested, this, [this] (const QModelIndex &index) {
        restoreNoteRequestedI(*m_pinnedNoteView, index);
    });

    connect(tagPool, &TagPool::dataUpdated, this, [this] (int) {
        if (m_pinnedNoteModel->rowCount() > 0) {
            emit m_pinnedNoteModel->dataChanged(m_pinnedNoteModel->index(0,0),
                                                m_pinnedNoteModel->index(m_pinnedNoteModel->rowCount() - 1,0));
            emit m_pinnedNoteModel->rowCountChanged();
        }
        if (m_listModel->rowCount() > 0) {
            emit m_listModel->dataChanged(m_listModel->index(0,0),
                                          m_listModel->index(m_listModel->rowCount() - 1,0));
            emit m_listModel->rowCountChanged();
        }
    });
    connect(m_listModel, &QAbstractItemModel::rowsInserted,
            this, &ListViewLogic::updateListViewLabel);
    connect(m_pinnedNoteModel, &QAbstractItemModel::rowsInserted,
            this, &ListViewLogic::updateListViewLabel);
    connect(m_listModel, &QAbstractItemModel::rowsRemoved,
            this, &ListViewLogic::updateListViewLabel);
    connect(m_pinnedNoteModel, &QAbstractItemModel::rowsRemoved,
            this, &ListViewLogic::updateListViewLabel);
    connect(m_listView, &NoteListView::newNoteRequested,
            this, &ListViewLogic::requestNewNote);
    connect(m_pinnedNoteView, &NoteListView::newNoteRequested,
            this, &ListViewLogic::requestNewNote);
    connect(m_listView, &NoteListView::moveNoteRequested,
            this, &ListViewLogic::moveNoteRequested);
    connect(m_pinnedNoteView, &NoteListView::moveNoteRequested,
            this, &ListViewLogic::moveNoteRequested);
    connect(m_listModel, &NoteListModel::rowCountChanged, this, [this] {
        onRowCountChanged(*m_listView);
    });
    connect(m_pinnedNoteModel, &NoteListModel::rowCountChanged, this, [this] {
        onRowCountChanged(*m_pinnedNoteView);
    });
    connect(m_listView, &NoteListView::doubleClicked,
            this, &ListViewLogic::onNoteDoubleClicked);
    connect(m_pinnedNoteView, &NoteListView::doubleClicked,
            this, &ListViewLogic::onNoteDoubleClicked);

    connect(m_listView, &NoteListView::setPinnedNoteRequested,
            this, &ListViewLogic::onSetPinnedNoteRequested);
    connect(m_pinnedNoteView, &NoteListView::setPinnedNoteRequested,
            this, &ListViewLogic::onSetPinnedNoteRequested);
    connect(m_pinnedNoteModel, &NoteListModel::setCurrentIndex,
            this, [this] (const QModelIndex& index) {
        m_pinnedNoteView->setCurrentIndex(index);
    });
}

void ListViewLogic::selectNote(NoteListView& listView, const QModelIndex &noteIndex)
{
    if(noteIndex.isValid()) {
        auto model = dynamic_cast<NoteListModel*>(listView.model());
        if (model) {
            auto note = model->getNote(noteIndex);
            listView.selectionModel()->select(noteIndex, QItemSelectionModel::ClearAndSelect);
            listView.setCurrentIndex(noteIndex);
            listView.scrollTo(noteIndex);
            if (listView.isPinnedList()) {
                m_listView->setCurrentIndex({});
                listView.setVisible(true);
                emit pinnedNoteListVisibleChanged(true);
            } else {
                m_pinnedNoteView->setCurrentIndex({});
            }
            emit showNoteInEditor(note);
        } else {
            qDebug() << "MainWindow::selectNote() : model is not valid";
        }
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
        noteIndex = m_pinnedNoteModel->getNoteIndex(note.id());
        if (noteIndex.isValid()) {
            m_pinnedNoteView->scrollToTop();
            // move the current selected note to the top
            QModelIndex destinationIndex = m_pinnedNoteModel->index(0);
            m_pinnedNoteModel->moveRow(noteIndex, noteIndex.row(), destinationIndex, 0);
            // update the current item
            noteIndex = destinationIndex;
            m_pinnedNoteView->setCurrentIndex(noteIndex);
        } else {
            qDebug() << "ListViewLogic::moveNoteToTop : Note is not in list";
        }
    }
}

void ListViewLogic::setNoteData(const NodeData &note)
{
    auto setNoteDataInternal = [this] (NoteListModel& model, const QModelIndex& index, const NodeData& note) {
        QMap<int, QVariant> dataValue;
        auto wasTemp = index.data(NoteListModel::NoteIsTemp).toBool();
        dataValue[NoteListModel::NoteContent] =  QVariant::fromValue(note.content());
        dataValue[NoteListModel::NoteFullTitle] =  QVariant::fromValue(note.fullTitle());
        dataValue[NoteListModel::NoteLastModificationDateTime] =
                QVariant::fromValue(note.lastModificationdateTime());
        dataValue[NoteListModel::NoteIsTemp] =
                QVariant::fromValue(note.isTempNote());
        dataValue[NoteListModel::NoteScrollbarPos] =
                QVariant::fromValue(note.scrollBarPosition());
        model.setItemData(index, dataValue);
        if (wasTemp) {
            auto tagIds = index.data(NoteListModel::NoteTagsList).value<QSet<int>>();
            for (const auto tagId : QT_AS_CONST(tagIds)) {
                emit requestAddTagDb(note.id(), tagId);
            }
        }
    };
    QModelIndex noteIndex = m_listModel->getNoteIndex(note.id());
    if (noteIndex.isValid()) {
        setNoteDataInternal(*m_listModel, noteIndex, note);
    } else {
        noteIndex = m_pinnedNoteModel->getNoteIndex(note.id());
        if (noteIndex.isValid()) {
            setNoteDataInternal(*m_pinnedNoteModel, noteIndex, note);
        } else {
            qDebug() << __FUNCTION__ << " : Note is not in list";
        }
    }
}

void ListViewLogic::onNoteEditClosed(const NodeData &note)
{
    if (note.isTempNote()) {
        QModelIndex noteIndex = m_listModel->getNoteIndex(note.id());
        if (noteIndex.isValid()) {
            m_listModel->removeNote(noteIndex);
        } else {
            noteIndex = m_pinnedNoteModel->getNoteIndex(note.id());
            if (noteIndex.isValid()) {
                m_pinnedNoteModel->removeNote(noteIndex);
            }
        }
    }
}

void ListViewLogic::deleteNoteRequested(const NodeData &note)
{
    auto index = m_listModel->getNoteIndex(note.id());
    if (index.isValid()) {
        deleteNoteRequestedI(*m_listView, index);
    } else {
        index = m_pinnedNoteModel->getNoteIndex(note.id());
        if (index.isValid()) {
            deleteNoteRequestedI(*m_pinnedNoteView, index);
        }
    }
}

void ListViewLogic::selectNoteUp()
{
    auto currentIndex = getCurrentIndex();
    if(currentIndex.first.isValid()) {
        int currentRow = currentIndex.first.row();
        QModelIndex aboveIndex = dynamic_cast<NoteListModel*>(currentIndex.second.model())
                ->index(currentRow - 1, 0);
        if (aboveIndex.isValid()) {
            selectNote(currentIndex.second, aboveIndex);
            currentIndex.second.setCurrentRowActive(false);
        } else {
            if ((!currentIndex.second.isPinnedList())) {
                aboveIndex = m_pinnedNoteModel->index(m_pinnedNoteModel->rowCount() - 1, 0);
                if (aboveIndex.isValid()) {
                    selectNote(*m_pinnedNoteView, aboveIndex);
                    m_pinnedNoteView->setCurrentRowActive(false);
                } else {
                    selectFirstNote();
                }
            } else {
                selectFirstNote();
            }
        }
    } else {
        selectFirstNote();
    }
    if (!m_searchEdit->text().isEmpty()) {
        m_searchEdit->setFocus();
    } else {
        m_listView->setFocus();
    }
}

void ListViewLogic::selectNoteDown()
{
    auto currentIndex = getCurrentIndex();
    if(currentIndex.first.isValid()) {
        int currentRow = currentIndex.first.row();
        QModelIndex belowIndex = dynamic_cast<NoteListModel*>(currentIndex.second.model())
                ->index(currentRow + 1, 0);
        if (belowIndex.isValid()) {
            selectNote(currentIndex.second, belowIndex);
            currentIndex.second.setCurrentRowActive(false);
        } else {
            if (currentIndex.second.isPinnedList()) {
                belowIndex = m_listModel->index(0, 0);
                if (belowIndex.isValid()) {
                    selectNote(*m_listView, belowIndex);
                    m_listView->setCurrentRowActive(false);
                } else {
                    selectLastNote();
                }
            } else {
                selectLastNote();
            }
        }
    } else {
        selectLastNote();
    }
    if (!m_searchEdit->text().isEmpty()) {
        m_searchEdit->setFocus();
    } else {
        m_listView->setFocus();
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
            auto index = getCurrentIndex();
            if (index.first.isValid()) {
                m_listViewInfo.currentNoteId = index.first.data(NoteListModel::NoteID).toInt();
            } else {
                m_listViewInfo.currentNoteId = SpecialNodeID::InvalidNodeId;
            }
        }
        m_clearButton->show();
        emit requestSearchInDb(keyword, m_listViewInfo);
    }
}

void ListViewLogic::clearSearch(bool createNewNote, int scrollToId)
{
    m_listViewInfo.needCreateNewNote = createNewNote;
    m_listViewInfo.scrollToId = scrollToId;
    emit requestClearSearchDb(m_listViewInfo);
    emit requestClearSearchUI();
}

void ListViewLogic::loadNoteListModel(const QVector<NodeData>& noteList, const ListViewInfo& inf)
{
    auto currentNoteId = m_listViewInfo.currentNoteId;
    m_listViewInfo = inf;
    bool isInAllNote = false;
    if ((!m_listViewInfo.isInTag) && m_listViewInfo.parentFolderId == SpecialNodeID::RootFolder) {
        isInAllNote = true;
    } else {
        isInAllNote = false;
    }
    m_noteListDelegate->setIsInAllNotes(isInAllNote);
    m_pinnedNoteListDelegate->setIsInAllNotes(isInAllNote);
    QVector<NodeData> pinned, notPinned;
    for (const auto& note: QT_AS_CONST(noteList)) {
        if (isInAllNote) {
            if (note.isPinnedNoteAN()) {
                pinned.push_back(note);
            } else {
                notPinned.push_back(note);
            }
        } else {
            if (note.isPinnedNote()) {
                pinned.push_back(note);
            } else {
                notPinned.push_back(note);
            }
        }
    }
    m_listModel->setListNote(notPinned, m_listViewInfo);
    m_listView->setListViewInfo(m_listViewInfo);
    m_pinnedNoteModel->setListNote(pinned, m_listViewInfo);
    m_pinnedNoteView->setListViewInfo(m_listViewInfo);
    updateListViewLabel();

    if ((!m_listViewInfo.isInTag) && m_listViewInfo.parentFolderId == SpecialNodeID::TrashFolder) {
        emit setNewNoteButtonVisible(false);
        m_listView->setIsInTrash(true);
        m_pinnedNoteView->setIsInTrash(true);
    } else {
        emit setNewNoteButtonVisible(true);
        m_listView->setIsInTrash(false);
        m_pinnedNoteView->setIsInTrash(false);
    }
    if (m_listViewInfo.isInTag) {
        m_listView->setCurrentFolderId(SpecialNodeID::InvalidNodeId);
        m_pinnedNoteView->setCurrentFolderId(SpecialNodeID::InvalidNodeId);
    } else {
        m_listView->setCurrentFolderId(m_listViewInfo.parentFolderId);
        m_pinnedNoteView->setCurrentFolderId(m_listViewInfo.parentFolderId);
    }

    if (m_listViewInfo.needCreateNewNote) {
        m_listViewInfo.needCreateNewNote = false;
        QTimer::singleShot(50, this, &ListViewLogic::requestNewNote);
    }

    if (!m_listViewInfo.isInSearch && currentNoteId != SpecialNodeID::InvalidNodeId) {
        bool sr = false;
        if (m_listViewInfo.scrollToId != SpecialNodeID::InvalidNodeId) {
            sr = true;
            currentNoteId = m_listViewInfo.scrollToId;
            m_listViewInfo.scrollToId = SpecialNodeID::InvalidNodeId;
        }
        auto index = getNoteIndex(currentNoteId);
        if (index.first.isValid()) {
            index.second.selectionModel()->select(index.first, QItemSelectionModel::ClearAndSelect);
            index.second.setCurrentIndex(index.first);
            if (sr) {
                index.second.scrollTo(index.first, QAbstractItemView::PositionAtCenter);
            }
            auto currentNote = dynamic_cast<NoteListModel*>(index.second.model())->getNote(index.first);
            emit showNoteInEditor(currentNote);
            return;
        }
    }
    if (m_needLoadSavedState > 0) {
        m_needLoadSavedState -= 1;
        if (m_lastSelectedNote != SpecialNodeID::InvalidNodeId) {
            auto index = getNoteIndex(m_lastSelectedNote);
            if (index.first.isValid()) {
                selectNote(index.second, index.first);
                return;
            }
        }
    }
    selectFirstNote();
}

void ListViewLogic::onAddTagRequest(NoteListView& listView, const QModelIndex &index, int tagId)
{
    if (index.isValid()) {
        auto noteId = index.data(NoteListModel::NoteID).toInt();
        auto isTemp = index.data(NoteListModel::NoteIsTemp).toBool();
        if (!isTemp) {
            emit requestAddTagDb(noteId, tagId);
        }
        auto tagIds = index.data(NoteListModel::NoteTagsList).value<QSet<int>>();
        tagIds.insert(tagId);
        auto model = dynamic_cast<NoteListModel*>(listView.model());
        if (model) {
            model->setData(index, QVariant::fromValue(tagIds), NoteListModel::NoteTagsList);
        } else {
            qDebug() << __FUNCTION__ << "model is not valid";
        }
        listView.closePersistentEditorC(index);
        listView.openPersistentEditorC(index);
        emit noteTagListChanged(noteId, tagIds);
    } else {
        qDebug() << __FUNCTION__ << "index is not valid";
    }
}

void ListViewLogic::onAddTagRequestD(int noteId, int tagId)
{
    auto index = m_listModel->getNoteIndex(noteId);
    if (index.isValid()) {
        onAddTagRequest(*m_listView, index, tagId);
    } else {
        index = m_pinnedNoteModel->getNoteIndex(noteId);
        if (index.isValid()) {
            onAddTagRequest(*m_pinnedNoteView, index, tagId);
        }
    }
}

void ListViewLogic::onNoteMovedOut(int nodeId, int targetId)
{
    auto index = getNoteIndex(nodeId);
    if (index.first.isValid()) {
        if ((!m_listViewInfo.isInTag
             && m_listViewInfo.parentFolderId != SpecialNodeID::RootFolder
             && m_listViewInfo.parentFolderId != targetId)
                || targetId == SpecialNodeID::TrashFolder) {
            selectNoteDown();
            auto model = dynamic_cast<NoteListModel*>(index.second.model());
            if (model) {
                model->removeNote(index.first);
                if (model->rowCount() == 0) {
                    emit closeNoteEditor();
                }
            } else {
                qDebug() << __FUNCTION__ << "model is not valid";
            }

        } else {
            NodeData note;
            QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                      Q_RETURN_ARG(NodeData, note),
                                      Q_ARG(int, nodeId)
                                      );
            if (note.id() != SpecialNodeID::InvalidNodeId) {
                index.second.closePersistentEditorC(index.first);
                auto model = dynamic_cast<NoteListModel*>(index.second.model());
                if (model) {
                    model->setNoteData(index.first, note);
                } else {
                    qDebug() << __FUNCTION__ << "model is not valid";
                }
                index.second.openPersistentEditorC(index.first);
            } else {
                qDebug() << __FUNCTION__ << "Note id" << nodeId << "not found!";
            }
        }
    }
}

void ListViewLogic::onRemoveTagRequest(NoteListView& listView, const QModelIndex &index, int tagId)
{
    if (index.isValid()) {
        auto noteId = index.data(NoteListModel::NoteID).toInt();
        auto isTemp = index.data(NoteListModel::NoteIsTemp).toBool();
        if (!isTemp) {
            emit requestRemoveTagDb(noteId, tagId);
        }
        auto tagIds = index.data(NoteListModel::NoteTagsList).value<QSet<int>>();
        tagIds.remove(tagId);
        auto model = dynamic_cast<NoteListModel*>(listView.model());
        if (model) {
            model->setData(index, QVariant::fromValue(tagIds), NoteListModel::NoteTagsList);
        } else {
            qDebug() << __FUNCTION__ << "model is not valid";
        }
        listView.closePersistentEditorC(index);
        listView.openPersistentEditorC(index);
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

void ListViewLogic::onNotePressed(NoteListView& listView, const QModelIndex &index)
{
    selectNote(listView, index);
    listView.setCurrentRowActive(false);
}

void ListViewLogic::deleteNoteRequestedI(NoteListView& listView, const QModelIndex &index)
{
    if (index.isValid()) {
        auto model = dynamic_cast<NoteListModel*>(listView.model());
        if (!model) {
            qDebug() << __FUNCTION__ << "model is not valid";
            return;
        }

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
                selectNoteDown();
                model->removeNote(index);
                if (model->rowCount() == 0) {
                    emit closeNoteEditor();
                }
                emit requestRemoveNoteDb(note);
            }
        } else {
            selectNoteDown();
            model->removeNote(index);
            if (model->rowCount() == 0) {
                emit closeNoteEditor();
            }
            emit requestRemoveNoteDb(note);
        }
    }
}

void ListViewLogic::restoreNoteRequestedI(NoteListView& listView, const QModelIndex &index)
{
    if (index.isValid()) {
        auto model = dynamic_cast<NoteListModel*>(listView.model());
        if (!model) {
            qDebug() << __FUNCTION__ << "model is not valid";
            return;
        }
        auto id = index.data(NoteListModel::NoteID).toInt();
        NodeData note;
        QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(NodeData, note),
                                  Q_ARG(int, id)
                                  );
        if (note.parentId() == SpecialNodeID::TrashFolder) {
            model->removeNote(index);
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

void ListViewLogic::updateListViewLabel()
{
    QString l1, l2;
    if ((!m_listViewInfo.isInTag) && m_listViewInfo.parentFolderId == SpecialNodeID::RootFolder) {
        l1 = "All Notes";
    } else if ((!m_listViewInfo.isInTag) && m_listViewInfo.parentFolderId == SpecialNodeID::TrashFolder) {
        l1 = "Trash";
    } else if (!m_listViewInfo.isInTag) {
        NodeData parentFolder;
        QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(NodeData, parentFolder),
                                  Q_ARG(int, m_listViewInfo.parentFolderId)
                                  );
        l1 = parentFolder.fullTitle();
    } else {
        if (m_listViewInfo.currentTagList.size() == 0) {
            l1 = "Tags ...";
        } else if (m_listViewInfo.currentTagList.size() > 1) {
            l1 = "Multiple tags ...";
        } else {
            int tagId = *m_listViewInfo.currentTagList.begin();
            if (!m_tagPool->contains(tagId)) {
                l1 = "Tags ...";
            } else {
                TagData tag = m_tagPool->getTag(tagId);
                l1 = tag.name();
            }
        }
    }
    l2 = QString::number(m_listModel->rowCount() + m_pinnedNoteModel->rowCount());
    emit listViewLabelChanged(l1, l2, havePinnedNote());
}

void ListViewLogic::onRowCountChanged(NoteListView& listView)
{
    listView.closeAllEditor();
    auto model = dynamic_cast<NoteListModel*>(listView.model());
    if (!model) {
        qDebug() << __FUNCTION__ << "model is not valid";
        return;
    }
    auto delegate = dynamic_cast<NoteListDelegate*>(listView.itemDelegate());
    if (!delegate) {
        qDebug() << __FUNCTION__ << "delegate is not valid";
        return;
    }

    delegate->clearSizeMap();
    for (int i = 0; i < model->rowCount(); ++i) {
        auto index = model->index(i, 0);
        auto y = listView.visualRect(index).y();
        auto range = abs(listView.viewport()->height());
        if (y < -range) {
            continue;
        } else if (y > 2 * range) {
            break;
        }
        listView.openPersistentEditorC(index);
    }
}

void ListViewLogic::onNoteDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid() || !m_listViewInfo.isInSearch) {
        return;
    }
    auto id = index.data(NoteListModel::NoteID).toInt();
    clearSearch(false, id);
}

void ListViewLogic::onSetPinnedNoteRequested(int noteId, bool isPinned)
{
    auto index = getNoteIndex(noteId);
    if (index.first.isValid()) {
        auto model = dynamic_cast<NoteListModel*>(index.second.model());
        if (model) {
            auto note = model->getNote(index.first);
            if (note.isPinnedNote() != isPinned) {
                note.setIsPinnedNote(isPinned);
                model->removeNote(index.first);
                if (isPinned) {
                    m_pinnedNoteModel->addNote(note);
                    m_pinnedNoteModel->updatePinnedRelativePosition();
                } else {
                    m_listModel->addNote(note);
                    m_listModel->sort(0, Qt::AscendingOrder);
                }
                emit requestUpdatePinnedDb(noteId, isPinned);
                auto newIndex = getNoteIndex(noteId);
                selectNote(newIndex.second, newIndex.first);
            }
        } else {
            qDebug() << __FUNCTION__ << "model is not valid";
        }
    }
}

bool ListViewLogic::havePinnedNote() const
{
    return m_pinnedNoteModel->rowCount() > 0;
}

std::pair<QModelIndex, NoteListView&> ListViewLogic::getNoteIndex(int id) const
{
    auto index = m_listModel->getNoteIndex(id);
    if (index.isValid()) {
        return std::make_pair(index, std::ref(*m_listView));
    } else {
        index = m_pinnedNoteModel->getNoteIndex(id);
        return std::make_pair(index, std::ref(*m_pinnedNoteView));
    }
}

std::pair<QModelIndex, NoteListView &> ListViewLogic::getCurrentIndex() const
{
    auto index = m_listView->currentIndex();
    if (index.isValid()) {
        return std::make_pair(index, std::ref(*m_listView));
    } else {
        index = m_pinnedNoteView->currentIndex();
        return std::make_pair(index, std::ref(*m_pinnedNoteView));
    }
}

void ListViewLogic::selectFirstNote()
{
    if (m_pinnedNoteModel->rowCount() > 0) {
        QModelIndex index = m_pinnedNoteModel->index(0,0);
        if (index.isValid()) {
            m_pinnedNoteView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
            m_pinnedNoteView->setCurrentIndex(index);
            auto firstNote = m_pinnedNoteModel->getNote(index);
            emit showNoteInEditor(firstNote);
        }
    } else if (m_listModel->rowCount() > 0){
        QModelIndex index = m_listModel->index(0,0);
        if (index.isValid()) {
            m_listView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
            m_listView->setCurrentIndex(index);
            auto firstNote = m_listModel->getNote(index);
            emit showNoteInEditor(firstNote);
        }
    } else {
        emit closeNoteEditor();
    }
}

void ListViewLogic::selectLastNote()
{
    if (m_listModel->rowCount() > 0){
        QModelIndex index = m_listModel->index(m_listModel->rowCount() - 1, 0);
        if (index.isValid()) {
            m_listView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
            m_listView->setCurrentIndex(index);
            auto firstNote = m_listModel->getNote(index);
            emit showNoteInEditor(firstNote);
        }
    } else if (m_pinnedNoteModel->rowCount() > 0) {
        QModelIndex index = m_pinnedNoteModel->index(m_pinnedNoteModel->rowCount() - 1, 0);
        if (index.isValid()) {
            m_pinnedNoteView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
            m_pinnedNoteView->setCurrentIndex(index);
            auto firstNote = m_pinnedNoteModel->getNote(index);
            emit showNoteInEditor(firstNote);
        }
    } else {
        emit closeNoteEditor();
    }
}

void ListViewLogic::setTheme(Theme theme)
{
    m_listView->setTheme(theme);
    m_noteListDelegate->setTheme(theme);
    m_listView->update();
    m_pinnedNoteView->setTheme(theme);
    m_pinnedNoteListDelegate->setTheme(theme);
    m_pinnedNoteView->update();
}

bool ListViewLogic::isAnimationRunning()
{
    return m_noteListDelegate->animationState() == QTimeLine::Running
            || m_pinnedNoteListDelegate->animationState() == QTimeLine::Running ;
}

void ListViewLogic::setLastSavedState(int lastSelectedNote)
{
    m_needLoadSavedState = 2;
    m_lastSelectedNote = lastSelectedNote;
}

const ListViewInfo &ListViewLogic::listViewInfo() const
{
    return m_listViewInfo;
}

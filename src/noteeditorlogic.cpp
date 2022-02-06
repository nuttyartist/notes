#include "noteeditorlogic.h"
#include "customDocument.h"
#include "markdownhighlighter.h"
#include "dbmanager.h"
#include "taglistview.h"
#include "taglistmodel.h"
#include "tagpool.h"
#include "taglistdelegate.h"
#include <QScrollBar>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>

#define FIRST_LINE_MAX 80

NoteEditorLogic::NoteEditorLogic(CustomDocument *textEdit,
                                 QLabel* editorDateLabel,
                                 QLineEdit* searchEdit,
                                 TagListView *tagListView, TagPool *tagPool,
                                 DBManager *dbManager,
                                 QObject *parent) : QObject(parent),
    m_textEdit{textEdit},
    m_editorDateLabel{editorDateLabel},
    m_searchEdit{searchEdit},
    m_tagListView{tagListView},
    m_dbManager{dbManager},
    m_isContentModified{false}
{
    m_highlighter = new MarkdownHighlighter(m_textEdit->document());
    connect(m_textEdit, &QTextEdit::textChanged, this, &NoteEditorLogic::onTextEditTextChanged);
    connect(this, &NoteEditorLogic::requestCreateUpdateNote,
            m_dbManager, &DBManager::onCreateUpdateRequestedNoteContent, Qt::QueuedConnection);
    // auto save timer
    m_autoSaveTimer.setSingleShot(true);
    m_autoSaveTimer.setInterval(50);
    connect(&m_autoSaveTimer, &QTimer::timeout, this, [this]() {
        saveNoteToDB();
    });
    m_tagListModel = new TagListModel{this};
    m_tagListModel->setTagPool(tagPool);
    m_tagListView->setModel(m_tagListModel);
    m_tagListDelegate = new TagListDelegate{this};
    m_tagListView->setItemDelegate(m_tagListDelegate);
    connect(tagPool, &TagPool::dataUpdated, this, [this] (int) {
        showTagListForCurrentNote();
    });
}

bool NoteEditorLogic::markdownEnabled() const
{
    return m_highlighter == nullptr;
}

void NoteEditorLogic::setMarkdownEnabled(bool newMarkdownEnabled)
{
    if (markdownEnabled()) {
        delete m_highlighter;
        m_highlighter = nullptr;
    }
    if (newMarkdownEnabled) {
        m_highlighter = new MarkdownHighlighter(m_textEdit->document());
    } else {
        delete m_highlighter;
        m_highlighter = nullptr;
    }
}

void NoteEditorLogic::showNoteInEditor(const NodeData &note)
{
    if (note.id() != m_currentNote.id()) {
        emit noteEditClosed(m_currentNote);
    }
    m_textEdit->blockSignals(true);
    m_currentNote = note;
    showTagListForCurrentNote();
//    / fixing bug #202
    m_textEdit->setTextBackgroundColor(QColor(247,247,247, 0));

    QString content = note.content();
    QDateTime dateTime = note.lastModificationdateTime();
    int scrollbarPos = note.scrollBarPosition();

    // set text and date
    m_textEdit->setText(content);
    QString noteDate = dateTime.toString(Qt::ISODate);
    QString noteDateEditor = getNoteDateEditor(noteDate);
    m_editorDateLabel->setText(noteDateEditor);
    // set scrollbar position
    m_textEdit->verticalScrollBar()->setValue(scrollbarPos);
    m_textEdit->blockSignals(false);

    highlightSearch();
}

void NoteEditorLogic::onTextEditTextChanged()
{
    if (m_currentNote.id() != SpecialNodeID::InvalidNodeId) {
        m_textEdit->blockSignals(true);
        QString content = m_currentNote.content();
        if(m_textEdit->toPlainText() != content){
            // move note to the top of the list
            emit moveNoteToListViewTop(m_currentNote);

            // Get the new data
            QString firstline = getFirstLine(m_textEdit->toPlainText());
            QDateTime dateTime = QDateTime::currentDateTime();
            QString noteDate = dateTime.toString(Qt::ISODate);
            m_editorDateLabel->setText(NoteEditorLogic::getNoteDateEditor(noteDate));
            // update note data
            m_currentNote.setContent(m_textEdit->toPlainText());
            m_currentNote.setFullTitle(firstline);
            m_currentNote.setLastModificationDateTime(dateTime);
            m_currentNote.setIsTempNote(false);
            m_currentNote.setScrollBarPosition(m_textEdit->verticalScrollBar()->value());
            // update note data in list view
            emit updateNoteDataInList(m_currentNote);
            m_isContentModified = true;
            m_autoSaveTimer.start();
            emit setVisibilityOfFrameRightNonEditor(false);
        }
        m_textEdit->blockSignals(false);
    } else {
        qDebug() << "NoteEditorLogic::onTextEditTextChanged() : m_currentNote is not valid";
    }
}

QDateTime NoteEditorLogic::getQDateTime(QString date)
{
    QDateTime dateTime = QDateTime::fromString(date, Qt::ISODate);
    return dateTime;
}

void NoteEditorLogic::showTagListForCurrentNote()
{
    if (m_currentNote.id() != SpecialNodeID::InvalidNodeId) {
        auto tagIds = m_currentNote.tagIds();
        if (tagIds.count() > 0) {
            m_tagListView->setVisible(true);
            m_tagListModel->setModelData(tagIds);
            return;
        }
        m_tagListModel->setModelData(tagIds);
    }
    m_tagListView->setVisible(false);
}

void NoteEditorLogic::saveNoteToDB()
{
    if(m_currentNote.id() != SpecialNodeID::InvalidNodeId
            && m_isContentModified && !m_currentNote.isTempNote()) {
        emit requestCreateUpdateNote(m_currentNote);
        m_isContentModified = false;
    }
}

void NoteEditorLogic::closeEditor()
{
    if (m_currentNote.id() != SpecialNodeID::InvalidNodeId) {
        saveNoteToDB();
        emit noteEditClosed(m_currentNote);
        m_currentNote.setId(SpecialNodeID::InvalidNodeId);
    } /*else {
        qDebug() << "NoteEditorLogic::closeEditor() : m_currentNote is not valid";
    }*/
    m_textEdit->blockSignals(true);
    m_textEdit->clear();
    m_textEdit->clearFocus();
    m_textEdit->blockSignals(false);
    m_tagListModel->setModelData({});
}

void NoteEditorLogic::onNoteTagListChanged(int noteId, const QSet<int> tagIds)
{
    if (m_currentNote.id() == noteId) {
        m_currentNote.setTagIds(tagIds);
        showTagListForCurrentNote();
    }
}

NodeData NoteEditorLogic::currentEditingNote() const
{
    return m_currentNote;
}

void NoteEditorLogic::deleteCurrentNote()
{
    if (m_currentNote.id() != SpecialNodeID::InvalidNodeId) {
        auto noteNeedDeleted = m_currentNote;
        saveNoteToDB();
        m_currentNote.setId(SpecialNodeID::InvalidNodeId);
        m_textEdit->blockSignals(true);
        m_textEdit->clear();
        m_textEdit->clearFocus();
        m_textEdit->blockSignals(false);
        emit noteEditClosed(noteNeedDeleted);
        emit deleteNoteRequested(noteNeedDeleted);
    }
}

/*!
 * \brief NoteEditorLogic::getFirstLine
 * Get a string 'str' and return only the first line of it
 * If the string contain no text, return "New Note"
 * TODO: We might make it more efficient by not loading the entire string into the memory
 * \param str
 * \return
 */
QString NoteEditorLogic::getFirstLine(const QString& str)
{
    if(str.simplified().isEmpty())
        return "New Note";

    QString text = str.trimmed();
    QTextStream ts(&text);
    return ts.readLine(FIRST_LINE_MAX);
}

QString NoteEditorLogic::getSecondLine(const QString &str)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    auto sl = str.split("\n", QString::SkipEmptyParts);
#else
    auto sl = str.split("\n", Qt::SkipEmptyParts);
#endif
    if (sl.size() < 2) {
        return getFirstLine(str);
    }
    int i = 1;
    QString text;
    do {
        if (i >= sl.size()) {
            return getFirstLine(str);
        }
        text = sl[i].trimmed();
        ++i;
    } while (text.isEmpty());
    QTextStream ts(&text);
    return ts.readLine(FIRST_LINE_MAX);
}

void NoteEditorLogic::setTheme(Theme newTheme)
{
    m_tagListDelegate->setTheme(newTheme);
}

QString NoteEditorLogic::getNoteDateEditor(QString dateEdited)
{
    QDateTime dateTimeEdited(getQDateTime(dateEdited));
    QLocale usLocale(QLocale(QStringLiteral("en_US")));

    return usLocale.toString(dateTimeEdited, QStringLiteral("MMMM d, yyyy, h:mm A"));
}

void NoteEditorLogic::highlightSearch() const
{
    QString searchString = m_searchEdit->text();

    if (searchString.isEmpty())
        return;

    m_textEdit->moveCursor(QTextCursor::Start);

    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextCharFormat highlightFormat;
    highlightFormat.setBackground(Qt::yellow);

    while (m_textEdit->find(searchString))
        extraSelections.append({ m_textEdit->textCursor(), highlightFormat});

    if (!extraSelections.isEmpty()) {
        m_textEdit->setTextCursor(extraSelections.first().cursor);
        m_textEdit->setExtraSelections(extraSelections);
    }
}

bool NoteEditorLogic::isTempNote() const
{
    if (m_currentNote.id() != SpecialNodeID::InvalidNodeId && m_currentNote.isTempNote()) {
        return true;
    }
    return false;
}

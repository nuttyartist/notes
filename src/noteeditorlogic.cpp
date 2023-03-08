#include "noteeditorlogic.h"
#include "customDocument.h"
#include "customMarkdownHighlighter.h"
#include "dbmanager.h"
#include "taglistview.h"
#include "taglistmodel.h"
#include "tagpool.h"
#include "taglistdelegate.h"
#include <QScrollBar>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QDebug>
#include <QCursor>

#define FIRST_LINE_MAX 80

NoteEditorLogic::NoteEditorLogic(CustomDocument *textEdit, QLabel *editorDateLabel,
                                 QLineEdit *searchEdit, TagListView *tagListView, TagPool *tagPool,
                                 DBManager *dbManager, QObject *parent)
    : QObject(parent),
      m_textEdit{ textEdit },
      m_highlighter{ new CustomMarkdownHighlighter{ m_textEdit->document() } },
      m_editorDateLabel{ editorDateLabel },
      m_searchEdit{ searchEdit },
      m_tagListView{ tagListView },
      m_dbManager{ dbManager },
      m_isContentModified{ false },
      m_spacerColor{ 191, 191, 191 },
      m_currentAdaptableEditorPadding{ 0 },
      m_currentMinimumEditorPadding{ 0 }
{
    connect(m_textEdit, &QTextEdit::textChanged, this, &NoteEditorLogic::onTextEditTextChanged);
    connect(this, &NoteEditorLogic::requestCreateUpdateNote, m_dbManager,
            &DBManager::onCreateUpdateRequestedNoteContent, Qt::QueuedConnection);
    // auto save timer
    m_autoSaveTimer.setSingleShot(true);
    m_autoSaveTimer.setInterval(50);
    connect(&m_autoSaveTimer, &QTimer::timeout, this, [this]() { saveNoteToDB(); });
    m_tagListModel = new TagListModel{ this };
    m_tagListModel->setTagPool(tagPool);
    m_tagListView->setModel(m_tagListModel);
    m_tagListDelegate = new TagListDelegate{ this };
    m_tagListView->setItemDelegate(m_tagListDelegate);
    connect(tagPool, &TagPool::dataUpdated, this, [this](int) { showTagListForCurrentNote(); });
    connect(m_textEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        if (m_currentNotes.size() == 1 && m_currentNotes[0].id() != SpecialNodeID::InvalidNodeId) {
            m_currentNotes[0].setScrollBarPosition(value);
            emit updateNoteDataInList(m_currentNotes[0]);
            m_isContentModified = true;
            m_autoSaveTimer.start();
        }
    });
}

bool NoteEditorLogic::markdownEnabled() const
{
    return m_highlighter->document() != nullptr;
}

void NoteEditorLogic::setMarkdownEnabled(bool enabled)
{
    m_highlighter->setDocument(enabled ? m_textEdit->document() : nullptr);
}

void NoteEditorLogic::showNotesInEditor(const QVector<NodeData> &notes)
{
    if (notes.size() == 1 && notes[0].id() != SpecialNodeID::InvalidNodeId) {
        auto currentId = currentEditingNoteId();
        if (currentId != SpecialNodeID::InvalidNodeId && notes[0].id() != currentId) {
            emit noteEditClosed(m_currentNotes[0], false);
        }
        m_textEdit->blockSignals(true);
        m_textEdit->setVisible(true);
        m_currentNotes = notes;
        showTagListForCurrentNote();
        //     fixing bug #202
        m_textEdit->setTextBackgroundColor(QColor(247, 247, 247, 0));

        QString content = notes[0].content();
        QDateTime dateTime = notes[0].lastModificationdateTime();
        int scrollbarPos = notes[0].scrollBarPosition();

        // set text and date
        m_textEdit->setText(content);
        QString noteDate = dateTime.toString(Qt::ISODate);
        QString noteDateEditor = getNoteDateEditor(noteDate);
        m_editorDateLabel->setText(noteDateEditor);
        // set scrollbar position
        m_textEdit->verticalScrollBar()->setValue(scrollbarPos);
        m_textEdit->blockSignals(false);
        m_textEdit->setReadOnly(false);
        m_textEdit->setTextInteractionFlags(Qt::TextEditorInteraction);
        m_textEdit->setFocusPolicy(Qt::StrongFocus);
        highlightSearch();
    } else if (notes.size() > 1) {
        m_currentNotes = notes;
        m_tagListView->setVisible(false);
        m_textEdit->blockSignals(true);
        auto verticalScrollBarValueToRestore = m_textEdit->verticalScrollBar()->value();
        m_textEdit->clear();
        auto padding = m_currentAdaptableEditorPadding > m_currentMinimumEditorPadding
                ? m_currentAdaptableEditorPadding
                : m_currentMinimumEditorPadding;
        QPixmap sep(QSize{ m_textEdit->width() - padding * 2 - 12, 4 });
        sep.fill(Qt::transparent);
        QPainter painter(&sep);
        painter.setPen(m_spacerColor);
        painter.drawRect(0, 1, sep.width(), 1);
        m_textEdit->document()->addResource(QTextDocument::ImageResource, QUrl("mydata://sep.png"),
                                            sep);
        for (int i = 0; i < notes.size(); ++i) {
            auto cursor = m_textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            if (!notes[i].content().endsWith("\n")) {
                if (i != 0) {
                    cursor.insertText("\n" + notes[i].content() + "\n");
                } else {
                    cursor.insertText(notes[i].content() + "\n");
                }
            } else {
                cursor.insertText(notes[i].content());
            }
            if (i != notes.size() - 1) {
                cursor.movePosition(QTextCursor::End);
                cursor.insertText("\n");
                cursor.insertImage("mydata://sep.png");
                cursor.insertText("\n");
            }
        }
        m_textEdit->verticalScrollBar()->setValue(verticalScrollBarValueToRestore);
        m_textEdit->blockSignals(false);
        m_textEdit->setReadOnly(true);
        m_textEdit->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_textEdit->setFocusPolicy(Qt::NoFocus);
        highlightSearch();
    }
}

void NoteEditorLogic::onTextEditTextChanged()
{
    if (currentEditingNoteId() != SpecialNodeID::InvalidNodeId) {
        m_textEdit->blockSignals(true);
        QString content = m_currentNotes[0].content();
        if (m_textEdit->toPlainText() != content) {
            // move note to the top of the list
            emit moveNoteToListViewTop(m_currentNotes[0]);

            // Get the new data
            QString firstline = getFirstLine(m_textEdit->toPlainText());
            QDateTime dateTime = QDateTime::currentDateTime();
            QString noteDate = dateTime.toString(Qt::ISODate);
            m_editorDateLabel->setText(NoteEditorLogic::getNoteDateEditor(noteDate));
            // update note data
            m_currentNotes[0].setContent(m_textEdit->toPlainText());
            m_currentNotes[0].setFullTitle(firstline);
            m_currentNotes[0].setLastModificationDateTime(dateTime);
            m_currentNotes[0].setIsTempNote(false);
            m_currentNotes[0].setScrollBarPosition(m_textEdit->verticalScrollBar()->value());
            // update note data in list view
            emit updateNoteDataInList(m_currentNotes[0]);
            m_isContentModified = true;
            m_autoSaveTimer.start();
            emit setVisibilityOfFrameRightNonEditor(false);
        }
        m_textEdit->blockSignals(false);
    } else {
        qDebug() << "NoteEditorLogic::onTextEditTextChanged() : m_currentNote is not valid";
    }
}

QDateTime NoteEditorLogic::getQDateTime(const QString &date)
{
    QDateTime dateTime = QDateTime::fromString(date, Qt::ISODate);
    return dateTime;
}

void NoteEditorLogic::showTagListForCurrentNote()
{
    if (currentEditingNoteId() != SpecialNodeID::InvalidNodeId) {
        auto tagIds = m_currentNotes[0].tagIds();
        if (tagIds.count() > 0) {
            m_tagListView->setVisible(true);
            m_tagListModel->setModelData(tagIds);
            return;
        }
        m_tagListModel->setModelData(tagIds);
    }
    m_tagListView->setVisible(false);
}

bool NoteEditorLogic::isInEditMode() const
{
    if (m_currentNotes.size() == 1) {
        return true;
    }
    return false;
}

int NoteEditorLogic::currentMinimumEditorPadding() const
{
    return m_currentMinimumEditorPadding;
}

void NoteEditorLogic::setCurrentMinimumEditorPadding(int newCurrentMinimumEditorPadding)
{
    m_currentMinimumEditorPadding = newCurrentMinimumEditorPadding;
}

int NoteEditorLogic::currentAdaptableEditorPadding() const
{
    return m_currentAdaptableEditorPadding;
}

void NoteEditorLogic::setCurrentAdaptableEditorPadding(int newCurrentAdaptableEditorPadding)
{
    m_currentAdaptableEditorPadding = newCurrentAdaptableEditorPadding;
}

int NoteEditorLogic::currentEditingNoteId() const
{
    if (isInEditMode()) {
        return m_currentNotes[0].id();
    }
    return SpecialNodeID::InvalidNodeId;
}

void NoteEditorLogic::saveNoteToDB()
{
    if (currentEditingNoteId() != SpecialNodeID::InvalidNodeId && m_isContentModified
        && !m_currentNotes[0].isTempNote()) {
        emit requestCreateUpdateNote(m_currentNotes[0]);
        m_isContentModified = false;
    }
}

void NoteEditorLogic::closeEditor()
{
    if (currentEditingNoteId() != SpecialNodeID::InvalidNodeId) {
        saveNoteToDB();
        emit noteEditClosed(m_currentNotes[0], false);
    }
    m_currentNotes.clear();

    m_textEdit->blockSignals(true);
    m_textEdit->clear();
    m_textEdit->clearFocus();
    m_textEdit->blockSignals(false);
    m_tagListModel->setModelData({});
}

void NoteEditorLogic::onNoteTagListChanged(int noteId, const QSet<int> &tagIds)
{
    if (currentEditingNoteId() == noteId) {
        m_currentNotes[0].setTagIds(tagIds);
        showTagListForCurrentNote();
    }
}

void NoteEditorLogic::deleteCurrentNote()
{
    if (isTempNote()) {
        auto noteNeedDeleted = m_currentNotes[0];
        m_currentNotes.clear();
        m_textEdit->blockSignals(true);
        m_textEdit->clear();
        m_textEdit->clearFocus();
        m_textEdit->blockSignals(false);
        emit noteEditClosed(noteNeedDeleted, true);
    } else if (currentEditingNoteId() != SpecialNodeID::InvalidNodeId) {
        auto noteNeedDeleted = m_currentNotes[0];
        saveNoteToDB();
        m_currentNotes.clear();
        m_textEdit->blockSignals(true);
        m_textEdit->clear();
        m_textEdit->clearFocus();
        m_textEdit->blockSignals(false);
        emit noteEditClosed(noteNeedDeleted, false);
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
QString NoteEditorLogic::getFirstLine(const QString &str)
{
    QString text = str.trimmed();
    if (text.isEmpty()) {
        return "New Note";
    }
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

void NoteEditorLogic::setTheme(Theme newTheme, QColor textColor)
{
    m_tagListDelegate->setTheme(newTheme);
    m_highlighter->setTheme(newTheme, textColor);
    switch (newTheme) {
    case Theme::Light: {
        m_spacerColor = QColor(191, 191, 191);
        break;
    }
    case Theme::Dark: {
        m_spacerColor = QColor(204, 204, 204);
        break;
    }
    case Theme::Sepia: {
        m_spacerColor = QColor(191, 191, 191);
        break;
    }
    }
    if (currentEditingNoteId() != SpecialNodeID::InvalidNodeId) {
        int verticalScrollBarValueToRestore = m_textEdit->verticalScrollBar()->value();
        m_textEdit->setText(
                m_textEdit->toPlainText()); // TODO: Update the text color without setting the text
        m_textEdit->verticalScrollBar()->setValue(verticalScrollBarValueToRestore);
    } else {
        int verticalScrollBarValueToRestore = m_textEdit->verticalScrollBar()->value();
        showNotesInEditor(m_currentNotes);
        m_textEdit->verticalScrollBar()->setValue(verticalScrollBarValueToRestore);
    }
}

QString NoteEditorLogic::getNoteDateEditor(const QString &dateEdited)
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
        extraSelections.append({ m_textEdit->textCursor(), highlightFormat });

    if (!extraSelections.isEmpty()) {
        m_textEdit->setTextCursor(extraSelections.first().cursor);
        m_textEdit->setExtraSelections(extraSelections);
    }
}

bool NoteEditorLogic::isTempNote() const
{
    if (currentEditingNoteId() != SpecialNodeID::InvalidNodeId && m_currentNotes[0].isTempNote()) {
        return true;
    }
    return false;
}

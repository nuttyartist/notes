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

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)

NoteEditorLogic::NoteEditorLogic(CustomDocument *textEdit, QLabel *editorDateLabel,
                                 QLineEdit *searchEdit, QWidget *kanbanWidget,
                                 TagListView *tagListView, TagPool *tagPool, DBManager *dbManager,
                                 QObject *parent)
#else
NoteEditorLogic::NoteEditorLogic(CustomDocument *textEdit, QLabel *editorDateLabel,
                                 QLineEdit *searchEdit, TagListView *tagListView, TagPool *tagPool,
                                 DBManager *dbManager, QObject *parent)
#endif
    : QObject(parent),
      m_textEdit{ textEdit },
      m_highlighter{ new CustomMarkdownHighlighter{ m_textEdit->document() } },
      m_editorDateLabel{ editorDateLabel },
      m_searchEdit{ searchEdit },
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
      m_kanbanWidget{ kanbanWidget },
#endif
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    connect(this, &NoteEditorLogic::showKanbanView, this, [this]() {
        if (m_kanbanWidget != nullptr) {
            emit setVisibilityOfFrameRightNonEditor(false);
            bool shouldRecheck = checkForTasksInEditor();
            if (shouldRecheck) {
                checkForTasksInEditor();
            }
            m_kanbanWidget->show();
            m_textEdit->hide();
            m_textEdit->clearFocus();
            emit kanbanShown();
        }
    });
    connect(this, &NoteEditorLogic::hideKanbanView, this, [this]() {
        if (m_kanbanWidget != nullptr) {
            emit setVisibilityOfFrameRightNonEditor(true);
            m_kanbanWidget->hide();
            m_textEdit->show();
            emit clearKanbanModel();
            emit textShown();
        }
    });
#endif
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
    auto currentId = currentEditingNoteId();
    if (notes.size() == 1 && notes[0].id() != SpecialNodeID::InvalidNodeId) {
        if (currentId != SpecialNodeID::InvalidNodeId && notes[0].id() != currentId) {
            emit noteEditClosed(m_currentNotes[0], false);
        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        emit resetKanbanSettings();
        emit checkMultipleNotesSelected(
                QVariant(false)); // TODO: if not PRO version, should be true
#endif

        m_textEdit->blockSignals(true);

        m_currentNotes = notes;
        showTagListForCurrentNote();
        //     fixing bug #202
        m_textEdit->setTextBackgroundColor(QColor(247, 247, 247, 0));

        QString content = notes[0].content();
        QDateTime dateTime = notes[0].lastModificationdateTime();
        int scrollbarPos = notes[0].scrollBarPosition();

        // set text and date
        bool isTextChanged = content != m_textEdit->toPlainText();
        if (isTextChanged) {
            m_textEdit->setText(content);
        }
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        if (m_kanbanWidget != nullptr && m_kanbanWidget->isVisible()) {
            emit clearKanbanModel();
            bool shouldRecheck = checkForTasksInEditor();
            if (shouldRecheck) {
                checkForTasksInEditor();
            }
            m_textEdit->setVisible(false);
            return;
        } else {
            m_textEdit->setVisible(true);
        }
#else
        m_textEdit->setVisible(true);
#endif
        emit textShown();
    } else if (notes.size() > 1) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        emit checkMultipleNotesSelected(QVariant(true));
#endif
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
            emit setVisibilityOfFrameRightWidgets(false);
        }
        m_textEdit->blockSignals(false);
    } else {
        qDebug() << "NoteEditorLogic::onTextEditTextChanged() : m_currentNote is not valid";
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)

void NoteEditorLogic::rearrangeTasksInTextEditor(int startLinePosition, int endLinePosition,
                                                 int newLinePosition)
{
    QTextDocument *document = m_textEdit->document();
    QTextCursor cursor(document);
    cursor.setPosition(document->findBlockByNumber(startLinePosition).position());
    cursor.setPosition(document->findBlockByNumber(endLinePosition).position(),
                       QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    if (document->findBlockByNumber(endLinePosition + 1).isValid()) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }
    QString selectedText = cursor.selectedText();
    cursor.removeSelectedText();

    if (newLinePosition <= startLinePosition) {
        cursor.setPosition(document->findBlockByLineNumber(newLinePosition).position());
    } else {
        int newPositionBecauseOfRemoval =
                newLinePosition - (endLinePosition - startLinePosition + 1);
        if (newPositionBecauseOfRemoval == document->lineCount()) {
            cursor.setPosition(
                    document->findBlockByLineNumber(newPositionBecauseOfRemoval - 1).position());
            cursor.movePosition(QTextCursor::EndOfBlock);
            selectedText = "\n" + selectedText;
        } else {
            cursor.setPosition(
                    document->findBlockByLineNumber(newPositionBecauseOfRemoval).position());
        }
    }
    cursor.insertText(selectedText);

    checkForTasksInEditor();
}

void NoteEditorLogic::rearrangeColumnsInTextEditor(int startLinePosition, int endLinePosition,
                                                   int newLinePosition)
{
    QTextDocument *document = m_textEdit->document();
    QTextCursor cursor(document);
    cursor.setPosition(document->findBlockByNumber(startLinePosition).position());
    cursor.setPosition(document->findBlockByNumber(endLinePosition).position(),
                       QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    if (document->findBlockByNumber(endLinePosition + 1).isValid()) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }
    QString selectedText = cursor.selectedText();
    cursor.removeSelectedText();
    cursor.setPosition(document->findBlockByNumber(startLinePosition).position());
    if (document->findBlockByNumber(startLinePosition + 1).isValid()) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    }

    if (startLinePosition < newLinePosition) {
        // Goes down
        int newPositionBecauseOfRemoval =
                newLinePosition - (endLinePosition - startLinePosition + 1);
        cursor.setPosition(document->findBlockByLineNumber(newPositionBecauseOfRemoval).position());
        cursor.movePosition(QTextCursor::EndOfBlock);
        cursor.insertText("\n" + selectedText);
    } else {
        // Goes up
        cursor.setPosition(document->findBlockByLineNumber(newLinePosition).position());
        cursor.insertText(selectedText + "\n");
    }

    checkForTasksInEditor();
}

QMap<QString, int> NoteEditorLogic::getTaskDataInLine(const QString &line)
{
    QStringList taskExpressions = { "- [ ]", "- [x]", "* [ ]", "* [x]", "- [X]", "* [X]" };
    QMap<QString, int> taskMatchLineData;
    taskMatchLineData["taskMatchIndex"] = -1;

    int taskMatchIndex = -1;
    for (int j = 0; j < taskExpressions.size(); j++) {
        taskMatchIndex = line.indexOf(taskExpressions[j]);
        if (taskMatchIndex != -1) {
            taskMatchLineData["taskMatchIndex"] = taskMatchIndex;
            taskMatchLineData["taskExpressionSize"] = taskExpressions[j].size();
            taskMatchLineData["taskChecked"] = taskExpressions[j][3] == 'x' ? 1 : 0;
            return taskMatchLineData;
        }
    }

    return taskMatchLineData;
}

void NoteEditorLogic::checkTaskInLine(int lineNumber)
{
    QTextDocument *document = m_textEdit->document();
    QTextBlock block = document->findBlockByLineNumber(lineNumber);

    if (block.isValid()) {
        int indexOfTaskInLine = getTaskDataInLine(block.text())["taskMatchIndex"];
        if (indexOfTaskInLine == -1)
            return;
        QTextCursor cursor(block);
        cursor.setPosition(block.position() + indexOfTaskInLine + 3, QTextCursor::MoveAnchor);

        // Remove the old character and insert the new one
        cursor.deleteChar();
        cursor.insertText("x");
    }
}

void NoteEditorLogic::uncheckTaskInLine(int lineNumber)
{
    QTextDocument *document = m_textEdit->document();
    QTextBlock block = document->findBlockByLineNumber(lineNumber);

    if (block.isValid()) {
        int indexOfTaskInLine = getTaskDataInLine(block.text())["taskMatchIndex"];
        if (indexOfTaskInLine == -1)
            return;
        QTextCursor cursor(block);
        cursor.setPosition(block.position() + indexOfTaskInLine + 3, QTextCursor::MoveAnchor);

        // Remove the old character and insert the new one
        cursor.deleteChar();
        cursor.insertText(" ");
    }
}

void NoteEditorLogic::replaceTextBetweenLines(int startLinePosition, int endLinePosition,
                                              QString &newText)
{
    QTextDocument *document = m_textEdit->document();
    QTextBlock startBlock = document->findBlockByLineNumber(startLinePosition);
    QTextBlock endBlock = document->findBlockByLineNumber(endLinePosition);
    QTextCursor cursor(startBlock);
    cursor.setPosition(endBlock.position() + endBlock.length() - 1, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.insertText(newText);
}

void NoteEditorLogic::updateTaskText(int startLinePosition, int endLinePosition,
                                     const QString &newText)
{
    QTextDocument *document = m_textEdit->document();
    QTextBlock block = document->findBlockByLineNumber(startLinePosition);
    if (block.isValid()) {
        QMap<QString, int> taskData = getTaskDataInLine(block.text());
        int indexOfTaskInLine = taskData["taskMatchIndex"];
        if (indexOfTaskInLine == -1)
            return;
        QString taskExpressionText =
                block.text().mid(0, taskData["taskMatchIndex"] + taskData["taskExpressionSize"]);

        QString newTextModified = newText;
        newTextModified.replace("\n\n", "\n");
        newTextModified.replace("~~", "");
        QStringList taskExpressions = { "- [ ]", "- [x]", "* [ ]", "* [x]", "- [X]", "* [X]" };
        for (const auto &taskExpression : taskExpressions) {
            newTextModified.replace(taskExpression, "");
        }

        // We must allow hashtags solely for the first line, otherwise it will mess up
        // the parser - interprate the task's description as columns
        if (newTextModified.count('\n') > 1) {
            QStringList newTextModifiedSplitted = newTextModified.split('\n');

            if (newTextModifiedSplitted.size() > 1) {
                for (int i = 1; i < newTextModifiedSplitted.size(); i++) {
                    // Skipping the first line
                    newTextModifiedSplitted[i].replace("# ", "");
                    newTextModifiedSplitted[i].replace("#", "");
                }

                newTextModified = newTextModifiedSplitted.join('\n');
            }
        }

        QString newTaskText = taskExpressionText + " " + newTextModified;
        if (newTaskText.size() > 0 && newTaskText[newTaskText.size() - 1] == '\n') {
            newTaskText.remove(newTaskText.size() - 1, 1);
        }
        replaceTextBetweenLines(startLinePosition, endLinePosition, newTaskText);
        checkForTasksInEditor();
    }
}

void NoteEditorLogic::addNewTask(int startLinePosition, const QString newTaskText)
{
    QString newText = "\n- [ ] " + newTaskText;
    QTextDocument *document = m_textEdit->document();
    QTextBlock startBlock = document->findBlockByLineNumber(startLinePosition);

    if (!startBlock.isValid())
        return;

    QTextCursor cursor(startBlock);
    cursor.movePosition(QTextCursor::EndOfBlock);
    cursor.insertText(newText);

    checkForTasksInEditor();
}

void NoteEditorLogic::removeTextBetweenLines(int startLinePosition, int endLinePosition)
{
    if (startLinePosition < 0 || endLinePosition < startLinePosition) {
        return;
    }

    QTextDocument *document = m_textEdit->document();
    QTextCursor cursor(document);
    cursor.setPosition(document->findBlockByNumber(startLinePosition).position());
    cursor.setPosition(document->findBlockByNumber(endLinePosition).position(),
                       QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    if (document->findBlockByNumber(endLinePosition + 1).isValid()) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }
    cursor.removeSelectedText();
}

void NoteEditorLogic::removeTask(int startLinePosition, int endLinePosition)
{
    removeTextBetweenLines(startLinePosition, endLinePosition);
    checkForTasksInEditor();
}

void NoteEditorLogic::addNewColumn(int startLinePosition, const QString &columnTitle)
{
    if (startLinePosition < 0)
        return;

    QTextDocument *document = m_textEdit->document();
    QTextBlock block = document->findBlockByNumber(startLinePosition);

    if (block.isValid()) {
        QTextCursor cursor(block);
        if (startLinePosition == 0) {
            cursor.movePosition(QTextCursor::StartOfBlock);
        } else {
            cursor.movePosition(QTextCursor::EndOfBlock);
        }
        cursor.insertText(columnTitle);
        m_textEdit->setTextCursor(cursor);
    } else {
        m_textEdit->append(columnTitle);
    }

    checkForTasksInEditor();
}

void NoteEditorLogic::removeColumn(int startLinePosition, int endLinePosition)
{
    removeTextBetweenLines(startLinePosition, endLinePosition);

    if (startLinePosition < 0 || endLinePosition < startLinePosition)
        return;
    QTextDocument *document = m_textEdit->document();
    QTextCursor cursor(document);
    cursor.setPosition(document->findBlockByNumber(startLinePosition).position());
    if (cursor.block().isValid() && cursor.block().text().isEmpty()) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    }

    checkForTasksInEditor();
}

void NoteEditorLogic::updateColumnTitle(int lineNumber, const QString &newText)
{
    QTextDocument *document = m_textEdit->document();
    QTextBlock block = document->findBlockByLineNumber(lineNumber);

    if (block.isValid()) {
        // Header by hashtag
        int lastIndexOfHashTag = block.text().lastIndexOf("#");
        if (lastIndexOfHashTag != -1) {
            QTextCursor cursor(block);
            cursor.setPosition(block.position() + lastIndexOfHashTag + 1, QTextCursor::MoveAnchor);
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            cursor.setPosition(block.position() + lastIndexOfHashTag + 1);
            cursor.insertText(" " + newText);

        } else {
            int lastIndexofColon = block.text().lastIndexOf("::");
            if (lastIndexofColon != -1) {
                // Header by double colons
                QTextCursor cursor(block);
                cursor.setPosition(block.position(), QTextCursor::MoveAnchor);
                cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                cursor.removeSelectedText();
                cursor.setPosition(block.position());
                cursor.insertText(newText + "::");
            }
        }
    }
}

void NoteEditorLogic::addUntitledColumnToTextEditor(int startLinePosition)
{
    QString columnTitle = "# Untitled\n\n";
    QTextDocument *document = m_textEdit->document();
    QTextBlock block = document->findBlockByNumber(startLinePosition);

    if (block.isValid()) {
        QTextCursor cursor(block);
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.insertText(columnTitle);
    }
}

void NoteEditorLogic::appendNewColumn(QJsonArray &data, QJsonObject &currentColumn,
                                      QString &currentTitle, QJsonArray &tasks)
{
    if (!tasks.isEmpty()) {
        currentColumn["title"] = currentTitle;
        currentColumn["tasks"] = tasks;
        currentColumn["columnEndLine"] = tasks.last()["taskEndLine"];
        data.append(currentColumn);
        currentColumn = QJsonObject();
        tasks = QJsonArray();
    }
}

// Check if there are any tasks in the current note.
// If there are, sends the data to the kanban view.
// Structure:
// QJsonArray([
// {
//    "title":"TODO",
//    "columnStartLine": 1
//    "columnEndLine": 4
//    "tasks":[
//              {"checked":false,"text":"todo 1", "taskStartine": 3, "taskEndLine": 3},
//              {"checked":false,"text":"todo 2", "taskStartine": 4, "taskEndLine": 4}}]
// },
// ])
bool NoteEditorLogic::checkForTasksInEditor()
{
    QStringList lines = m_textEdit->toPlainText().split("\n");
    QJsonArray data;
    QJsonObject currentColumn;
    QJsonArray tasks;
    QString currentTitle = "";
    bool isPreviousLineATask = false;

    for (int i = 0; i < lines.size(); i++) {
        QString line = lines[i];
        QString lineTrimmed = line.trimmed();

        // Header title
        if (lineTrimmed.startsWith("#")) {
            if (!tasks.isEmpty() && currentTitle.isEmpty()) {
                // If we have only tasks without a header we insert one and call this function again
                addUntitledColumnToTextEditor(tasks.first()["taskStartLine"].toInt());
                return true;
            }
            appendNewColumn(data, currentColumn, currentTitle, tasks);
            currentColumn["columnStartLine"] = i;
            int countOfHashTags = lineTrimmed.count('#');
            currentTitle = lineTrimmed.mid(countOfHashTags);
            isPreviousLineATask = false;
        }
        // Non-header text with double colons
        else if (lineTrimmed.endsWith("::") && getTaskDataInLine(line)["taskMatchIndex"] == -1) {
            if (!tasks.isEmpty() && currentTitle.isEmpty()) {
                // If we have only tasks without a header we insert one and call this function again
                addUntitledColumnToTextEditor(tasks.first()["taskStartLine"].toInt());
                return true;
            }
            appendNewColumn(data, currentColumn, currentTitle, tasks);
            currentColumn["columnStartLine"] = i;
            QStringList parts = line.split("::");
            currentTitle = parts[0].trimmed();
            isPreviousLineATask = false;
        }
        // Todo item
        else {
            QMap<QString, int> taskDataInLine = getTaskDataInLine(line);
            int indexOfTaskInLine = taskDataInLine["taskMatchIndex"];

            if (indexOfTaskInLine != -1) {
                QJsonObject taskObject;
                QString taskText =
                        line.mid(indexOfTaskInLine + taskDataInLine["taskExpressionSize"])
                                .trimmed();
                taskObject["text"] = taskText;
                taskObject["checked"] = taskDataInLine["taskChecked"] == 1;
                taskObject["taskStartLine"] = i;
                taskObject["taskEndLine"] = i;
                tasks.append(taskObject);
                isPreviousLineATask = true;
            }
            // If it's a continues description of the task push current line's text to the last task
            else if (!line.isEmpty() && isPreviousLineATask) {
                if (tasks.size() > 0) {
                    QJsonObject newTask = tasks[tasks.size() - 1].toObject();
                    QString newTaskText = newTask["text"].toString() + "  \n"
                            + lineTrimmed; // For markdown rendering a line break needs two white
                                           // spaces
                    newTask["text"] = newTaskText;
                    newTask["taskEndLine"] = i;
                    tasks[tasks.size() - 1] = newTask;
                }
            } else {
                isPreviousLineATask = false;
            }
        }
    }

    if (!tasks.isEmpty() && currentTitle.isEmpty()) {
        // If we have only tasks without a header we insert one and call this function again
        addUntitledColumnToTextEditor(tasks.first()["taskStartLine"].toInt());
        return true;
    }

    appendNewColumn(data, currentColumn, currentTitle, tasks);

    emit tasksFoundInEditor(QVariant(data));

    return false;
}
#endif

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

void NoteEditorLogic::setTheme(Theme::Value theme, QColor textColor, qreal fontSize)
{
    m_tagListDelegate->setTheme(theme);
    m_highlighter->setTheme(theme, textColor, fontSize);
    switch (theme) {
    case Theme::Light: {
        m_spacerColor = QColor(191, 191, 191);
        break;
    }
    case Theme::Dark: {
        m_spacerColor = QColor(212, 212, 212);
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

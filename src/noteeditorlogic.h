#ifndef NOTEEDITORLOGIC_H
#define NOTEEDITORLOGIC_H

#include <QObject>
#include <QTimer>
#include <QColor>
#include <QVector>
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#  include <QWidget>
#  include <QVariant>
#  include <QJsonArray>
#  include <QJsonObject>
#  include <QRegularExpression>
#endif

#include "nodedata.h"
#include "editorsettingsoptions.h"

class CustomDocument;
class CustomMarkdownHighlighter;
class QLabel;
class QLineEdit;
class DBManager;
class TagListView;
class TagListModel;
class TagPool;
class TagListDelegate;
class QListWidget;
class NoteEditorLogic : public QObject
{
    Q_OBJECT
public:
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    explicit NoteEditorLogic(CustomDocument *textEdit, QLabel *editorDateLabel,
                             QLineEdit *searchEdit, QWidget *kanbanWidget, TagListView *tagListView,
                             TagPool *tagPool, DBManager *dbManager, QObject *parent = nullptr);

#else
    explicit NoteEditorLogic(CustomDocument *textEdit, QLabel *editorDateLabel,
                             QLineEdit *searchEdit, TagListView *tagListView, TagPool *tagPool,
                             DBManager *dbManager, QObject *parent = nullptr);
#endif

    bool markdownEnabled() const;
    void setMarkdownEnabled(bool enabled);
    static QString getNoteDateEditor(const QString &dateEdited);
    void highlightSearch() const;
    bool isTempNote() const;
    void saveNoteToDB();
    int currentEditingNoteId() const;
    void deleteCurrentNote();

    static QString getFirstLine(const QString &str);
    static QString getSecondLine(const QString &str);
    void setTheme(Theme::Value theme, QColor textColor, qreal fontSize);

    int currentAdaptableEditorPadding() const;
    void setCurrentAdaptableEditorPadding(int newCurrentAdaptableEditorPadding);

    int currentMinimumEditorPadding() const;
    void setCurrentMinimumEditorPadding(int newCurrentMinimumEditorPadding);

public slots:
    void showNotesInEditor(const QVector<NodeData> &notes);
    void onTextEditTextChanged();
    void closeEditor();
    void onNoteTagListChanged(int noteId, const QSet<int> &tagIds);
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    bool checkForTasksInEditor();
    void rearrangeTasksInTextEditor(int startLinePosition, int endLinePosition,
                                    int newLinePosition);
    void rearrangeColumnsInTextEditor(int startLinePosition, int endLinePosition,
                                      int newLinePosition);
    void checkTaskInLine(int lineNumber);
    void uncheckTaskInLine(int lineNumber);
    void updateTaskText(int startLinePosition, int endLinePosition, const QString &newText);
    void addNewTask(int startLinePosition, const QString newTaskText);
    void removeTask(int startLinePosition, int endLinePosition);
    void addNewColumn(int startLinePosition, const QString &columnTitle);
    void removeColumn(int startLinePosition, int endLinePosition);
    void updateColumnTitle(int lineNumber, const QString &newText);
#endif
signals:
    void requestCreateUpdateNote(const NodeData &note);
    void noteEditClosed(const NodeData &note, bool selectNext);
    void setVisibilityOfFrameRightWidgets(bool);
    void setVisibilityOfFrameRightNonEditor(bool);
    void moveNoteToListViewTop(const NodeData &note);
    void updateNoteDataInList(const NodeData &note);
    void deleteNoteRequested(const NodeData &note);
    void showKanbanView();
    void hideKanbanView();
    void textShown();
    void kanbanShown();
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    void tasksFoundInEditor(QVariant data);
    void clearKanbanModel();
    void resetKanbanSettings();
    void checkMultipleNotesSelected(QVariant isMultipleNotesSelected);
#endif

private:
    static QDateTime getQDateTime(const QString &date);
    void showTagListForCurrentNote();
    bool isInEditMode() const;
    QString moveTextToNewLinePosition(const QString &inputText, int startLinePosition,
                                      int endLinePosition, int newLinePosition,
                                      bool isColumns = false);
    QMap<QString, int> getTaskDataInLine(const QString &line);
    void replaceTextBetweenLines(int startLinePosition, int endLinePosition, QString &newText);
    void removeTextBetweenLines(int startLinePosition, int endLinePosition);
    void appendNewColumn(QJsonArray &data, QJsonObject &currentColumn, QString &currentTitle,
                         QJsonArray &tasks);
    void addUntitledColumnToTextEditor(int startLinePosition);

private:
    CustomDocument *m_textEdit;
    CustomMarkdownHighlighter *m_highlighter;
    QLabel *m_editorDateLabel;
    QLineEdit *m_searchEdit;
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    QWidget *m_kanbanWidget;
#endif
    TagListView *m_tagListView;
    DBManager *m_dbManager;
    QVector<NodeData> m_currentNotes;
    bool m_isContentModified;
    QTimer m_autoSaveTimer;
    TagListDelegate *m_tagListDelegate;
    TagListModel *m_tagListModel;
    QColor m_spacerColor;
    int m_currentAdaptableEditorPadding;
    int m_currentMinimumEditorPadding;
};

#endif // NOTEEDITORLOGIC_H

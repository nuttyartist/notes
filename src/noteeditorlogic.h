#ifndef NOTEEDITORLOGIC_H
#define NOTEEDITORLOGIC_H

#include <QObject>
#include <QTimer>
#include "nodedata.h"
#include "styleeditorwindow.h"

class CustomDocument;
class MarkdownHighlighter;
class QLabel;
class QLineEdit;
class DBManager;
class TagListView;
class TagListModel;
class TagPool;
class TagListDelegate;

class NoteEditorLogic : public QObject
{
    Q_OBJECT
public:
    explicit NoteEditorLogic(CustomDocument* textEdit,
                             QLabel* editorDateLabel,
                             QLineEdit* searchEdit,
                             TagListView* tagListView,
                             TagPool* tagPool,
                             DBManager* dbManager,
                             QObject *parent = nullptr);

    bool markdownEnabled() const;
    void setMarkdownEnabled(bool newMarkdownEnabled);
    static QString getNoteDateEditor(QString dateEdited);
    void highlightSearch() const;
    bool isTempNote() const;
    void saveNoteToDB();
    NodeData currentEditingNote() const;
    void deleteCurrentNote();

    static QString getFirstLine(const QString &str);
    static QString getSecondLine(const QString &str);
    void setTheme(Theme newTheme);

public slots:
    void showNoteInEditor(const NodeData& note);
    void onTextEditTextChanged();
    void closeEditor();
    void onNoteTagListChanged(int noteId, const QSet<int> tagIds);

signals:
    void requestCreateUpdateNote(const NodeData& note);
    void noteEditClosed(const NodeData& note);
    void setVisibilityOfFrameRightNonEditor(bool);
    void moveNoteToListViewTop(const NodeData& note);
    void updateNoteDataInList(const NodeData& note);
    void deleteNoteRequested(const NodeData& note);
private:
    static QDateTime getQDateTime(QString date);
    void showTagListForCurrentNote();

private:
    CustomDocument* m_textEdit;
    MarkdownHighlighter *m_highlighter;
    QLabel* m_editorDateLabel;
    QLineEdit* m_searchEdit;
    TagListView* m_tagListView;
    DBManager* m_dbManager;
    NodeData m_currentNote;
    bool m_isContentModified;
    QTimer m_autoSaveTimer;
    TagListDelegate* m_tagListDelegate;
    TagListModel* m_tagListModel;
};

#endif // NOTEEDITORLOGIC_H

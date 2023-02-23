#ifndef NOTEEDITORLOGIC_H
#define NOTEEDITORLOGIC_H

#include <QObject>
#include <QTimer>
#include "nodedata.h"
#include "styleeditorwindow.h"

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
    explicit NoteEditorLogic(CustomDocument *textEdit, QLabel *editorDateLabel,
                             QLineEdit *searchEdit, TagListView *tagListView, TagPool *tagPool,
                             DBManager *dbManager, QObject *parent = nullptr);

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
    void setTheme(Theme theme, QColor textColor);

    int currentAdaptableEditorPadding() const;
    void setCurrentAdaptableEditorPadding(int newCurrentAdaptableEditorPadding);

    int currentMinimumEditorPadding() const;
    void setCurrentMinimumEditorPadding(int newCurrentMinimumEditorPadding);

public slots:
    void showNotesInEditor(const QVector<NodeData> &notes);
    void onTextEditTextChanged();
    void closeEditor();
    void onNoteTagListChanged(int noteId, const QSet<int> &tagIds);
signals:
    void requestCreateUpdateNote(const NodeData &note);
    void noteEditClosed(const NodeData &note, bool selectNext);
    void setVisibilityOfFrameRightNonEditor(bool);
    void moveNoteToListViewTop(const NodeData &note);
    void updateNoteDataInList(const NodeData &note);
    void deleteNoteRequested(const NodeData &note);

private:
    static QDateTime getQDateTime(const QString &date);
    void showTagListForCurrentNote();
    bool isInEditMode() const;

private:
    CustomDocument *m_textEdit;
    CustomMarkdownHighlighter *m_highlighter;
    QLabel *m_editorDateLabel;
    QLineEdit *m_searchEdit;
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

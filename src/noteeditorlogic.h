#ifndef NOTEEDITORLOGIC_H
#define NOTEEDITORLOGIC_H

#include <QObject>
#include <QTimer>
#include "nodedata.h"

class CustomDocument;
class MarkdownHighlighter;
class QLabel;
class QLineEdit;
class DBManager;

class NoteEditorLogic : public QObject
{
    Q_OBJECT
public:
    explicit NoteEditorLogic(CustomDocument* textEdit,
                             QLabel* editorDateLabel,
                             QLineEdit* searchEdit,
                             DBManager* dbManager,
                             QObject *parent = nullptr);

    bool markdownEnabled() const;
    void setMarkdownEnabled(bool newMarkdownEnabled);
    static QString getNoteDateEditor(QString dateEdited);
    void highlightSearch() const;

    bool isTempNote() const;
    void setIsTempNote(bool newIsTempNote);
    void saveNoteToDB();

    static QString getFirstLine(const QString &str);
public slots:
    void showNoteInEditor(const NodeData& note);
    void onTextEditTextChanged();
signals:
    void requestCreateUpdateNote(const NodeData& note);
    void setVisibilityOfFrameRightNonEditor(bool);
    void moveNoteToListViewTop(const NodeData& note);
    void updateNoteDataInList(const NodeData& note);

private:
    static QDateTime getQDateTime(QString date);

private:
    CustomDocument* m_textEdit;
    MarkdownHighlighter *m_highlighter;
    QLabel* m_editorDateLabel;
    QLineEdit* m_searchEdit;
    DBManager* m_dbManager;
    NodeData m_currentNote;
    bool m_isTempNote;
    bool m_isContentModified;
    QTimer m_autoSaveTimer;
};

#endif // NOTEEDITORLOGIC_H

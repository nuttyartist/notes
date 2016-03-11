/*********************************************************************************************
* Mozila License
* Just a meantime project to see the ability of qt, the framework that my OS might be based on
* And for those linux users that beleive in the power of notes
*********************************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtCore>
#include <QGroupBox>
#include <QPushButton>
#include <vector>
#include <QToolButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QSettings>
#include "notedata.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void paintEvent(QPaintEvent* e);
    void SetUpMainWindow ();
    void SetUpKeyboardShortcuts ();
    void SetUpNewNoteButtonAndTrahButton();
    void SetUpEditorDateLabel();
    void SetUpSplitter();
    void SetUpLine ();
    void SetUpFrame ();
    void SetUpTitleBarButtons ();
    void CreateClearButton ();
    void CreateMagnifyingGlassIcon ();
    void SetUpLineEdit ();
    void SetUpScrollArea ();
    void SetUpTextEdit ();
    void InitializeVariables();
    void InitializeSettingsDatabase();
    void SetUpDatabases ();
    void SetLayoutForScrollArea ();
    void RestoreStates();
    QString GetFirstLine (const QString& str);
    QString GetElidedText (QString str, QFontMetrics labelFontMetrics, int size);
    QString GetFirstLineAndElide (NoteData* note);
    QDateTime GetQDateTime (QString date);
    QString GetNoteDateEditor (QString dateEdited);
    QString GetNoteDate (QString dateEdited);
    NoteData* AddNote (QString noteName, bool isLoadingOrNew);
    void SortNotesList (QStringList &stringNotesList);
    void LoadNotes ();
    void SelectFirstNote ();
    void CreateNewNoteIfEmpty();
    void on_newNoteButton_clicked();
    void on_trashButton_clicked();
    void UnhighlightNote (NoteData* note);
    void HighlightNote (NoteData* note, QString rgbStringColor);
    QPropertyAnimation* GetAnimationForDeletion (NoteData* note);
    void note_buttuon_pressed ();
    void ClearButtonClicked ();
    void DeleteNoteFromDataBase (NoteData* note);
    void DeleteNoteFromVisual (NoteData* note);
    void on_textEdit_textChanged ();
    bool IsFound (QString keyword, QString content);
    void SimpleUnhighlightNote (NoteData* note);
    void ClearAllNotesFromVisual ();
    bool GoToAndSelectNote (QString noteName);
    void on_lineEdit_textChanged (const QString& arg1);
    QString CreateNewNoteInDatabase ();
    void NewNoteAnimation ();
    void Create_new_note ();
    void DeleteSelectedNoteFromVisual ();
    void DeleteTempNoteFromVisual ();
    void DeleteSelectedNote ();
    void SetFocusOnText ();
    void SetFocusOnScrollArea ();
    void SelectNoteUp ();
    void SelectNoteDown ();
    void FullscreenWindow ();
    void MaximizeWindow ();
    void MinimizeWindow ();
    void QuitApplication ();
    void on_greenMaximizeButton_pressed ();
    void on_yellowMinimizeButton_pressed ();
    void on_redCloseButton_pressed ();
    void on_greenMaximizeButton_clicked();
    void on_yellowMinimizeButton_clicked();
    void on_redCloseButton_clicked();
    void ScrollAreaScrollBarRangeChange (int, int verticalScrollBarRange);
    void TextEditScrollBarRangeChange (int, int verticalScrollBarRange);
    void TextEditScrollBarValueChange (int verticalScrollBarValue);
    void closeEvent(QCloseEvent* event);
    void mousePressEvent (QMouseEvent* event);
    void mouseMoveEvent (QMouseEvent* event);
    void mouseReleaseEvent (QMouseEvent* event);
    bool IsClickingButton (QPoint mousePos, QPushButton* button);
    void mouseDoubleClickEvent (QMouseEvent* e);
    void resizeEvent (QResizeEvent *);
    bool IsSpacerInsideLayout(QSpacerItem *spacer, QVBoxLayout* layout);
    void ResizeRestWhenSplitterMove(int pos, int index);
    bool eventFilter (QObject* object, QEvent* event);

private:

    Ui::MainWindow* ui;

    QSettings* m_notesDatabase;
    QSettings* m_trashDatabase;
    QSettings* m_settingsDatabase;

    struct noteDataForSorting
    {
        QString m_noteName;
        QDateTime m_dateTime;

        bool operator <(const noteDataForSorting& noteToCompare) const
        {
            return this->m_dateTime < noteToCompare.m_dateTime;
        }
    };
    QList<noteDataForSorting> m_notesDataForSorting;

    std::vector<NoteData*> m_allNotesList; // All the notes stored in the database
    std::vector<NoteData*> m_visibleNotesList; // Notes currently displayed inside scrollArea

    QVBoxLayout* m_lay;
    QToolButton* m_clearButton;
    QFrame* frame;

    NoteData* m_tempNote;
    NoteData* m_currentSelectedNote;
    NoteData* m_currentHoveredNote;
    QString m_noteOnTopInTheLayoutName;
    QString m_tempSelectedNoteBeforeSearchingName;
    int m_currentVerticalScrollAreaRange;
    bool m_isTemp;

    int m_nMouseClick_X_Coordinate;
    int m_nMouseClick_Y_Coordinate;
    int m_textEditLeftPadding;
    bool m_canMoveWindow;
    bool m_focusBreaker;
};

#endif // MAINWINDOW_H

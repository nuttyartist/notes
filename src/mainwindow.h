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

    QList<NoteData*> m_allNotesList; // All the notes stored in the database
    QList<NoteData*> m_visibleNotesList; // Notes currently displayed inside scrollArea

    QVBoxLayout* m_lay;
    QToolButton* m_clearButton;
    QFrame* frame;

    NoteData* m_tempNote;
    NoteData* m_currentSelectedNote;
    NoteData* m_currentHoveredNote;
    QString m_noteOnTopInTheLayoutName;
    QString m_tempSelectedNoteBeforeSearchingName;
    int m_currentVerticalScrollAreaRange;
    int m_nMouseClick_X_Coordinate;
    int m_nMouseClick_Y_Coordinate;
    int m_textEditLeftPadding;
    bool m_canMoveWindow;
    bool m_focusBreaker;
    bool m_isTemp;

    void setupMainWindow();
    void setupKeyboardShortcuts();
    void setupNewNoteButtonAndTrahButton();
    void setupEditorDateLabel();
    void setupSplitter();
    void setupLine();
    void setupFrame();
    void setupTitleBarButtons();
    void setupSignalsSlots();
    void setupLineEdit();
    void setupScrollArea();
    void setupTextEdit();
    void setupDatabases();
    void initializeVariables();
    void initializeSettingsDatabase();
    void createClearButton();
    void createMagnifyingGlassIcon();
    void createNewNoteIfEmpty();
    QString createNewNoteInDatabase();
    void deleteNoteFromDataBase(NoteData* note);
    void deleteNoteFromVisual(NoteData* note);
    void newNoteAnimation();
    void setLayoutForScrollArea();
    void restoreStates();
    QString getFirstLine(const QString& str);
    QString getElidedText(QString str, QFontMetrics labelFontMetrics, int size);
    QString getFirstLineAndElide (NoteData* note);
    QString getNoteDateEditor (QString dateEdited);
    QString getNoteDate(QString dateEdited);
    NoteData* addNote(QString noteName, bool isLoadingOrNew);
    QDateTime getQDateTime(QString date);
    QPropertyAnimation* getAnimationForDeletion(NoteData* note);
    void showNoteInEditor(NoteData* note);
    void sortNotesList(QStringList &stringNotesList);
    void loadNotes();
    void selectFirstNote();
    void simpleUnhighlightNote(NoteData* note);
    void unhighlightNote(NoteData* note);
    void highlightNote(NoteData* note, QString rgbStringColor);
    void clearAllNotesFromVisual();
    bool goToAndSelectNote(QString noteName);
    bool isFound(QString keyword, QString content);
    bool isClickingButton(QPoint mousePos, QPushButton* button);
    bool isSpacerInsideLayout(QSpacerItem *spacer, QVBoxLayout* layout);
    void paintEvent(QPaintEvent* e);
    void closeEvent(QCloseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* e);
    bool eventFilter(QObject* object, QEvent* event);
    void resizeEvent(QResizeEvent *);

private slots:
    void onNewNoteButtonClicked();
    void onTrashButtonClicked();
    void onNoteClicked();
    void onTextEditTextChanged();
    void onLineEditTextChanged(const QString& arg1);
    void onGreenMaximizeButtonPressed ();
    void onYellowMinimizeButtonPressed ();
    void onRedCloseButtonPressed ();
    void onGreenMaximizeButtonClicked();
    void onYellowMinimizeButtonClicked();
    void onRedCloseButtonClicked();
    void createNewNote();
    void deleteSelectedNote();
    void deleteSelectedNoteFromVisual();
    void deleteTempNoteFromVisual();
    void setFocusOnScrollArea();
    void selectNoteDown();
    void selectNoteUp();
    void setFocusOnText();
    void fullscreenWindow();
    void maximizeWindow();
    void minimizeWindow();
    void QuitApplication();
    void resizeRestWhenSplitterMove(int pos, int index);
    void clearButtonClicked();
    void scrollAreaScrollBarRangeChange(int, int verticalScrollBarRange);
    void textEditScrollBarRangeChange(int, int verticalScrollBarRange);
    void textEditScrollBarValueChange(int verticalScrollBarValue);
};

#endif // MAINWINDOW_H

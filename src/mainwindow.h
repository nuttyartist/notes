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
    void setUpMainWindow();
    void setUpKeyboardShortcuts();
    void setUpNewNoteButtonAndTrahButton();
    void setUpEditorDateLabel();
    void setUpSplitter();
    void setUpLine();
    void setUpFrame();
    void setUpTitleBarButtons();
    void createClearButton();
    void createMagnifyingGlassIcon();
    void setUpLineEdit();
    void setUpScrollArea();
    void setUpTextEdit();
    void initializeVariables();
    void initializeSettingsDatabase();
    void setUpDatabases();
    void setLayoutForScrollArea();
    void restoreStates();
    QString getFirstLine(const QString& str);
    QString getElidedText(QString str, QFontMetrics labelFontMetrics, int size);
    QString getFirstLineAndElide (NoteData* note);
    QDateTime getQDateTime(QString date);
    QString getNoteDateEditor (QString dateEdited);
    QString getNoteDate(QString dateEdited);
    NoteData* addNote(QString noteName, bool isLoadingOrNew);
    void showNoteInEditor(NoteData* note);
    void sortNotesList(QStringList &stringNotesList);
    void loadNotes();
    void selectFirstNote();
    void createNewNoteIfEmpty();
    void onNewNoteButtonClicked();
    void onTrashButtonClicked();
    void unhighlightNote(NoteData* note);
    void highlightNote(NoteData* note, QString rgbStringColor);
    QPropertyAnimation* getAnimationForDeletion(NoteData* note);
    void onNoteClicked();
    void clearButtonClicked();
    void deleteNoteFromDataBase(NoteData* note);
    void deleteNoteFromVisual(NoteData* note);
    void onTextEditTextChanged();
    bool isFound(QString keyword, QString content);
    void simpleUnhighlightNote(NoteData* note);
    void clearAllNotesFromVisual();
    bool goToAndSelectNote(QString noteName);
    void onLineEditTextChanged(const QString& arg1);
    QString createNewNoteInDatabase();
    void newNoteAnimation();
    void createNewNote();
    void deleteSelectedNoteFromVisual();
    void deleteTempNoteFromVisual();
    void deleteSelectedNote();
    void setFocusOnText();
    void setFocusOnScrollArea();
    void selectNoteUp();
    void selectNoteDown();
    void fullscreenWindow();
    void maximizeWindow();
    void minimizeWindow();
    void QuitApplication();
    void onGreenMaximizeButtonPressed ();
    void onYellowMinimizeButtonPressed ();
    void onRedCloseButtonPressed ();
    void onGreenMaximizeButtonClicked();
    void onYellowMinimizeButtonClicked();
    void onRedCloseButtonClicked();
    void scrollAreaScrollBarRangeChange(int, int verticalScrollBarRange);
    void textEditScrollBarRangeChange(int, int verticalScrollBarRange);
    void textEditScrollBarValueChange(int verticalScrollBarValue);
    void closeEvent(QCloseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    bool isClickingButton(QPoint mousePos, QPushButton* button);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void resizeEvent(QResizeEvent *);
    bool isSpacerInsideLayout(QSpacerItem *spacer, QVBoxLayout* layout);
    void resizeRestWhenSplitterMove(int pos, int index);
    bool eventFilter(QObject* object, QEvent* event);

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

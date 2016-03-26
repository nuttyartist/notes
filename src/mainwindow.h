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

protected:
    void paintEvent(QPaintEvent* e) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent*) Q_DECL_OVERRIDE;
    void resizeWindow(QMouseEvent* event);
    bool eventFilter(QObject* object, QEvent* event) Q_DECL_OVERRIDE;

private:

    Ui::MainWindow* ui;

    QSettings* m_notesDatabase;
    QSettings* m_trashDatabase;
    QSettings* m_settingsDatabase;
    QList<NoteData*> m_allNotesList; // All the notes stored in the database
    QList<NoteData*> m_visibleNotesList; // Notes currently displayed inside scrollArea
    QVBoxLayout* m_noteWidgetsContainer;
    QToolButton* m_clearButton;

    NoteData* m_tempNote;
    NoteData* m_currentSelectedNote;
    NoteData* m_currentHoveredNote;
    NoteData* m_selectedNoteBeforeSearching;
    NoteData* m_noteOnTopInTheLayout;
    int m_currentVerticalScrollAreaRange;
    int m_mousePressX;
    int m_mousePressY;
    int m_textEditLeftPadding;
    bool m_canBeResized;
    bool m_resizeHorzTop;
    bool m_resizeHorzBottom;
    bool m_resizeVertRight;
    bool m_resizeVertLeft;
    bool m_canMoveWindow;
    bool m_focusBreaker;
    bool m_isTemp;

    void setupMainWindow();
    void setupKeyboardShortcuts();
    void setupNewNoteButtonAndTrahButton();
    void setupEditorDateLabel();
    void setupSplitter();
    void setupLine();
    void setupRightFrame();
    void setupTitleBarButtons();
    void setupSignalsSlots();
    void setupLineEdit();
    void setupScrollArea();
    void setupTextEdit();
    void setupDatabases();
    void initializeSettingsDatabase();
    void createNewNoteIfEmpty();
    QString createNewNoteInDatabase();
    void deleteNoteFromDataBase(NoteData* note);
    void setLayoutForScrollArea();
    void restoreStates();
    QString getFirstLine(const QString& str);
    QString getNoteDateEditor (QString dateEdited);
    NoteData* generateNote(QString noteName, bool isLoadingOrNew);
    QDateTime getQDateTime(QString date);
    QPropertyAnimation* getAnimationForDeletion(NoteData* note);
    void showNoteInEditor(NoteData* note);
    void sortNotesList(QStringList &stringNotesList);
    void loadNotes();
    void saveCurrentNoteToDB();
    void selectFirstNote();
    void clearAllNotesFromVisual();
    bool goToAndSelectNote(NoteData *note);
    QPropertyAnimation* createAnimation(NoteData* note, const QPair<int, int> &start, const QPair<int, int> &end, int duration);
    void moveNoteToTop();
    void moveNoteToTopWithAnimation();
    void clearSearch(NoteData *previousNote);
    void findNotesContain(const QString &keyword);
    void selectNote(NoteData* note);

private slots:
    void InitData();
    void onNewNoteButtonClicked();
    void onTrashButtonClicked();
    void onNotePressed();
    void onNoteHoverEntered();
    void onNoteHoverLeft();
    void onTextEditTextChanged();
    void onLineEditTextChanged(const QString& keyword);
    void onGreenMaximizeButtonPressed ();
    void onYellowMinimizeButtonPressed ();
    void onRedCloseButtonPressed ();
    void onGreenMaximizeButtonClicked();
    void onYellowMinimizeButtonClicked();
    void onRedCloseButtonClicked();
    void createNewNote();
    void createNewNoteWithAnimation();
    void deleteNote(NoteData *note, bool isFromUser=true);
    void deleteNoteWithAnimation(NoteData *note, bool isFromUser=true);
    void deleteSelectedNote();
    void setFocusOnCurrentNote();
    void selectNoteDown();
    void selectNoteUp();
    void setFocusOnText();
    void fullscreenWindow();
    void maximizeWindow();
    void minimizeWindow();
    void QuitApplication();
};

#endif // MAINWINDOW_H

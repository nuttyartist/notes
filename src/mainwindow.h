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
#include <QTextEdit>
#include <QSettings>
#include "notedata.h"
#include "notemodel.h"
#include "noteview.h"

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

    QTimer* m_autoSaveTimer;
    QSettings* m_notesDatabase;
    QSettings* m_trashDatabase;
    QSettings* m_settingsDatabase;
    QVBoxLayout* m_noteWidgetsContainer;
    QToolButton* m_clearButton;
    QPushButton* m_greenMaximizeButton;
    QPushButton* m_redCloseButton;
    QPushButton* m_yellowMinimizeButton;
    QPushButton* m_newNoteButton;
    QPushButton* m_trashButton;
    QTextEdit* m_textEdit;
    QLineEdit* m_lineEdit;
    QLabel* m_editorDateLabel;


    NoteView* m_noteView;
    NoteModel* m_noteModel;
    NoteModel* m_deletedNotesModel;
    QSortFilterProxyModel* m_proxyModel;
    QModelIndex m_currentSelectedNoteProxy;
    QModelIndex m_selectedNoteBeforeSearching;

    int m_currentVerticalScrollAreaRange;
    int m_mousePressX;
    int m_mousePressY;
    int m_textEditLeftPadding;
    int m_noteCounter;
    int m_trashCounter;
    bool m_canBeResized;
    bool m_resizeHorzTop;
    bool m_resizeHorzBottom;
    bool m_resizeVertRight;
    bool m_resizeVertLeft;
    bool m_canMoveWindow;
    bool m_focusBreaker;
    bool m_isTemp;
    bool m_isListViewScrollBarHidden;
    bool m_isContentModified;
    bool m_isOperationRunning;

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
    void setupTextEdit();
    void setupDatabases();
    void setupModelView();
    void initializeSettingsDatabase();
    void createNewNoteIfEmpty();
    QString createNewNoteInDatabase();
    void deleteNoteFromDataBase(const QModelIndex& noteIndex);
    void setLayoutForScrollArea();
    void restoreStates();
    QString getFirstLine(const QString& str);
    QString getNoteDateEditor (QString dateEdited);
    NoteData *generateNote(QString noteName, bool isLoadingOrNew);
    QDateTime getQDateTime(QString date);
    void showNoteInEditor(const QModelIndex& noteIndex);
    void sortNotesList(QStringList &stringNotesList);
    void loadNotes();
    void saveNoteToDB(const QModelIndex& noteIndex);
    void selectFirstNote();
    bool goToAndSelectNote(const QModelIndex& noteIndex);
    void moveNoteToTop();
    void clearSearch(const QModelIndex &previousNote);
    void findNotesContain(const QString &keyword);
    void selectNote(const QModelIndex& noteIndex);

private slots:
    void InitData();
    void onNewNoteButtonClicked();
    void onTrashButtonClicked();
    void onNotePressed(const QModelIndex &index);
    void onTextEditTextChanged();
    void onLineEditTextChanged(const QString& keyword);
    void onGreenMaximizeButtonPressed ();
    void onYellowMinimizeButtonPressed ();
    void onRedCloseButtonPressed ();
    void onGreenMaximizeButtonClicked();
    void onYellowMinimizeButtonClicked();
    void onRedCloseButtonClicked();
    void createNewNote();
    void deleteNote(const QModelIndex& noteIndex, bool isFromUser=true);
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

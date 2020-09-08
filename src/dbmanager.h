#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "notedata.h"
#include <QObject>
#include <QtSql/QSqlDatabase>

class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(QObject *parent = Q_NULLPTR);

private:

    void open(const QString& path, bool doCreate = false);
    void createTables();
    int  getLastRowID();
    bool forceLastRowIndexValue(const int indexValue);

    NoteData* getNote(QString id);
    bool isNoteExist(NoteData* note);

    QList<NoteData *> getAllNotes();
    bool addNote(NoteData* note);
    bool removeNote(NoteData* note);
    bool permanantlyRemoveAllNotes();
    bool updateNote(NoteData* note);
    bool migrateNote(NoteData* note);
    bool migrateTrash(NoteData* note);

signals:
    void notesReceived(QList<NoteData*> noteList, int noteCounter);

public slots:

    void onNotesListRequested();
    void onOpenDBManagerRequested(QString path, bool doCreate);
    void onCreateUpdateRequested(NoteData* note);
    void onDeleteNoteRequested(NoteData* note);
    void onImportNotesRequested(QList<NoteData *> noteList);
    void onRestoreNotesRequested(QList<NoteData *> noteList);
    void onExportNotesRequested(QString fileName);
    void onMigrateNotesRequested(QList<NoteData *> noteList);
    void onMigrateTrashRequested(QList<NoteData *> noteList);
    void onForceLastRowIndexValueRequested(int index);
};

#endif // DBMANAGER_H

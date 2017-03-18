#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "notedata.h"

#include <QObject>
#include <QtSql/QSqlDatabase>

class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(const QString& path, bool doCreate = false, QObject *parent = 0);

    bool isNoteExist(NoteData* note);

private:
    QSqlDatabase m_db;
    bool isDuplicate(NoteData* note);
    bool titleAndIDMatch(NoteData* note);

signals:
    void notesReceived(QList<NoteData*> noteList);

public slots:
    QList<NoteData*> getAllNotes();
    bool addNote(NoteData* note);
    bool removeNote(NoteData* note);
    bool modifyNote(NoteData* note);
    bool migrateNote(NoteData* note);
    bool migrateTrash(NoteData* note);
    int getLastRowID();
    QList<NoteExport> getBackup();
    void restore(QList<NoteExport> noteList);
};

#endif // DBMANAGER_H

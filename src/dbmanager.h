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
    QList<NoteExport> exportNotes();
    void importNote(const NoteExport& noteExport);

private:
    QSqlDatabase m_db;
    NoteData* getNote(QString id);
    NoteExport buildNote(QSqlQuery query);
    bool addImportedNote(NoteExport* note);
    bool updateImportedNote(NoteExport* note);
    NoteExport* getNoteExport(QString id);

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
};

#endif // DBMANAGER_H

#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "notedata.h"

#include <QObject>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQueryModel>

class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(const QString& path, QObject *parent = 0);

private:
    QSqlDatabase m_db;


signals:
    void notesReceived(QList<NoteData*> noteList);

public slots:
    void getAllNotes();
    void addNote(NoteData* note);
    void removeNote(NoteData* note);
    void modifyNote(NoteData* note);

};

#endif // DBMANAGER_H

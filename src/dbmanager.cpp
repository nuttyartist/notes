#include "dbmanager.h"

DBManager::DBManager(const QString& path, QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QList<NoteData*> >("QList<NoteData*>");

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open()){
        qDebug() << "Error: connection with database fail";
    }else{
        qDebug() << "Database: connection ok";
    }
}

void DBManager::getAllNotes()
{
    QList<NoteData *> noteList;

    QSqlQuery query;
    query.prepare("SELECT * FROM active_notes");
    bool status = query.exec();
    if(status){
        while(query.next()){
            NoteData* note = new NoteData;
            int id =  query.value(0).toInt();
            int epochDateTimeCreation = query.value(1).toInt();
            QDateTime dateTimeCreation = QDateTime::fromMSecsSinceEpoch(epochDateTimeCreation);
            int epochDateTimeModification= query.value(2).toInt();
            QDateTime dateTimeModification = QDateTime::fromMSecsSinceEpoch(epochDateTimeModification);
            QString content = query.value(4).toString();
            QString fullTitle = query.value(5).toString();

            note->setId(QString::number(id));
            note->setCreationDateTime(dateTimeCreation);
            note->setLastModificationDateTime(dateTimeModification);
            note->setContent(content);
            note->setFullTitle(fullTitle);

            noteList.push_back(note);
        }

        emit notesReceived(noteList);
    }
}

void DBManager::addNote(NoteData* note)
{
    QSqlQuery query;

    int id = note->id().toInt();
    int epochTimeDateCreated = note->creationDateTime().toMSecsSinceEpoch();
    QString content = note->content();
    QString fullTitle = note->fullTitle();

    query.prepare(QStringLiteral("UPDATE active_notes"
                  "SET modification_date=%1, content=%2, full_title=%3"
                  "WHERE id=%4")
                  .arg(epochTimeDateCreated)
                  .arg(content)
                  .arg(fullTitle)
                  .arg(id));
    query.exec();

}

void DBManager::removeNote(NoteData* note)
{
    QSqlQuery query;

    int id = note->id().toInt();

    query.prepare(QStringLiteral("DELETE FROM active_notes"
                  "WHERE id=%1")
                  .arg(id));
    query.exec();
}

void DBManager::modifyNote(NoteData* note)
{
    QSqlQuery query;

    int id = note->id().toInt();
    int epochTimeDateModified = note->lastModificationdateTime().toMSecsSinceEpoch();
    QString content = note->content();
    QString fullTitle = note->fullTitle();

    query.prepare(QStringLiteral("UPDATE active_notes"
                  "SET modification_date=%1, content=%2, full_title=%3"
                  "WHERE id=%4")
                  .arg(epochTimeDateModified)
                  .arg(content)
                  .arg(fullTitle)
                  .arg(id));
    query.exec();
}

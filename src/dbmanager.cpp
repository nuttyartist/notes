#include "dbmanager.h"
#include <QtSql/QSqlQuery>
#include <QTimeZone>
#include <QDateTime>
#include <QDebug>

DBManager::DBManager(const QString& path, bool doCreate, QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QList<NoteData*> >("QList<NoteData*>");

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open()){
        qDebug() << "Error: connection with database fail";
    }else{
        qDebug() << "Database: connection ok";
    }

    if(doCreate){
        QSqlQuery query;
        QString active = "CREATE TABLE active_notes ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "creation_date INTEGER NOT NULL DEFAULT (0),"
                         "modification_date INTEGER NOT NULL DEFAULT (0),"
                         "deletion_date INTEGER NOT NULL DEFAULT (0),"
                         "content TEXT, "
                         "full_title TEXT);";

        query.exec(active);

        QString active_index = "CREATE UNIQUE INDEX active_index on active_notes (id ASC);";
        query.exec(active_index);

        QString deleted = "CREATE TABLE deleted_notes ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                          "creation_date INTEGER NOT NULL DEFAULT (0),"
                          "modification_date INTEGER NOT NULL DEFAULT (0),"
                          "deletion_date INTEGER NOT NULL DEFAULT (0),"
                          "content TEXT,"
                          "full_title TEXT)";
        query.exec(deleted);
    }

}

bool DBManager::isNoteExist(NoteData* note)
{
    QSqlQuery query;

    int id = note->id().split('_')[1].toInt();
    QString queryStr = QStringLiteral("SELECT EXISTS(SELECT 1 FROM active_notes WHERE id = %1 LIMIT 1 )")
                       .arg(id);
    query.exec(queryStr);
    query.next();

    return query.value(0).toInt() == 1;
}

NoteData* DBManager::getNote(QString id) {
    QSqlQuery query;

    int parsedId = id.split('_')[1].toInt();
    QString queryStr = QStringLiteral("SELECT * FROM active_notes WHERE id = %1 LIMIT 1").arg(parsedId);
    query.exec(queryStr);

    if (query.first()) {
        NoteData* note = new NoteData(this);
        int id =  query.value(0).toInt();
        qint64 epochDateTimeCreation = query.value(1).toLongLong();
        QDateTime dateTimeCreation = QDateTime::fromMSecsSinceEpoch(epochDateTimeCreation, QTimeZone::systemTimeZone());
        qint64 epochDateTimeModification= query.value(2).toLongLong();
        QDateTime dateTimeModification = QDateTime::fromMSecsSinceEpoch(epochDateTimeModification, QTimeZone::systemTimeZone());
        QString content = query.value(4).toString();
        QString fullTitle = query.value(5).toString();

        note->setId(QStringLiteral("noteID_%1").arg(id));
        note->setCreationDateTime(dateTimeCreation);
        note->setLastModificationDateTime(dateTimeModification);
        note->setContent(content);
        note->setFullTitle(fullTitle);
        return note;
    }
    return NULL;
}

/**
 * Returns true if an existing note has the same id, title, and content, otherwise returns false.
 * This is used by the import process to determine whether or not a message should be created or updated.
 * @brief DBManager::isDuplicate
 * @param note
 * @return
 */
bool DBManager::isDuplicate(NoteData* note) {
    QSqlQuery query;
    QString queryStr = QStringLiteral("SELECT EXISTS(SELECT 1 FROM active_notes WHERE full_title = '%1' AND content = '%2' LIMIT 1 )")
            .arg(note->fullTitle())
            .arg(note->content());
    query.exec(queryStr);
    query.next();
    return query.value(0).toInt() == 1;
}

/**
 * Returns true if an existing note has the same id and title, otherwise returns false.
 * This is used by the import process to determine whether or not a message should be created or updated.
 * @brief DBManager::isDuplicate
 * @param note
 * @return
 */
bool DBManager::titleAndIDMatch(NoteData* note) {
    QSqlQuery query;
    int id = note->id().split('_')[1].toInt();
    QString queryStr = QStringLiteral("SELECT EXISTS(SELECT 1 FROM active_notes WHERE id = %1 AND full_title = '%2' LIMIT 1 )")
            .arg(id)
            .arg(note->fullTitle());
    query.exec(queryStr);
    query.next();

    return query.value(0).toInt() == 1;
}

QList<NoteData *> DBManager::getAllNotes()
{
    QList<NoteData *> noteList;

    QSqlQuery query;
    query.prepare("SELECT * FROM active_notes");
    bool status = query.exec();
    if(status){
        while(query.next()){
            NoteData* note = new NoteData(this);
            int id =  query.value(0).toInt();
            qint64 epochDateTimeCreation = query.value(1).toLongLong();
            QDateTime dateTimeCreation = QDateTime::fromMSecsSinceEpoch(epochDateTimeCreation, QTimeZone::systemTimeZone());
            qint64 epochDateTimeModification= query.value(2).toLongLong();
            QDateTime dateTimeModification = QDateTime::fromMSecsSinceEpoch(epochDateTimeModification, QTimeZone::systemTimeZone());
            QString content = query.value(4).toString();
            QString fullTitle = query.value(5).toString();

            note->setId(QStringLiteral("noteID_%1").arg(id));
            note->setCreationDateTime(dateTimeCreation);
            note->setLastModificationDateTime(dateTimeModification);
            note->setContent(content);
            note->setFullTitle(fullTitle);

            noteList.push_back(note);
        }

        emit notesReceived(noteList);
    }

    return noteList;
}

bool DBManager::addNote(NoteData* note)
{
    QSqlQuery query;
    QString emptyStr;

    qint64 epochTimeDateCreated = note->creationDateTime()
                                  .toMSecsSinceEpoch();
    QString content = note->content()
                      .replace("'","''")
                      .replace(QChar('\x0'), emptyStr);
    QString fullTitle = note->fullTitle()
                        .replace("'","''")
                        .replace(QChar('\x0'), emptyStr);

    QString queryStr = QString("INSERT INTO active_notes (creation_date, modification_date, deletion_date, content, full_title) "
                               "VALUES (%1, %1, -1, '%2', '%3');")
                       .arg(epochTimeDateCreated)
                       .arg(content)
                       .arg(fullTitle);

    query.exec(queryStr);
    return (query.numRowsAffected() == 1);

}

bool DBManager::removeNote(NoteData* note)
{
    QSqlQuery query;
    QString emptyStr;

    int id = note->id().split('_')[1].toInt();

    QString queryStr = QStringLiteral("DELETE FROM active_notes "
                                      "WHERE id=%1")
                       .arg(id);
    query.exec(queryStr);
    bool removed = (query.numRowsAffected() == 1);

    qint64 epochTimeDateCreated = note->creationDateTime().toMSecsSinceEpoch();
    qint64 epochTimeDateModified = note->lastModificationdateTime().toMSecsSinceEpoch();
    qint64 epochTimeDateDeleted = note->deletionDateTime().toMSecsSinceEpoch();
    QString content = note->content()
                      .replace("'","''")
                      .replace(QChar('\x0'), emptyStr);
    QString fullTitle = note->fullTitle()
                        .replace("'","''")
                        .replace(QChar('\x0'), emptyStr);

    queryStr = QString("INSERT INTO deleted_notes "
                       "VALUES (%1, %2, %3, %4, '%5', '%6');")
               .arg(id)
               .arg(epochTimeDateCreated)
               .arg(epochTimeDateModified)
               .arg(epochTimeDateDeleted)
               .arg(content)
               .arg(fullTitle);

    query.exec(queryStr);
    bool addedToTrashDB = (query.numRowsAffected() == 1);

    return (removed && addedToTrashDB);
}

bool DBManager::modifyNote(NoteData* note)
{
    QSqlQuery query;
    QString emptyStr;

    int id = note->id().split('_')[1].toInt();
    qint64 epochTimeDateModified = note->lastModificationdateTime().toMSecsSinceEpoch();
    QString content = note->content()
                      .replace("'","''")
                      .replace(QChar('\x0'), emptyStr);
    QString fullTitle = note->fullTitle()
                        .replace("'","''")
                        .replace(QChar('\x0'), emptyStr);

    QString queryStr = QStringLiteral("UPDATE active_notes "
                                      "SET modification_date=%1, content='%2', full_title='%3' "
                                      "WHERE id=%4")
                       .arg(epochTimeDateModified)
                       .arg(content)
                       .arg(fullTitle)
                       .arg(id);
    query.exec(queryStr);
    return (query.numRowsAffected() == 1);
}

bool DBManager::migrateNote(NoteData* note)
{
    QSqlQuery query;

    QString emptyStr;

    int id = note->id().split('_')[1].toInt();
    qint64 epochTimeDateCreated = note->creationDateTime().toMSecsSinceEpoch();
    qint64 epochTimeDateModified = note->lastModificationdateTime().toMSecsSinceEpoch();
    QString content = note->content()
                      .replace("'","''")
                      .replace(QChar('\x0'), emptyStr);
    QString fullTitle = note->fullTitle()
                        .replace("'","''")
                        .replace(QChar('\x0'), emptyStr);

    QString queryStr = QString("INSERT INTO active_notes "
                               "VALUES (%1, %2, %3, -1, '%4', '%5');")
                       .arg(id)
                       .arg(epochTimeDateCreated)
                       .arg(epochTimeDateModified)
                       .arg(content)
                       .arg(fullTitle);

    query.exec(queryStr);
    return (query.numRowsAffected() == 1);
}

bool DBManager::migrateTrash(NoteData* note)
{
    QSqlQuery query;
    QString emptyStr;

    int id = note->id().split('_')[1].toInt();
    qint64 epochTimeDateCreated = note->creationDateTime().toMSecsSinceEpoch();
    qint64 epochTimeDateModified = note->lastModificationdateTime().toMSecsSinceEpoch();
    qint64 epochTimeDateDeleted = note->deletionDateTime().toMSecsSinceEpoch();
    QString content = note->content()
                      .replace("'","''")
                      .replace(QChar('\x0'), emptyStr);
    QString fullTitle = note->fullTitle()
                        .replace("'","''")
                        .replace(QChar('\x0'), emptyStr);

    QString queryStr = QString("INSERT INTO deleted_notes "
                       "VALUES (%1, %2, %3, %4, '%5', '%6');")
               .arg(id)
               .arg(epochTimeDateCreated)
               .arg(epochTimeDateModified)
               .arg(epochTimeDateDeleted)
               .arg(content)
               .arg(fullTitle);

    query.exec(queryStr);
    return (query.numRowsAffected() == 1);
}

int DBManager::getLastRowID()
{
    QSqlQuery query;
    query.exec("SELECT seq from SQLITE_SEQUENCE WHERE name='active_notes';");
    query.next();
    return query.value(0).toInt();
}

QList<NoteExport> DBManager::getBackup() {
    QList<NoteExport> noteExports;
    QList<NoteData*> noteList = getAllNotes();
    for (int i = 0; i < noteList.size(); ++i) {
        noteExports << noteList[i]->exportNote();
    }
    return noteExports;
}

void DBManager::importNote(NoteExport noteExport) {
    NoteData* note = getNote(noteExport.id);
    if (note == NULL) {
        // Note doesn't exist, create it
        note = new NoteData(this);
        note->setId(noteExport.id);
    } else {
        // Note with this ID exists
        if (note->creationDateTime() == noteExport.creationDateTime) {
            // Note is the same
            if (note->lastModificationdateTime() == noteExport.lastModificationDateTime) {
                // note has not been updated, skip it
                return;
            }
            // Note has been updated, create a new one
            note->setId(QString());
        } else {
            // note is different, remove the ID to create a new one
            note->setId(QString());
        }
    }

    note->setFullTitle(noteExport.fullTitle);
    note->setCreationDateTime(noteExport.creationDateTime);
    note->setLastModificationDateTime(noteExport.lastModificationDateTime);
    note->setContent(noteExport.content);

    if (note->id().isNull()) {
        addNote(note);
    } else {
        modifyNote(note);
    }
}

#include "dbmanager.h"
#include <QtSql/QSqlQuery>
#include <QTimeZone>
#include <QDateTime>
#include <QDebug>
#include <QtConcurrent>

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

    int id = note->id();
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
        NoteData* note = new NoteData(this->parent() == Q_NULLPTR ? Q_NULLPTR : this);
        int id =  query.value(0).toInt();
        qint64 epochDateTimeCreation = query.value(1).toLongLong();
        QDateTime dateTimeCreation = QDateTime::fromMSecsSinceEpoch(epochDateTimeCreation);
        qint64 epochDateTimeModification= query.value(2).toLongLong();
        QDateTime dateTimeModification = QDateTime::fromMSecsSinceEpoch(epochDateTimeModification);
        QString content = query.value(4).toString();
        QString fullTitle = query.value(5).toString();

        note->setId(id);
        note->setCreationDateTime(dateTimeCreation);
        note->setLastModificationDateTime(dateTimeModification);
        note->setContent(content);
        note->setFullTitle(fullTitle);
        return note;
    }
    return Q_NULLPTR;
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
            QDateTime dateTimeCreation = QDateTime::fromMSecsSinceEpoch(epochDateTimeCreation);
            qint64 epochDateTimeModification= query.value(2).toLongLong();
            QDateTime dateTimeModification = QDateTime::fromMSecsSinceEpoch(epochDateTimeModification);
            QString content = query.value(4).toString();
            QString fullTitle = query.value(5).toString();

            note->setId(id);
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

    qint64 epochTimeDateLastModified = note->lastModificationdateTime().isNull() ? epochTimeDateCreated :  note->lastModificationdateTime().toMSecsSinceEpoch();

    QString queryStr = QString("INSERT INTO active_notes (creation_date, modification_date, deletion_date, content, full_title) "
                               "VALUES (%1, %2, -1, '%3', '%4');")
                       .arg(epochTimeDateCreated)
                       .arg(epochTimeDateLastModified)
                       .arg(content)
                       .arg(fullTitle);

    query.exec(queryStr);

    if (this->parent() == Q_NULLPTR) {
        delete note;
    }
    return (query.numRowsAffected() == 1);
}

bool DBManager::removeNote(NoteData* note)
{
    QSqlQuery query;
    QString emptyStr;

    int id = note->id();
    QString queryStr = QStringLiteral("DELETE FROM active_notes "
                                      "WHERE id=%1").arg(id);
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

bool DBManager::permanantlyRemoveAllNotes() {
    QSqlQuery query;
    return query.exec(QString("DELETE FROM active_notes"));
}

bool DBManager::modifyNote(NoteData* note)
{
    QSqlQuery query;
    QString emptyStr;

    int id = note->id();
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

    int id = note->id();
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

    int id = note->id();
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

bool DBManager::forceLastRowIndexValue(const int indexValue)
{
    QSqlQuery query;
    QString queryStr = QStringLiteral("UPDATE SQLITE_SEQUENCE "
                                      "SET seq=%1 "
                                      "WHERE name='active_notes';").arg(indexValue);
    query.exec(queryStr);
    return query.numRowsAffected() == 1;
}

void DBManager::importNotes(QList<NoteData*> noteList) {
    QSqlDatabase::database().transaction();
    QtConcurrent::blockingMap(noteList, [this] (NoteData* note) { this->addNote(note); });
    QSqlDatabase::database().commit();
}

void DBManager::restoreNotes(QList<NoteData*> noteList) {
    this->permanantlyRemoveAllNotes();
    this->importNotes(noteList);
}

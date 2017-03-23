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
    return Q_NULLPTR;
}

//NoteExport* DBManager::getNoteExport(QString id) {
//    QSqlQuery query;

//    int parsedId = id.split('_')[1].toInt();
//    QString queryStr = QStringLiteral("SELECT * FROM active_notes WHERE id = %1 LIMIT 1").arg(parsedId);
//    query.exec(queryStr);

//    if (query.first()) {
//        NoteExport* note = new NoteExport();
//        int id =  query.value(0).toInt();
//        qint64 epochDateTimeCreation = query.value(1).toLongLong();
//        QDateTime dateTimeCreation = QDateTime::fromMSecsSinceEpoch(epochDateTimeCreation, QTimeZone::systemTimeZone());
//        qint64 epochDateTimeModification= query.value(2).toLongLong();
//        QDateTime dateTimeModification = QDateTime::fromMSecsSinceEpoch(epochDateTimeModification, QTimeZone::systemTimeZone());
//        QString content = query.value(4).toString();
//        QString fullTitle = query.value(5).toString();

//        note->id = QStringLiteral("noteID_%1").arg(id);
//        note->creationDateTime = dateTimeCreation;
//        note->lastModificationDateTime = dateTimeModification;
//        note->content = content;
//        note->fullTitle = fullTitle;
//        return note;
//    }
//    return Q_NULLPTR;
//}


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

//bool DBManager::addImportedNote(NoteExport* note) {
//    QSqlQuery query;
//    QString emptyStr;
//    qint64 epochTimeDateCreated = note->creationDateTime.toMSecsSinceEpoch();
//    qint64 epochTimeDateModified = note->lastModificationDateTime.toMSecsSinceEpoch();
//    QString content = note->content.replace("'","''").replace(QChar('\x0'), emptyStr);
//    QString fullTitle = note->fullTitle.replace("'","''").replace(QChar('\x0'), emptyStr);
//    QString queryStr = QString("INSERT INTO active_notes (creation_date, modification_date, deletion_date, content, full_title) "
//                               "VALUES (%1, %2, -1, '%3', '%4');")
//            .arg(epochTimeDateCreated)
//            .arg(epochTimeDateModified)
//            .arg(content)
//            .arg(fullTitle);

//    query.exec(queryStr);
//    return (query.numRowsAffected() == 1);

//}

//bool DBManager::updateImportedNote(NoteExport* note) {
//    QSqlQuery query;
//    QString emptyStr;
//    int id = note->id.split('_')[1].toInt();
//    qint64 epochTimeDateModified = note->lastModificationDateTime.toMSecsSinceEpoch();
//    QString content = note->content.replace("'","''").replace(QChar('\x0'), emptyStr);
//    QString fullTitle = note->fullTitle.replace("'","''").replace(QChar('\x0'), emptyStr);
//    QString queryStr = QStringLiteral("UPDATE active_notes "
//                                      "SET modification_date=%1, content='%2', full_title='%3' "
//                                      "WHERE id=%4")
//                       .arg(epochTimeDateModified)
//                       .arg(content)
//                       .arg(fullTitle)
//                       .arg(id);
//    query.exec(queryStr);
//    return (query.numRowsAffected() == 1);
//}

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

//QList<NoteExport> DBManager::exportNotes() {
//    QList<NoteExport> noteList;
//    QSqlQuery query;
//    query.prepare("SELECT * FROM active_notes");
//    bool status = query.exec();
//    if(status){
//        while(query.next()){
//            noteList.push_back(buildNote(query));
//        }
//    }
//    return noteList;
//}

//NoteExport DBManager::buildNote(QSqlQuery query) {
//    NoteExport note;
//    int id =  query.value(0).toInt();
//    qint64 epochDateTimeCreation = query.value(1).toLongLong();
//    QDateTime dateTimeCreation = QDateTime::fromMSecsSinceEpoch(epochDateTimeCreation, QTimeZone::systemTimeZone());
//    qint64 epochDateTimeModification= query.value(2).toLongLong();
//    QDateTime dateTimeModification = QDateTime::fromMSecsSinceEpoch(epochDateTimeModification, QTimeZone::systemTimeZone());
//    QString content = query.value(4).toString();
//    QString fullTitle = query.value(5).toString();
//    note.id = QStringLiteral("noteID_%1").arg(id);
//    note.fullTitle = fullTitle;
//    note.creationDateTime = dateTimeCreation;
//    note.lastModificationDateTime = dateTimeModification;
//    note.content = content;
//    return note;
//}

void DBManager::importNote(NoteData* noteExport) {
    qInfo() << "XXXXXXXXXXXXX 1";
    NoteData* note = getNote(noteExport->id());
    if (note == Q_NULLPTR) {
        qInfo() << "XXXXXXXXXXXXX 2";
        // Note doesn't exist, create it
        addNote(noteExport);
    } else {

        qInfo() << "XXXXXXXXXXXXX 3";
        // Note with this ID exists
        if (note->creationDateTime() == noteExport->creationDateTime()) {
            // Note is the same
            if (note->lastModificationdateTime() == noteExport->lastModificationdateTime()) {
                // note has not been updated, skip it
                qInfo() << "XXXXXXXXXXXXX 4";
                return;
            }
            // Note has been updated, create a new one
            qInfo() << "XXXXXXXXXXXXX 5";
            note->setId(QString());
            addNote(noteExport);
        } else {
            // note is different, remove the ID to create a new one
            qInfo() << "XXXXXXXXXXXXX 6";
            note->setId(QString());
            addNote(noteExport);
        }
    }
}

//void DBManager::importNote(const NoteExport& noteExport) {
//    NoteExport* note = getNoteExport(noteExport.id);
//    bool isNew = false;
//    if (note == Q_NULLPTR) {
//        // Note doesn't exist, create it
//        note = new NoteExport();
//        note->id = noteExport.id;
//        isNew = true;
//    } else {
//        // Note with this ID exists
//        if (note->creationDateTime == noteExport.creationDateTime) {
//            // Note is the same
//            if (note->lastModificationDateTime == noteExport.lastModificationDateTime) {
//                // note has not been updated, skip it
//                return;
//            }
//            // Note has been updated, create a new one
//            note->id = QString();
//        } else {
//            // note is different, remove the ID to create a new one
//            note->id = QString();
//        }
//    }

//    note->fullTitle = noteExport.fullTitle;
//    note->creationDateTime = noteExport.creationDateTime;
//    note->lastModificationDateTime = noteExport.lastModificationDateTime;
//    note->content = noteExport.content;

//    if (isNew || note->id.isNull()) {
//        addImportedNote(note);
//    } else {
//        updateImportedNote(note);
//    }
//    delete note;
//}

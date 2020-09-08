#include "dbmanager.h"
#include <QtSql/QSqlQuery>
#include <QTimeZone>
#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QtConcurrent>

/*!
 * \brief DBManager::DBManager
 * \param parent
 */
DBManager::DBManager(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QList<NoteData*> >("QList<NoteData*>");
}

/*!
 * \brief DBManager::open
 * \param path
 * \param doCreate
 */
void DBManager::open(const QString &path, bool doCreate)
{
    QSqlDatabase m_db;
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    m_db.setDatabaseName(path);
    if(!m_db.open()){
        qDebug() << "Error: connection with database fail";
    }else{
        qDebug() << "Database: connection ok";
    }

    if(doCreate)
        createTables();
}

/*!
 * \brief DBManager::createTables
 */
void DBManager::createTables()
{
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

/*!
 * \brief DBManager::getLastRowID
 * \return
 */
int DBManager::getLastRowID()
{
    QSqlQuery query;
    query.exec("SELECT seq from SQLITE_SEQUENCE WHERE name='active_notes';");
    query.next();
    return query.value(0).toInt();
}

/*!
 * \brief DBManager::forceLastRowIndexValue
 * \param indexValue
 * \return
 */
bool DBManager::forceLastRowIndexValue(const int indexValue)
{
    QSqlQuery query;
    QString queryStr = QStringLiteral("UPDATE SQLITE_SEQUENCE "
                                      "SET seq=%1 "
                                      "WHERE name='active_notes';").arg(indexValue);
    query.exec(queryStr);
    return query.numRowsAffected() == 1;
}

/*!
 * \brief DBManager::getNote
 * \param id
 * \return
 */
NoteData* DBManager::getNote(QString id)
{
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

/*!
 * \brief DBManager::isNoteExist
 * \param note
 * \return
 */
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

/*!
 * \brief DBManager::getAllNotes
 * \return
 */
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
    }

    return noteList;
}

/*!
 * \brief DBManager::addNote
 * \param note
 * \return
 */
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

    qint64 epochTimeDateLastModified = note->lastModificationdateTime().isNull() ? epochTimeDateCreated
                                                                                 : note->lastModificationdateTime().toMSecsSinceEpoch();

    QString queryStr = QString("INSERT INTO active_notes "
                               "(creation_date, modification_date, deletion_date, content, full_title) "
                               "VALUES (%1, %2, -1, '%3', '%4');")
                               .arg(epochTimeDateCreated)
                               .arg(epochTimeDateLastModified)
                               .arg(content)
                               .arg(fullTitle);

    query.exec(queryStr);

    return (query.numRowsAffected() == 1);
}

/*!
 * \brief DBManager::removeNote
 * \param note
 * \return
 */
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

/*!
 * \brief DBManager::permanantlyRemoveAllNotes
 * \return
 */
bool DBManager::permanantlyRemoveAllNotes()
{
    QSqlQuery query;
    return query.exec(QString("DELETE FROM active_notes"));
}

/*!
 * \brief DBManager::updateNote
 * \param note
 * \return
 */
bool DBManager::updateNote(NoteData* note)
{
    QSqlQuery query;
    QString emptyStr;

    int id = note->id();
    qint64 epochTimeDateModified = note->lastModificationdateTime().toMSecsSinceEpoch();
    QString content = note->content().replace(QChar('\x0'), emptyStr);
    QString fullTitle = note->fullTitle().replace(QChar('\x0'), emptyStr);

    query.prepare(QStringLiteral("UPDATE active_notes SET modification_date = :date, content = :content, "
                                 "full_title = :title WHERE id = :id"));
    query.bindValue(QStringLiteral(":date"), epochTimeDateModified);
    query.bindValue(QStringLiteral(":content"), content);
    query.bindValue(QStringLiteral(":title"), fullTitle);
    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        qWarning () << __func__ << ": " << query.lastError();
    }
    return (query.numRowsAffected() == 1);
}

/*!
 * \brief DBManager::migrateNote
 * \param note
 * \return
 */
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

/*!
 * \brief DBManager::migrateTrash
 * \param note
 * \return
 */
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

/*!
 * \brief DBManager::onNotesListRequested
 */
void DBManager::onNotesListRequested()
{
    int noteCounter;
    QList<NoteData *> noteList;

    noteCounter = getLastRowID();
    noteList    = getAllNotes();

    emit notesReceived(noteList, noteCounter);
}

/*!
 * \brief DBManager::onOpenDBManagerRequested
 * \param path
 * \param doCreate
 */
void DBManager::onOpenDBManagerRequested(QString path, bool doCreate)
{
    open(path, doCreate);
}

/*!
 * \brief DBManager::onCreateUpdateRequested
 * \param note
 */
void DBManager::onCreateUpdateRequested(NoteData* note)
{
    bool exists = isNoteExist(note);

    if(exists)
        updateNote(note);
    else
        addNote(note);
}

/*!
 * \brief DBManager::onDeleteNoteRequested
 * \param note
 */
void DBManager::onDeleteNoteRequested(NoteData* note)
{
    removeNote(note);
}

/*!
 * \brief DBManager::onImportNotesRequested
 * \param noteList
 */
void DBManager::onImportNotesRequested(QList<NoteData *> noteList) {
    QSqlDatabase::database().transaction();
    for(NoteData* note : noteList)
        addNote(note);
    QSqlDatabase::database().commit();
}

/*!
 * \brief DBManager::onRestoreNotesRequested
 * \param noteList
 */
void DBManager::onRestoreNotesRequested(QList<NoteData*> noteList) {
    this->permanantlyRemoveAllNotes();
    this->onImportNotesRequested(noteList);
}

/*!
 * \brief DBManager::onExportNotesRequested
 * \param fileName
 */
void DBManager::onExportNotesRequested(QString fileName)
{
    QList<NoteData *> noteList;
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    out.setVersion(QDataStream::Qt_5_6);
#elif QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    out.setVersion(QDataStream::Qt_5_4);
#elif QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
    out.setVersion(QDataStream::Qt_5_2);
#endif
    noteList = getAllNotes();
    out << noteList;
    file.close();

    qDeleteAll(noteList);
    noteList.clear();
}

/*!
 * \brief DBManager::onMigrateNotesRequested
 * \param noteList
 */
void DBManager::onMigrateNotesRequested(QList<NoteData *> noteList)
{
    QSqlDatabase::database().transaction();
    for(NoteData* note : noteList)
        migrateNote(note);
    QSqlDatabase::database().commit();

    qDeleteAll(noteList);
    noteList.clear();
}

/*!
 * \brief DBManager::onMigrateTrashRequested
 * \param noteList
 */
void DBManager::onMigrateTrashRequested(QList<NoteData *> noteList)
{
    QSqlDatabase::database().transaction();
    for(NoteData* note : noteList)
        migrateTrash(note);
    QSqlDatabase::database().commit();

    qDeleteAll(noteList);
    noteList.clear();
}

/*!
 * \brief DBManager::onForceLastRowIndexValueRequested
 * \param index
 */
void DBManager::onForceLastRowIndexValueRequested(int index)
{
    forceLastRowIndexValue(index);
}

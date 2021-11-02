#include "dbmanager.h"
#include <QtSql/QSqlQuery>
#include <QTimeZone>
#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QtConcurrent>
#include <QSqlRecord>
#include <QSet>

/*!
 * \brief DBManager::DBManager
 * \param parent
 */
DBManager::DBManager(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QList<NodeData*>>("QList<NodeData*>");
    qRegisterMetaType<QVector<NodeData>>("QVector<NodeData>");
    qRegisterMetaType<NodeTagTreeData>("NodeTagTreeData");
    qRegisterMetaType<QSet<int>>("QSet<int>");
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
    QSqlDatabase::database().transaction();
    QSqlQuery query;

    QString nodeTable = R"(CREATE TABLE "node_table" ()"
                        R"(    "id"	INTEGER NOT NULL,)"
                        R"(    "title"	TEXT,)"
                        R"(    "creation_date"	INTEGER NOT NULL DEFAULT 0,)"
                        R"(    "modification_date"	INTEGER NOT NULL DEFAULT 0,)"
                        R"(    "deletion_date"	INTEGER NOT NULL DEFAULT 0,)"
                        R"(    "content"	TEXT,)"
                        R"(    "node_type"	INTEGER NOT NULL,)"
                        R"(    "parent_id"	INTEGER NOT NULL,)"
                        R"(    "relative_position"	INTEGER NOT NULL,)"
                        R"(    "absolute_path"	TEXT NOT NULL)"
                        R"();)";
    query.exec(nodeTable);

    QString tagRelationship = R"(CREATE TABLE "tag_relationship" ()"
                              R"(    "node_id"	INTEGER NOT NULL,)"
                              R"(    "tag_id"	INTEGER NOT NULL)"
                              R"();)";
    query.exec(tagRelationship);
    QString tagTable = R"(CREATE TABLE "tag_table" ()"
                       R"(    "id"	INTEGER NOT NULL,)"
                       R"(    "name"	TEXT NOT NULL,)"
                       R"(    "color"	TEXT NOT NULL,)"
                       R"(    "relative_position"	INTEGER NOT NULL)"
                       R"();)";
    query.exec(tagTable);

    QString metadata = R"(CREATE TABLE "metadata" ()"
                       R"(    "key"	TEXT NOT NULL,)"
                       R"(    "value"	INTEGER NOT NULL)"
                       R"();)";
    query.exec(metadata);

    query.prepare(R"(INSERT INTO "metadata"("key","value") VALUES (:key, :value);)");
    query.bindValue(":key", "next_node_id");
    query.bindValue(":value", 0);
    query.exec();

    query.bindValue(":key", "next_tag_id");
    query.bindValue(":value", 0);
    query.exec();

    NodeData rootFolder; // id 0
    rootFolder.setNodeType(NodeData::Folder);
    QDateTime noteDate = QDateTime::currentDateTime();
    rootFolder.setCreationDateTime(noteDate);
    rootFolder.setLastModificationDateTime(noteDate);
    rootFolder.setFullTitle(QStringLiteral("/"));
    rootFolder.setParentId(-1);
    addNode(rootFolder);

    NodeData trashFolder; // id 1
    trashFolder.setNodeType(NodeData::Folder);
    trashFolder.setCreationDateTime(noteDate);
    trashFolder.setLastModificationDateTime(noteDate);
    trashFolder.setFullTitle(QStringLiteral("Trash"));
    trashFolder.setParentId(0);
    addNode(trashFolder);

    NodeData notesFolder; // id 2
    notesFolder.setNodeType(NodeData::Folder);
    notesFolder.setCreationDateTime(noteDate);
    notesFolder.setLastModificationDateTime(noteDate);
    notesFolder.setFullTitle(QStringLiteral("Notes"));
    notesFolder.setParentId(0);
    addNode(notesFolder);

    QSqlDatabase::database().commit();
}

/*!
 * \brief DBManager::isNoteExist
 * \param note
 * \return
 */
bool DBManager::isNodeExist(const NodeData& node)
{
    QSqlQuery query;

    int id = node.id();
    QString queryStr = QStringLiteral("SELECT EXISTS(SELECT 1 FROM node_table WHERE id = :id LIMIT 1 )");
    query.prepare(queryStr);
    query.bindValue(":id", id);
    query.exec();
    query.next();
    return query.value(0).toInt() == 1;
}


QVector<NodeData> DBManager::getAllFolders()
{
    QVector<NodeData> nodeList;

    QSqlQuery query;
    query.prepare(R"(SELECT)"
                  R"("id",)"
                  R"("title",)"
                  R"("creation_date",)"
                  R"("modification_date",)"
                  R"("deletion_date",)"
                  R"("content",)"
                  R"("node_type",)"
                  R"("parent_id",)"
                  R"("relative_position",)"
                  R"("absolute_path" )"
                  R"(FROM node_table WHERE node_type=:node_type;)"
                );
    query.bindValue(":node_type", static_cast<int>(NodeData::Type::Folder));
    bool status = query.exec();
    if(status) {
        while(query.next()) {
            NodeData node;
            node.setId(query.value(0).toInt());
            node.setFullTitle(query.value(1).toString());
            node.setCreationDateTime(QDateTime::fromMSecsSinceEpoch(query.value(2).toLongLong()));
            node.setLastModificationDateTime(QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong()));
            node.setDeletionDateTime(QDateTime::fromMSecsSinceEpoch(query.value(4).toLongLong()));
            node.setContent(query.value(5).toString());
            node.setNodeType(static_cast<NodeData::Type>(query.value(6).toInt()));
            node.setParentId(query.value(7).toInt());
            node.setRelativePosition(query.value(8).toInt());
            node.setAbsolutePath(query.value(9).toString());
            if (node.nodeType() == NodeData::Note) {
                node.setTagIds(getAllTagForNote(node.id()));
            }
            nodeList.append(node);
        }
    } else {
        qDebug() << "getAllNodes query failed!" << query.lastError();
    }

    return nodeList;
}

QVector<TagData> DBManager::getAllTagInfo()
{
    QVector<TagData> tagList;

    QSqlQuery query;
    query.prepare(R"(SELECT "id","name","color","relative_position" FROM tag_table;)");
    bool status = query.exec();
    if(status) {
        while(query.next()) {
            TagData tag;
            tag.setId(query.value(0).toInt());
            tag.setName(query.value(1).toString());
            tag.setColor(query.value(2).toString());
            tag.setRelativePosition(query.value(3).toInt());
            tagList.append(tag);
        }
    } else {
        qDebug() << "getAllTagInfo query failed!" << query.lastError();
    }

    return tagList;
}

QSet<int> DBManager::getAllTagForNote(int noteId)
{
    QSet<int> tagIds;
    QSqlQuery query;
    query.prepare(R"(SELECT "tag_id" FROM tag_relationship WHERE node_id = :node_id;)");
    query.bindValue(":node_id", noteId);
    bool status = query.exec();
    if(status) {
        while(query.next()) {
            tagIds.insert(query.value(0).toInt());
        }
    } else {
        qDebug() << __FUNCTION__ << "query failed!" << query.lastError();
    }
    return tagIds;
}

int DBManager::addNode(const NodeData &node)
{
    QSqlQuery query;
    QString emptyStr;

    qint64 epochTimeDateCreated = node.creationDateTime()
            .toMSecsSinceEpoch();
    QString content = node.content()
            .replace("'","''")
            .replace(QChar('\x0'), emptyStr);
    QString fullTitle = node.fullTitle()
            .replace("'","''")
            .replace(QChar('\x0'), emptyStr);

    qint64 epochTimeDateLastModified = node.lastModificationdateTime().isNull() ? epochTimeDateCreated
                                                                                : node.lastModificationdateTime().toMSecsSinceEpoch();

    int relationalPosition = 0;
    if (node.parentId() != -1) {
        // DIEPDTN: wrong algorithm, need fix
        query.prepare("SELECT COUNT(*) FROM node_table WHERE parent_id = :parent_id");
        query.bindValue(":parent_id", node.parentId());
        query.exec();
        query.last();
        relationalPosition = query.value(0).toInt();
        query.finish();
    }
    int nodeId = nextAvailableNodeId();
    QString absolutePath;
    if (node.parentId() != -1) {
        absolutePath = getNodeAbsolutePath(node.parentId()).path();
    }
    qDebug() << " parent path " << absolutePath;
    absolutePath += PATH_SEPERATOR + QString::number(nodeId);
    qDebug() << absolutePath;
    QString queryStr = R"(INSERT INTO "node_table")"
                       R"(("id", "title", "creation_date", "modification_date", "deletion_date", "content", "node_type", "parent_id", "relative_position", "absolute_path"))"
                       R"(VALUES (:id, :title, :creation_date, :modification_date, :deletion_date, :content, :node_type, :parent_id, :relative_position, :absolute_path);)";

    query.prepare(queryStr);
    query.bindValue(":id", nodeId);
    query.bindValue(":title", fullTitle);
    query.bindValue(":creation_date", epochTimeDateCreated);
    query.bindValue(":modification_date", epochTimeDateLastModified);
    query.bindValue(":deletion_date", -1);
    query.bindValue(":content", content);
    query.bindValue(":node_type", static_cast<int>(node.nodeType()));
    query.bindValue(":parent_id", node.parentId());
    query.bindValue(":relative_position", relationalPosition);
    query.bindValue(":absolute_path", absolutePath);
    query.exec();
    query.finish();

    query.prepare(R"(UPDATE "metadata" SET "value"=:value WHERE "key"='next_node_id';)");
    query.bindValue(":value", nodeId + 1);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    return nodeId;
}

int DBManager::addTag(const TagData &tag)
{
    QSqlQuery query;

    int relationalPosition = -1;
    int id = nextAvailableTagId();

    QString queryStr = R"(INSERT INTO "tag_table" )"
                       R"(("id","name","color","relative_position") )"
                       R"(VALUES (:id, :name, :color, :relative_position);)";
    query.prepare(queryStr);
    query.bindValue(":id", id);
    query.bindValue(":name", tag.name());
    query.bindValue(":color", tag.color());
    query.bindValue(":relative_position", relationalPosition);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.finish();

    query.prepare(R"(UPDATE "metadata" SET "value"=:value WHERE "key"='next_tag_id';)");
    query.bindValue(":value", id + 1);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    auto newTag = tag;
    newTag.setId(id);
    emit tagAdded(newTag);
    return id;
}

void DBManager::addNoteToTag(int noteId, int tagId)
{
    QSqlQuery query;
    query.prepare(R"(INSERT INTO "tag_relationship" ("node_id","tag_id") VALUES (:note_id, :tag_id);)");
    query.bindValue(":note_id", noteId);
    query.bindValue(":tag_id", tagId);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

int DBManager::nextAvailableNodeId()
{
    QSqlQuery query;
    query.prepare("SELECT value FROM metadata WHERE key = :key");
    query.bindValue(":key", "next_node_id");
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.last();
    int nodeId = query.value(0).toInt();
    return nodeId;
}

int DBManager::nextAvailableTagId()
{
    QSqlQuery query;
    query.prepare("SELECT value FROM metadata WHERE key = :key");
    query.bindValue(":key", "next_tag_id");
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.last();
    int nodeId = query.value(0).toInt();
    return nodeId;
}

void DBManager::renameNode(int id, const QString &newName)
{
    QSqlQuery query;
    query.prepare(R"(UPDATE "node_table" SET "title"=:title WHERE "id"=:id;)");
    query.bindValue(":title", newName);
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

void DBManager::renameTag(int id, const QString &newName)
{
    QSqlQuery query;
    query.prepare(R"(UPDATE "tag_table" SET "name"=:name WHERE "id"=:id;)");
    query.bindValue(":name", newName);
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

void DBManager::changeTagColor(int id, const QString &newColor)
{
    QSqlQuery query;
    query.prepare(R"(UPDATE "tag_table" SET "color"=:color WHERE "id"=:id;)");
    query.bindValue(":color", newColor);
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

/*!
 * \brief DBManager::removeNote
 * \param note
 * \return
 */
void DBManager::removeNote(const NodeData &note)
{
    if (note.parentId() == SpecialNodeID::TrashFolder) {
        QSqlQuery query;
        query.prepare(R"(DELETE FROM "node_table" )"
                      R"(WHERE id = (:id) AND node_type = (:node_type);)"
                        );
        query.bindValue(QStringLiteral(":id"), note.id());
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));
        if (!query.exec()) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
    } else {
        auto trashFolder = getNode(SpecialNodeID::TrashFolder);
        moveNode(note.id(), trashFolder);
    }
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
bool DBManager::updateNoteContent(const NodeData& note)
{
    QSqlQuery query;
    QString emptyStr;

    int id = note.id();
    if (id == SpecialNodeID::InvalidNoteId) {
        qDebug() << "Invalid Note ID";
        return false;
    }
    qint64 epochTimeDateModified = note.lastModificationdateTime().toMSecsSinceEpoch();
    QString content = note.content().replace(QChar('\x0'), emptyStr);
    QString fullTitle = note.fullTitle().replace(QChar('\x0'), emptyStr);
    
    query.prepare(QStringLiteral("UPDATE node_table SET modification_date = :modification_date, content = :content, "
                                 "title = :title WHERE id = :id AND node_type = :node_type;"));
    query.bindValue(QStringLiteral(":modification_date"), epochTimeDateModified);
    query.bindValue(QStringLiteral(":content"), content);
    query.bindValue(QStringLiteral(":title"), fullTitle);
    query.bindValue(QStringLiteral(":id"), id);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));
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
bool DBManager::migrateNote(NodeData* note)
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
bool DBManager::migrateTrash(NodeData* note)
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

NodePath DBManager::getNodeAbsolutePath(int nodeId)
{
    QSqlQuery query;
    query.prepare("SELECT absolute_path FROM node_table WHERE id = :id");
    query.bindValue(":id", nodeId);
    query.exec();
    query.last();
    auto absolutePath = query.value(0).toString();
    return absolutePath;
}

NodeData DBManager::getNode(int nodeId)
{
    QSqlQuery query;
    query.prepare(R"(SELECT)"
                  R"("id",)"
                  R"("title",)"
                  R"("creation_date",)"
                  R"("modification_date",)"
                  R"("deletion_date",)"
                  R"("content",)"
                  R"("node_type",)"
                  R"("parent_id",)"
                  R"("relative_position",)"
                  R"("absolute_path" )"
                  R"(FROM node_table WHERE id=:id LIMIT 1;)"
                );
    query.bindValue(":id", nodeId);
    bool status = query.exec();
    if(status) {
        query.next();
        NodeData node;
        node.setId(query.value(0).toInt());
        node.setFullTitle(query.value(1).toString());
        node.setCreationDateTime(QDateTime::fromMSecsSinceEpoch(query.value(2).toLongLong()));
        node.setLastModificationDateTime(QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong()));
        node.setDeletionDateTime(QDateTime::fromMSecsSinceEpoch(query.value(4).toLongLong()));
        node.setContent(query.value(5).toString());
        node.setNodeType(static_cast<NodeData::Type>(query.value(6).toInt()));
        node.setParentId(query.value(7).toInt());
        node.setRelativePosition(query.value(8).toInt());
        node.setAbsolutePath(query.value(9).toString());
        if (node.nodeType() == NodeData::Note) {
            node.setTagIds(getAllTagForNote(node.id()));
        }
        return node;
    } else {
        qDebug() << "getAllNodes query failed!" << query.lastError();
    }
    qDebug() << "Can't find node with id" << nodeId;

    return NodeData();
}

void DBManager::moveFolderToTrash(const NodeData &node)
{
    QSqlQuery query;
    query.prepare(R"(DELETE FROM "node_table" )"
                  R"(WHERE absolute_path like (:path_expr) || '%' AND node_type = (:node_type);)"
                    );
    QString parentPath = node.absolutePath();
    query.bindValue(QStringLiteral(":path_expr"), parentPath);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Folder));
    bool status = query.exec();
    if(!status) {
        qDebug() << "moveFolderToTrash query failed!" << query.lastError();
    }
    query.clear();
    query.prepare(R"(SELECT id FROM "node_table" )"
                  R"(WHERE absolute_path like (:path_expr) || '%' AND node_type = (:node_type);)"
                    );
    query.bindValue(QStringLiteral(":path_expr"), parentPath);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));
    status = query.exec();
    QVector<int> childIds;
    if (status) {
        query.next();
        childIds.append(query.value(0).toInt());
    } else {
        qDebug() << "moveFolderToTrash query failed!" << query.lastError();
    }
    auto trashFolder = getNode(SpecialNodeID::TrashFolder);
    for (const auto& id : childIds) {
        moveNode(id, trashFolder);
    }
}

void DBManager::moveNode(int nodeId, const NodeData &target)
{
    if (target.nodeType() != NodeData::Folder) {
        qDebug() << "moveNode target is not folder" << target.id();
        return;
    }
    QSqlQuery query;

    QString newAbsolutePath = target.absolutePath() + PATH_SEPERATOR + QString::number(nodeId);
    query.prepare(QStringLiteral("UPDATE node_table SET parent_id = :parent_id, absolute_path = :absolute_path "
                                 "WHERE id = :id;"));
    query.bindValue(QStringLiteral(":parent_id"), target.id());
    query.bindValue(QStringLiteral(":absolute_path"), newAbsolutePath);
    query.bindValue(QStringLiteral(":id"), nodeId);
    bool status = query.exec();
    if(!status) {
        qDebug() << "moveNode query failed!" << query.lastError();
    }
}

void DBManager::onNodeTagTreeRequested()
{
    NodeTagTreeData d;
    d.nodeTreeData = getAllFolders();
    d.tagTreeData = getAllTagInfo();
    emit nodesTagTreeReceived(d);
}

/*!
 * \brief DBManager::onNotesListRequested
 */
void DBManager::onNotesListInFolderRequested(int parentID, bool isRecursive)
{
    QVector<NodeData> nodeList;
    QSqlQuery query;
    if (!isRecursive) {
        query.prepare(R"(SELECT )"
                      R"("id",)"
                      R"("title",)"
                      R"("creation_date",)"
                      R"("modification_date",)"
                      R"("deletion_date",)"
                      R"("content",)"
                      R"("node_type",)"
                      R"("parent_id",)"
                      R"("relative_position" )"
                      R"(FROM node_table )"
                      R"(WHERE parent_id = (:parent_id) AND node_type = (:node_type);)"
                    );
        query.bindValue(QStringLiteral(":parent_id"), parentID);
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));
        bool status = query.exec();
        if(status) {
            while(query.next()) {
                NodeData node;
                node.setId(query.value(0).toInt());
                node.setFullTitle(query.value(1).toString());
                node.setCreationDateTime(QDateTime::fromMSecsSinceEpoch(query.value(2).toLongLong()));
                node.setLastModificationDateTime(QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong()));
                node.setDeletionDateTime(QDateTime::fromMSecsSinceEpoch(query.value(4).toLongLong()));
                node.setContent(query.value(5).toString());
                node.setNodeType(static_cast<NodeData::Type>(query.value(6).toInt()));
                node.setParentId(query.value(7).toInt());
                node.setRelativePosition(query.value(8).toInt());
                node.setTagIds(getAllTagForNote(node.id()));
                nodeList.append(node);
            }
        } else {
            qDebug() << __LINE__ << "Database query failed!" << query.lastError();
            qDebug() << query.lastQuery().toStdString().c_str();
        }
    } else {
        auto parentPath = getNodeAbsolutePath(parentID).path();
        query.prepare(R"(SELECT )"
                      R"("id",)"
                      R"("title",)"
                      R"("creation_date",)"
                      R"("modification_date",)"
                      R"("deletion_date",)"
                      R"("content",)"
                      R"("node_type",)"
                      R"("parent_id",)"
                      R"("relative_position" )"
                      R"(FROM node_table )"
                      R"(WHERE absolute_path like (:path_expr) || '%' AND node_type = (:node_type);)"
                    );
        query.bindValue(QStringLiteral(":path_expr"), parentPath);
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Note));

        bool status = query.exec();
        if(status) {
            while(query.next()) {
                NodeData node;
                node.setId(query.value(0).toInt());
                node.setFullTitle(query.value(1).toString());
                node.setCreationDateTime(QDateTime::fromMSecsSinceEpoch(query.value(2).toLongLong()));
                node.setLastModificationDateTime(QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong()));
                node.setDeletionDateTime(QDateTime::fromMSecsSinceEpoch(query.value(4).toLongLong()));
                node.setContent(query.value(5).toString());
                node.setNodeType(static_cast<NodeData::Type>(query.value(6).toInt()));
                node.setParentId(query.value(7).toInt());
                node.setRelativePosition(query.value(8).toInt());
                node.setTagIds(getAllTagForNote(node.id()));
                nodeList.append(node);
            }
        } else {
            qDebug() << __LINE__ << "Database query failed!" << query.lastError();
        }
    }
    emit notesListReceived(nodeList);
}

void DBManager::onNotesListInTagRequested(int tagId)
{
    QVector<NodeData> nodeList;
    QSqlQuery query;
    query.prepare(R"(SELECT "node_id" FROM tag_relationship WHERE tag_id = :tag_id;)");
    query.bindValue(QStringLiteral(":tag_id"), tagId);
    bool status = query.exec();
    if(status) {
        QSet<int> noteIds;
        while(query.next()) {
            noteIds.insert(query.value(0).toInt());
        }
        for (const auto& id : noteIds) {
            NodeData node = getNode(id);
            if (node.id() != SpecialNodeID::InvalidNoteId &&
                    node.nodeType() == NodeData::Note) {
                nodeList.append(node);
            } else {
                qDebug() << __FUNCTION__ << "Note with id" << id << "is not valid";
            }
        }
    } else {
        qDebug() << __LINE__ << "Database query failed!" << query.lastError();
        qDebug() << query.lastQuery().toStdString().c_str();
    }
    emit notesListReceived(nodeList);
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
void DBManager::onCreateUpdateRequestedNoteContent(const NodeData &note)
{
    if (note.nodeType() != NodeData::Note) {
        qDebug() << "Wrong node type";
        return;
    }
    bool exists = isNodeExist(note);
    
    if(exists) {
        updateNoteContent(note);
    } else {
        addNode(note);
    }
}

/*!
 * \brief DBManager::onDeleteNoteRequested
 * \param note
 */
void DBManager::onDeleteNoteRequested(const NodeData& note)
{
    removeNote(note);
}

/*!
 * \brief DBManager::onImportNotesRequested
 * \param noteList
 */
void DBManager::onImportNotesRequested(QList<NodeData *> noteList) {
    //    QSqlDatabase::database().transaction();
    //    for(NodeData* note : noteList)
    //        addNote(note);
    //    QSqlDatabase::database().commit();
}

/*!
 * \brief DBManager::onRestoreNotesRequested
 * \param noteList
 */
void DBManager::onRestoreNotesRequested(QList<NodeData*> noteList) {
    this->permanantlyRemoveAllNotes();
    this->onImportNotesRequested(noteList);
}

/*!
 * \brief DBManager::onExportNotesRequested
 * \param fileName
 */
void DBManager::onExportNotesRequested(QString fileName)
{
    //    QList<NodeData *> noteList;
    //    QFile file(fileName);
    //    file.open(QIODevice::WriteOnly);
    //    QDataStream out(&file);
    //#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    //    out.setVersion(QDataStream::Qt_5_6);
    //#elif QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    //    out.setVersion(QDataStream::Qt_5_4);
    //#elif QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
    //    out.setVersion(QDataStream::Qt_5_2);
    //#endif
    //    noteList = getAllNotes();
    //    out << noteList;
    //    file.close();
    
    //    qDeleteAll(noteList);
    //    noteList.clear();
}

/*!
 * \brief DBManager::onMigrateNotesRequested
 * \param noteList
 */
void DBManager::onMigrateNotesRequested(QList<NodeData *> noteList)
{
    //    QSqlDatabase::database().transaction();
    //    for(NodeData* note : noteList)
    //        migrateNote(note);
    //    QSqlDatabase::database().commit();
    
    //    qDeleteAll(noteList);
    //    noteList.clear();
}

/*!
 * \brief DBManager::onMigrateTrashRequested
 * \param noteList
 */
void DBManager::onMigrateTrashRequested(QList<NodeData *> noteList)
{
    //    QSqlDatabase::database().transaction();
    //    for(NodeData* note : noteList)
    //        migrateTrash(note);
    //    QSqlDatabase::database().commit();
    
    //    qDeleteAll(noteList);
    //    noteList.clear();
}


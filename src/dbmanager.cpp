#include "dbmanager.h"
#include <QtSql/QSqlQuery>
#include <QTimeZone>
#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QtConcurrent>
#include <QSqlRecord>
#include <QSet>
#include <algorithm>

#define DEFAULT_DATABASE_NAME "default_database"
#define OUTSIDE_DATABASE_NAME "outside_database"

/*!
 * \brief DBManager::DBManager
 * \param parent
 */
DBManager::DBManager(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QList<NodeData *>>("QList<NodeData*>");
    qRegisterMetaType<QVector<NodeData>>("QVector<NodeData>");
    qRegisterMetaType<NodeTagTreeData>("NodeTagTreeData");
    qRegisterMetaType<QSet<int>>("QSet<int>");
    qRegisterMetaType<ListViewInfo>("ListViewInfo");
    qRegisterMetaType<FolderListType>("DBManager::FolderListType");
}

/*!
 * \brief DBManager::open
 * \param path
 * \param doCreate
 */
void DBManager::open(const QString &path, bool doCreate)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", DEFAULT_DATABASE_NAME);
    m_dbpath = path;
    m_db.setDatabaseName(path);
    if (!m_db.open()) {
        qDebug() << "Error: connection with database fail";
    } else {
        qDebug() << "Database: connection ok";
    }

    if (doCreate) {
        createTables();
    }
    recalculateChildNotesCount();
}

/*!
 * \brief DBManager::createTables
 */
void DBManager::createTables()
{
    if (!m_db.transaction()) {
        qDebug() << "Failed to start transaction to create tables. Error: " << m_db.lastError();
        return;
    }
    QSqlQuery query(m_db);

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
                        R"(    "scrollbar_position"	INTEGER NOT NULL,)"
                        R"(    "absolute_path"	TEXT NOT NULL,)"
                        R"(    "is_pinned_note"	INTEGER NOT NULL DEFAULT 0,)"
                        R"(    "relative_position_an"	INTEGER NOT NULL,)"
                        R"(    "child_notes_count"	INTEGER NOT NULL)"
                        R"();)";
    auto status = query.exec(nodeTable);
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();
    QString tagRelationship = R"(CREATE TABLE "tag_relationship" ()"
                              R"(    "node_id"	INTEGER NOT NULL,)"
                              R"(    "tag_id"	INTEGER NOT NULL,)"
                              R"(    UNIQUE(node_id, tag_id))"
                              R"();)";
    status = query.exec(tagRelationship);
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();

    QString tagTable = R"(CREATE TABLE "tag_table" ()"
                       R"(    "id"	INTEGER NOT NULL,)"
                       R"(    "name"	TEXT NOT NULL,)"
                       R"(    "color"	TEXT NOT NULL,)"
                       R"(    "child_notes_count"	INTEGER NOT NULL,)"
                       R"(    "relative_position"	INTEGER NOT NULL)"
                       R"();)";
    status = query.exec(tagTable);
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();

    QString metadata = R"(CREATE TABLE "metadata" ()"
                       R"(    "key"	TEXT NOT NULL,)"
                       R"(    "value"	INTEGER NOT NULL)"
                       R"();)";
    status = query.exec(metadata);
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();

    if (!query.prepare(R"(INSERT INTO "metadata"("key","value") VALUES (:key, :value);)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":key", "next_node_id");
    query.bindValue(":value", 0);
    status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":key", "next_tag_id");
    query.bindValue(":value", 0);
    status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();

    NodeData rootFolder; // id 0
    rootFolder.setNodeType(NodeData::Type::Folder);
    QDateTime noteDate = QDateTime::currentDateTime();
    rootFolder.setCreationDateTime(noteDate);
    rootFolder.setLastModificationDateTime(noteDate);
    rootFolder.setFullTitle(QStringLiteral("/"));
    rootFolder.setParentId(-1);
    addNode(rootFolder);

    NodeData trashFolder; // id 1
    trashFolder.setNodeType(NodeData::Type::Folder);
    trashFolder.setCreationDateTime(noteDate);
    trashFolder.setLastModificationDateTime(noteDate);
    trashFolder.setFullTitle(QStringLiteral("Trash"));
    trashFolder.setParentId(0);
    addNode(trashFolder);

    NodeData notesFolder; // id 2
    notesFolder.setNodeType(NodeData::Type::Folder);
    notesFolder.setCreationDateTime(noteDate);
    notesFolder.setLastModificationDateTime(noteDate);
    notesFolder.setFullTitle(QStringLiteral("Notes"));
    notesFolder.setParentId(0);
    addNode(notesFolder);

    if (!m_db.commit()) {
        qDebug() << "Failed to commit transaction to create tables. Error: " << m_db.lastError();
    }
}

/*!
 * \brief DBManager::isNoteExist
 * \param note
 * \return
 */
bool DBManager::isNodeExist(const NodeData &node)
{
    QSqlQuery query(m_db);

    int id = node.id();
    QString queryStr = QStringLiteral("SELECT EXISTS(SELECT 1 FROM node_table WHERE id = :id LIMIT 1 )");
    if (!query.prepare(queryStr)) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":id", id);
    bool status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError() << query.isValid();
    }
    if (!query.next()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    return query.value(0).toInt() == 1;
}

QVector<NodeData> DBManager::getAllFolders()
{
    QSqlQuery query(m_db);
    if (!query.prepare(R"(SELECT)"
                       R"("id",)"
                       R"("title",)"
                       R"("creation_date",)"
                       R"("modification_date",)"
                       R"("deletion_date",)"
                       R"("content",)"
                       R"("node_type",)"
                       R"("parent_id",)"
                       R"("relative_position",)"
                       R"("absolute_path", )"
                       R"("child_notes_count" )"
                       R"(FROM node_table WHERE node_type=:node_type;)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":node_type", static_cast<int>(NodeData::Type::Folder));

    QVector<NodeData> nodeList;
    bool status = query.exec();
    if (status) {
        while (query.next()) {
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
            node.setChildNotesCount(query.value(10).toInt());
            if (node.nodeType() == NodeData::Type::Note) {
                node.setTagIds(getAllTagForNote(node.id()));
            }
            nodeList.append(node);
        }
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }

    return nodeList;
}

QVector<TagData> DBManager::getAllTagInfo()
{
    QVector<TagData> tagList;

    QSqlQuery query(m_db);
    if (!query.prepare(R"(SELECT "id","name","color","relative_position","child_notes_count" FROM tag_table;)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    bool status = query.exec();
    if (status) {
        while (query.next()) {
            TagData tag;
            tag.setId(query.value(0).toInt());
            tag.setName(query.value(1).toString());
            tag.setColor(query.value(2).toString());
            tag.setRelativePosition(query.value(3).toInt());
            tag.setChildNotesCount(query.value(4).toInt());
            tagList.append(tag);
        }
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }

    return tagList;
}

QSet<int> DBManager::getAllTagForNote(int noteId)
{
    QSet<int> tagIds;
    QSqlQuery query(m_db);
    if (!query.prepare(R"(SELECT "tag_id" FROM tag_relationship WHERE node_id = :node_id;)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":node_id", noteId);
    bool status = query.exec();
    if (status) {
        while (query.next()) {
            tagIds.insert(query.value(0).toInt());
        }
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    return tagIds;
}

int DBManager::addNode(const NodeData &node)
{
    QSqlQuery query(m_db);
    QString emptyStr;

    qint64 epochTimeDateCreated = node.creationDateTime().toMSecsSinceEpoch();
    QString content = node.content();
    content.replace("'", "''").replace(QChar('\x0'), emptyStr);

    QString fullTitle = node.fullTitle();
    fullTitle.replace("'", "''").replace(QChar('\x0'), emptyStr);

    qint64 epochTimeDateLastModified = node.lastModificationdateTime().isNull() ? epochTimeDateCreated : node.lastModificationdateTime().toMSecsSinceEpoch();

    int relationalPosition = 0;
    if (node.parentId() != -1) {
        if (!query.prepare(R"(SELECT relative_position FROM "node_table" )"
                           R"(WHERE parent_id = :parent_id AND node_type = :node_type;)")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(":parent_id", node.parentId());
        query.bindValue(":node_type", static_cast<int>(node.nodeType()));
        bool status = query.exec();
        if (status) {
            while (query.next()) {
                if (relationalPosition <= query.value(0).toInt()) {
                    relationalPosition = query.value(0).toInt() + 1;
                }
            }
        } else {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.finish();
    }
    int nodeId = nextAvailableNodeId();
    QString absolutePath;
    if (node.parentId() != -1) {
        absolutePath = getNodeAbsolutePath(node.parentId()).path();
    }
    absolutePath += PATH_SEPARATOR + QString::number(nodeId);
    QString queryStr =
            R"(INSERT INTO "node_table")"
            R"(("id", "title", "creation_date", "modification_date", "deletion_date", "content", "node_type", "parent_id", "relative_position", "scrollbar_position", "absolute_path", "is_pinned_note", "relative_position_an", "child_notes_count"))"
            R"(VALUES (:id, :title, :creation_date, :modification_date, :deletion_date, :content, :node_type, :parent_id, :relative_position, :scrollbar_position, :absolute_path, :is_pinned_note, :relative_position_an, :child_notes_count);)";

    if (!query.prepare(queryStr)) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":id", nodeId);
    query.bindValue(":title", fullTitle);
    query.bindValue(":creation_date", epochTimeDateCreated);
    query.bindValue(":modification_date", epochTimeDateLastModified);
    if (node.deletionDateTime().isNull()) {
        query.bindValue(":deletion_date", -1);
    } else {
        query.bindValue(":deletion_date", node.deletionDateTime().toMSecsSinceEpoch());
    }
    query.bindValue(":content", content);
    query.bindValue(":node_type", static_cast<int>(node.nodeType()));
    query.bindValue(":parent_id", node.parentId());
    query.bindValue(":relative_position", relationalPosition);
    query.bindValue(":scrollbar_position", node.scrollBarPosition());
    query.bindValue(":absolute_path", absolutePath);
    query.bindValue(":is_pinned_note", node.isPinnedNote() ? 1 : 0);
    query.bindValue(":relative_position_an", node.relativePosAN());
    query.bindValue(":child_notes_count", node.childNotesCount());

    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.finish();

    if (!query.prepare(R"(UPDATE "metadata" SET "value"=:value WHERE "key"='next_node_id';)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":value", nodeId + 1);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    if (node.nodeType() == NodeData::Type::Note) {
        increaseChildNotesCountFolder(node.parentId());
        increaseChildNotesCountFolder(ROOT_FOLDER_ID);
    }
    return nodeId;
}

int DBManager::addNodePreComputed(const NodeData &node)
{
    QSqlQuery query(m_db);
    QString emptyStr;

    qint64 epochTimeDateCreated = node.creationDateTime().toMSecsSinceEpoch();
    QString content = node.content();
    content.replace("'", "''").replace(QChar('\x0'), emptyStr);
    QString fullTitle = node.fullTitle();
    fullTitle.replace("'", "''").replace(QChar('\x0'), emptyStr);

    qint64 epochTimeDateLastModified = node.lastModificationdateTime().isNull() ? epochTimeDateCreated : node.lastModificationdateTime().toMSecsSinceEpoch();

    int relationalPosition = node.relativePosition();
    int nodeId = node.id();
    QString queryStr =
            R"(INSERT INTO "node_table" )"
            R"(("id", "title", "creation_date", "modification_date", "deletion_date", "content", "node_type", "parent_id", "relative_position", "scrollbar_position", "absolute_path", "is_pinned_note", "relative_position_an", "child_notes_count") )"
            R"(VALUES (:id, :title, :creation_date, :modification_date, :deletion_date, :content, :node_type, :parent_id, :relative_position, :scrollbar_position, :absolute_path, :is_pinned_note, :relative_position_an, :child_notes_count);)";

    if (!query.prepare(queryStr)) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":id", nodeId);
    query.bindValue(":title", fullTitle);
    query.bindValue(":creation_date", epochTimeDateCreated);
    query.bindValue(":modification_date", epochTimeDateLastModified);
    if (node.deletionDateTime().isNull()) {
        query.bindValue(":deletion_date", -1);
    } else {
        query.bindValue(":deletion_date", node.deletionDateTime().toMSecsSinceEpoch());
    }
    query.bindValue(":content", content);
    query.bindValue(":node_type", static_cast<int>(node.nodeType()));
    query.bindValue(":parent_id", node.parentId());
    query.bindValue(":relative_position", relationalPosition);
    query.bindValue(":scrollbar_position", node.scrollBarPosition());
    query.bindValue(":absolute_path", node.absolutePath());
    query.bindValue(":is_pinned_note", node.isPinnedNote() ? 1 : 0);
    query.bindValue(":relative_position_an", node.relativePosAN());
    query.bindValue(":child_notes_count", node.childNotesCount());

    bool status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError() << query.isValid();
    }

    query.finish();

    return nodeId;
}

void DBManager::recalculateChildNotesCount()
{
    QSet<int> tagIds;
    QSqlQuery query(m_db);
    if (!query.prepare(R"(SELECT id FROM "tag_table")")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    bool status = query.exec();
    if (status) {
        while (query.next()) {
            tagIds.insert(query.value(0).toInt());
        }
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();
    for (const auto &id : std::as_const(tagIds)) {
        if (!query.prepare("SELECT count(*) FROM tag_relationship WHERE tag_id=:id")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":id"), id);
        status = query.exec();
        int childNotesCount = 0;
        if (status) {
            query.next();
            childNotesCount = query.value(0).toInt();
        } else {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.clear();
        if (!query.prepare(QStringLiteral("UPDATE tag_table SET child_notes_count = :child_notes_count "
                                          "WHERE id = :id"))) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":id"), id);
        query.bindValue(QStringLiteral(":child_notes_count"), childNotesCount);
        status = query.exec();
        if (!status) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        emit childNotesCountUpdatedTag(id, childNotesCount);
    }
    query.clear();
    QMap<int, QString> folderIds;
    if (!query.prepare(R"(SELECT id, absolute_path )"
                       R"(FROM node_table WHERE node_type=:node_type;)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":node_type", static_cast<int>(NodeData::Type::Folder));
    status = query.exec();
    if (status) {
        while (query.next()) {
            auto id = query.value(0).toInt();
            if (id != ROOT_FOLDER_ID) {
                folderIds[id] = query.value(1).toString();
            }
        }
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    for (const auto &id : folderIds.keys()) {
        if (!query.prepare(R"(SELECT count(*) FROM node_table )"
                           R"(WHERE node_type = (:node_type) AND parent_id = (:parent_id);)")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":parent_id"), id);
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
        status = query.exec();
        int childNotesCount = 0;
        if (status) {
            query.next();
            childNotesCount = query.value(0).toInt();
        } else {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.clear();
        if (!query.prepare(QStringLiteral("UPDATE node_table SET child_notes_count = :child_notes_count "
                                          "WHERE id = :id"))) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":id"), id);
        query.bindValue(QStringLiteral(":child_notes_count"), childNotesCount);
        status = query.exec();
        if (!status) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        emit childNotesCountUpdatedFolder(id, folderIds[id], childNotesCount);
    }
    recalculateChildNotesCountAllNotes();
}

void DBManager::recalculateChildNotesCountFolder(int folderId)
{
    QSqlQuery query(m_db);
    if (!query.prepare(R"(SELECT count(*) FROM node_table )"
                       R"(WHERE node_type = (:node_type) AND parent_id = (:parent_id);)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":parent_id"), folderId);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
    bool status = query.exec();
    int childNotesCount = 0;
    if (status) {
        query.next();
        childNotesCount = query.value(0).toInt();
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();
    if (!query.prepare(QStringLiteral("UPDATE node_table SET child_notes_count = :child_notes_count "
                                      "WHERE id = :id"))) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), folderId);
    query.bindValue(QStringLiteral(":child_notes_count"), childNotesCount);
    status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    emit childNotesCountUpdatedFolder(folderId, getNodeAbsolutePath(folderId).path(), childNotesCount);
}

void DBManager::recalculateChildNotesCountTag(int tagId)
{
    QSqlQuery query(m_db);
    if (!query.prepare("SELECT count(*) FROM tag_relationship WHERE tag_id=:id")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), tagId);
    bool status = query.exec();
    int childNotesCount = 0;
    if (status) {
        query.next();
        childNotesCount = query.value(0).toInt();
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();
    if (!query.prepare(QStringLiteral("UPDATE tag_table SET child_notes_count = :child_notes_count "
                                      "WHERE id = :id"))) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), tagId);
    query.bindValue(QStringLiteral(":child_notes_count"), childNotesCount);
    status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    emit childNotesCountUpdatedTag(tagId, childNotesCount);
}

void DBManager::recalculateChildNotesCountAllNotes()
{
    QSqlQuery query(m_db);
    if (!query.prepare(R"(SELECT count(*) FROM node_table )"
                       R"(WHERE node_type = (:node_type) AND parent_id != (:parent_id);)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
    query.bindValue(QStringLiteral(":parent_id"), static_cast<int>(TRASH_FOLDER_ID));
    bool status = query.exec();
    int childNotesCount = 0;
    if (status) {
        query.next();
        childNotesCount = query.value(0).toInt();
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();
    if (!query.prepare(QStringLiteral("UPDATE node_table SET child_notes_count = :child_notes_count "
                                      "WHERE id = :id"))) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), ROOT_FOLDER_ID);
    query.bindValue(QStringLiteral(":child_notes_count"), childNotesCount);
    status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    emit childNotesCountUpdatedFolder(ROOT_FOLDER_ID, getNodeAbsolutePath(ROOT_FOLDER_ID).path(), childNotesCount);
}

void DBManager::increaseChildNotesCountTag(int tagId)
{
    QSqlQuery query(m_db);
    if (!query.prepare(R"(SELECT child_notes_count FROM "tag_table" WHERE id=:id)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), tagId);
    bool status = query.exec();
    int childNotesCount = 0;
    if (status) {
        query.next();
        childNotesCount = query.value(0).toInt();
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        return;
    }
    query.clear();
    childNotesCount += 1;

    if (!query.prepare(QStringLiteral("UPDATE tag_table SET child_notes_count = :child_notes_count "
                                      "WHERE id = :id"))) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), tagId);
    query.bindValue(QStringLiteral(":child_notes_count"), childNotesCount);
    status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    emit childNotesCountUpdatedTag(tagId, childNotesCount);
}

void DBManager::decreaseChildNotesCountTag(int tagId)
{
    QSqlQuery query(m_db);
    if (!query.prepare(R"(SELECT child_notes_count FROM "tag_table" WHERE id=:id)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), tagId);
    bool status = query.exec();
    int childNoteCount = 0;
    if (status) {
        query.next();
        childNoteCount = query.value(0).toInt();
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        return;
    }
    query.clear();
    childNoteCount -= 1;
    childNoteCount = std::max(childNoteCount, 0);

    if (!query.prepare(QStringLiteral("UPDATE tag_table SET child_notes_count = :child_notes_count "
                                      "WHERE id = :id"))) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), tagId);
    query.bindValue(QStringLiteral(":child_notes_count"), childNoteCount);
    status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    emit childNotesCountUpdatedTag(tagId, childNoteCount);
}

void DBManager::increaseChildNotesCountFolder(int folderId)
{
    QSqlQuery query(m_db);
    if (!query.prepare(R"(SELECT child_notes_count, absolute_path  FROM "node_table" WHERE id=:id)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), folderId);
    bool status = query.exec();
    int childNotesCount = 0;
    QString absPath;
    if (status) {
        query.next();
        childNotesCount = query.value(0).toInt();
        absPath = query.value(1).toString();
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        return;
    }
    query.clear();
    childNotesCount += 1;

    if (!query.prepare(QStringLiteral("UPDATE node_table SET child_notes_count = :child_notes_count "
                                      "WHERE id = :id"))) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), folderId);
    query.bindValue(QStringLiteral(":child_notes_count"), childNotesCount);
    status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    emit childNotesCountUpdatedFolder(folderId, absPath, childNotesCount);
}

void DBManager::decreaseChildNotesCountFolder(int folderId)
{
    QSqlQuery query(m_db);
    if (!query.prepare(R"(SELECT child_notes_count, absolute_path  FROM "node_table" WHERE id=:id)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), folderId);
    bool status = query.exec();
    int childNotesCount = 0;
    QString absPath;
    if (status) {
        query.next();
        childNotesCount = query.value(0).toInt();
        absPath = query.value(1).toString();
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        return;
    }
    query.clear();
    childNotesCount -= 1;
    childNotesCount = std::max(childNotesCount, 0);

    if (!query.prepare(QStringLiteral("UPDATE node_table SET child_notes_count = :child_notes_count "
                                      "WHERE id = :id"))) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), folderId);
    query.bindValue(QStringLiteral(":child_notes_count"), childNotesCount);
    status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    emit childNotesCountUpdatedFolder(folderId, absPath, childNotesCount);
}

int DBManager::addTag(const TagData &tag)
{
    QSqlQuery query(m_db);

    int relationalPosition = 0;
    if (!query.prepare(R"(SELECT relative_position FROM "tag_table" )")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    bool status = query.exec();
    if (status) {
        while (query.next()) {
            if (relationalPosition <= query.value(0).toInt()) {
                relationalPosition = query.value(0).toInt() + 1;
            }
        }
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.finish();
    int id = nextAvailableTagId();

    QString queryStr = R"(INSERT INTO "tag_table" )"
                       R"(("id","name","color","relative_position","child_notes_count") )"
                       R"(VALUES (:id, :name, :color, :relative_position, :child_notes_count);)";
    if (!query.prepare(queryStr)) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":id", id);
    query.bindValue(":name", tag.name());
    query.bindValue(":color", tag.color());
    query.bindValue(":relative_position", relationalPosition);
    query.bindValue(":child_notes_count", tag.childNotesCount());
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.finish();

    if (!query.prepare(R"(UPDATE "metadata" SET "value"=:value WHERE "key"='next_tag_id';)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
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
    QSqlQuery query(m_db);
    if (!query.prepare(R"(INSERT OR IGNORE INTO "tag_relationship" ("node_id","tag_id") VALUES (:note_id, :tag_id);)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":note_id", noteId);
    query.bindValue(":tag_id", tagId);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    recalculateChildNotesCountTag(tagId);
}

void DBManager::removeNoteFromTag(int noteId, int tagId)
{
    QSqlQuery query(m_db);
    if (!query.prepare(R"(DELETE FROM "tag_relationship" )"
                       R"(WHERE node_id = (:note_id) AND tag_id = (:tag_id);)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":note_id", noteId);
    query.bindValue(":tag_id", tagId);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    decreaseChildNotesCountTag(tagId);
}

int DBManager::nextAvailableNodeId()
{
    QSqlQuery query(m_db);
    if (!query.prepare("SELECT value FROM metadata WHERE key = :key")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
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
    QSqlQuery query(m_db);
    if (!query.prepare("SELECT value FROM metadata WHERE key = :key")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
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
    QSqlQuery query(m_db);
    if (!query.prepare(R"(UPDATE "node_table" SET "title"=:title WHERE "id"=:id;)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":title", newName);
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

void DBManager::renameTag(int id, const QString &newName)
{
    QSqlQuery query(m_db);
    if (!query.prepare(R"(UPDATE "tag_table" SET "name"=:name WHERE "id"=:id;)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":name", newName);
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    emit tagRenamed(id, newName);
}

void DBManager::changeTagColor(int id, const QString &newColor)
{
    QSqlQuery query(m_db);
    if (!query.prepare(R"(UPDATE "tag_table" SET "color"=:color WHERE "id"=:id;)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":color", newColor);
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    emit tagColorChanged(id, newColor);
}

/*!
 * \brief DBManager::removeNote
 * \param note
 * \return
 */
void DBManager::removeNote(const NodeData &note)
{
    if (note.parentId() == TRASH_FOLDER_ID) {
        QSqlQuery query(m_db);
        if (!query.prepare(R"(DELETE FROM "node_table" )"
                           R"(WHERE id = (:id) AND node_type = (:node_type);)")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":id"), note.id());
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
        if (!query.exec()) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.clear();
        if (!query.prepare(R"(DELETE FROM "tag_relationship" )"
                           R"(WHERE node_id = (:id);)")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":id"), note.id());
        if (!query.exec()) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        if (note.nodeType() == NodeData::Type::Note) {
            decreaseChildNotesCountFolder(TRASH_FOLDER_ID);
        }
    } else {
        auto trashFolder = getNode(TRASH_FOLDER_ID);
        moveNode(note.id(), trashFolder);
    }
}

void DBManager::removeTag(int tagId)
{
    QSqlQuery query(m_db);
    if (!query.prepare(R"(DELETE FROM "tag_table" )"
                       R"(WHERE id = (:id);)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), tagId);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();
    if (!query.prepare(R"(DELETE FROM "tag_relationship" )"
                       R"(WHERE tag_id = (:id);)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), tagId);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    emit tagRemoved(tagId);
}

/*!
 * \brief DBManager::updateNote
 * \param note
 * \return
 */
bool DBManager::updateNoteContent(const NodeData &note)
{
    QSqlQuery query(m_db);
    QString emptyStr;

    int id = note.id();
    if (id == INVALID_NODE_ID) {
        qDebug() << "Invalid Note ID";
        return false;
    }
    qint64 epochTimeDateModified = note.lastModificationdateTime().toMSecsSinceEpoch();
    QString content = note.content();
    content.replace(QChar('\x0'), emptyStr);
    QString fullTitle = note.fullTitle();
    fullTitle.replace(QChar('\x0'), emptyStr);

    if (!query.prepare(QStringLiteral("UPDATE node_table SET modification_date = :modification_date, content = :content, "
                                      "title = :title, scrollbar_position = :scrollbar_position WHERE id = :id AND node_type "
                                      "= :node_type;"))) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":modification_date"), epochTimeDateModified);
    query.bindValue(QStringLiteral(":content"), content);
    query.bindValue(QStringLiteral(":title"), fullTitle);
    query.bindValue(QStringLiteral(":id"), id);
    query.bindValue(QStringLiteral(":scrollbar_position"), note.scrollBarPosition());
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    return (query.numRowsAffected() == 1);
}

QList<NodeData> DBManager::readOldNBK(const QString &fileName)
{
    QList<NodeData> noteList;
    {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_5_6);

        try {
            in >> noteList;
        } catch (...) {
            // Any exception deserializing will result in an empty note list and  the user will be
            // notified
        }
        file.close();
    }
    if (noteList.isEmpty()) {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_5_6);

        QList<NodeData *> nl;
        try {
            in >> nl;
        } catch (...) {
            // Any exception deserializing will result in an empty note list and  the user will be
            // notified
        }
        if (!nl.isEmpty()) {
            for (const auto &n : std::as_const(nl)) {
                noteList.append(*n);
            }
        }
        qDeleteAll(nl);
        file.close();
    }
    return noteList;
}

int DBManager::nextAvailablePosition(int parentId, NodeData::Type nodeType)
{
    QSqlQuery query(m_db);
    int relationalPosition = 0;
    if (parentId != -1) {
        if (!query.prepare(R"(SELECT relative_position FROM "node_table" )"
                           R"(WHERE parent_id = :parent_id AND node_type = :node_type;)")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(":parent_id", parentId);
        query.bindValue(":node_type", static_cast<int>(nodeType));
        bool status = query.exec();
        if (status) {
            while (query.next()) {
                if (relationalPosition <= query.value(0).toInt()) {
                    relationalPosition = query.value(0).toInt() + 1;
                }
            }
        } else {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.finish();
    }
    return relationalPosition;
}

NodePath DBManager::getNodeAbsolutePath(int nodeId)
{
    QSqlQuery query(m_db);
    if (!query.prepare("SELECT absolute_path FROM node_table WHERE id = :id")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":id", nodeId);
    bool status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError() << query.isValid();
    }
    query.last();
    auto absolutePath = query.value(0).toString();
    return absolutePath;
}

NodeData DBManager::getNode(int nodeId)
{
    QSqlQuery query(m_db);
    if (!query.prepare(R"(SELECT)"
                       R"("id",)"
                       R"("title",)"
                       R"("creation_date",)"
                       R"("modification_date",)"
                       R"("deletion_date",)"
                       R"("content",)"
                       R"("node_type",)"
                       R"("parent_id",)"
                       R"("relative_position",)"
                       R"("scrollbar_position",)"
                       R"("absolute_path", )"
                       R"("is_pinned_note", )"
                       R"("relative_position_an", )"
                       R"("child_notes_count" )"
                       R"(FROM node_table WHERE id=:id LIMIT 1;)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":id", nodeId);
    bool status = query.exec();
    if (status) {
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
        node.setScrollBarPosition(query.value(9).toInt());
        node.setAbsolutePath(query.value(10).toString());
        node.setIsPinnedNote(static_cast<bool>(query.value(11).toInt()));
        node.setRelativePosAN(query.value(13).toInt());
        node.setRelativePosAN(query.value(14).toInt());
        if (node.nodeType() == NodeData::Type::Note) {
            node.setTagIds(getAllTagForNote(node.id()));
            QSqlQuery query2(m_db);
            if (!query2.prepare(R"(SELECT)"
                                R"("title" )"
                                R"(FROM node_table WHERE id=:id LIMIT 1;)")) {
                qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
            }
            query2.bindValue(":id", node.parentId());
            if (query2.exec()) {
                query2.next();
                node.setParentName(query2.value(0).toString());
            } else {
                qDebug() << __FUNCTION__ << __LINE__ << query2.lastError();
            }
        }
        return node;
    }
    qDebug() << "Can't find node with id" << nodeId << ": " << query.lastError();
    return NodeData();
}

void DBManager::moveFolderToTrash(const NodeData &node)
{
    QSqlQuery query(m_db);
    QString parentPath = node.absolutePath() + PATH_SEPARATOR;
    if (!query.prepare(R"(SELECT id FROM "node_table" )"
                       R"(WHERE absolute_path like (:path_expr) || '%' AND node_type = (:node_type);)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":path_expr"), parentPath);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
    bool status = query.exec();
    QSet<int> childIds;
    if (status) {
        while (query.next()) {
            childIds.insert(query.value(0).toInt());
        }
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();
    auto trashFolder = getNode(TRASH_FOLDER_ID);
    for (const auto &id : childIds) {
        moveNode(id, trashFolder);
    }
    if (!query.prepare(R"(DELETE FROM "node_table" )"
                       R"(WHERE absolute_path like (:path_expr) || '%' AND node_type = (:node_type);)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":path_expr"), parentPath);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Folder));
    status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.clear();
    if (!query.prepare(R"(DELETE FROM "node_table" )"
                       R"(WHERE absolute_path like (:path_expr) AND node_type = (:node_type);)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":path_expr"), node.absolutePath());
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Folder));
    status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

FolderListType DBManager::getFolderList()
{
    QMap<int, QString> result;
    QSqlQuery query(m_db);
    if (!query.prepare(R"(SELECT "id", "title" FROM node_table WHERE id > 0 AND node_type = :node_type;)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":node_type", static_cast<int>(NodeData::Type::Folder));
    bool status = query.exec();
    if (status) {
        while (query.next()) {
            result[query.value(0).toInt()] = query.value(1).toString();
        }
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    return result;
}

void DBManager::moveNode(int nodeId, const NodeData &target)
{
    if (target.nodeType() != NodeData::Type::Folder) {
        qDebug() << "moveNode target is not folder" << target.id();
        return;
    }
    QSqlQuery query(m_db);
    auto node = getNode(nodeId);

    QString newAbsolutePath = QStringLiteral("%1%2%3").arg(target.absolutePath(), PATH_SEPARATOR).arg(nodeId);
    if (target.id() == TRASH_FOLDER_ID) {
        qint64 deletionTime = QDateTime::currentMSecsSinceEpoch();
        if (!query.prepare(QStringLiteral("UPDATE node_table SET parent_id = :parent_id, absolute_path = :absolute_path, "
                                          "is_pinned_note = :is_pinned_note, deletion_date = :deletion_date "
                                          "WHERE id = :id;"))) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":parent_id"), target.id());
        query.bindValue(QStringLiteral(":absolute_path"), newAbsolutePath);
        query.bindValue(QStringLiteral(":is_pinned_note"), false);
        query.bindValue(QStringLiteral(":deletion_date"), deletionTime);
        query.bindValue(QStringLiteral(":id"), nodeId);
    } else {
        if (!query.prepare(QStringLiteral("UPDATE node_table SET parent_id = :parent_id, absolute_path = :absolute_path "
                                          "WHERE id = :id;"))) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":parent_id"), target.id());
        query.bindValue(QStringLiteral(":absolute_path"), newAbsolutePath);
        query.bindValue(QStringLiteral(":id"), nodeId);
    }
    bool status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError() << query.lastQuery();
    }

    if (node.nodeType() == NodeData::Type::Folder) {
        const QString &oldAbsolutePath = node.absolutePath();
        QMap<int, QString> children;
        query.clear();
        if (!query.prepare(R"(SELECT id, absolute_path FROM "node_table" )"
                           R"(WHERE absolute_path like (:path_expr) || '%';)")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":path_expr"), oldAbsolutePath);
        if (query.exec()) {
            while (query.next()) {
                if (query.value(0).toInt() != node.id()) {
                    children[query.value(0).toInt()] = query.value(1).toString();
                }
            }
        } else {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        for (const auto id : children.keys()) {
            QString oldP = children[id];
            QString newP = oldP.replace(oldP.indexOf(oldAbsolutePath), oldAbsolutePath.size(), newAbsolutePath);
            if (target.id() == TRASH_FOLDER_ID) {
                qint64 deletionTime = QDateTime::currentMSecsSinceEpoch();
                if (!query.prepare(QStringLiteral("UPDATE node_table SET absolute_path = :absolute_path, is_pinned_note = "
                                                  ":is_pinned_note, deletion_date = :deletion_date "
                                                  "WHERE id = :id;"))) {
                    qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
                }
                query.bindValue(QStringLiteral(":absolute_path"), newP);
                query.bindValue(QStringLiteral(":id"), id);
                query.bindValue(QStringLiteral(":is_pinned_note"), false);
                query.bindValue(QStringLiteral(":deletion_date"), deletionTime);
            } else {
                if (!query.prepare(QStringLiteral("UPDATE node_table SET absolute_path = :absolute_path "
                                                  "WHERE id = :id;"))) {
                    qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
                }
                query.bindValue(QStringLiteral(":absolute_path"), newP);
                query.bindValue(QStringLiteral(":id"), id);
            }
            if (!query.exec()) {
                qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
            }
        }
        recalculateChildNotesCount();
    } else {
        decreaseChildNotesCountFolder(node.parentId());
        if (node.parentId() != TRASH_FOLDER_ID && target.id() == TRASH_FOLDER_ID) {
            decreaseChildNotesCountFolder(ROOT_FOLDER_ID);
            auto allTagInNote = getAllTagForNote(node.id());
            for (const auto &tagId : std::as_const(allTagInNote)) {
                decreaseChildNotesCountTag(tagId);
            }
        } else if (node.parentId() == TRASH_FOLDER_ID && target.id() != TRASH_FOLDER_ID) {
            increaseChildNotesCountFolder(ROOT_FOLDER_ID);
            auto allTagInNote = getAllTagForNote(node.id());
            for (const auto &tagId : std::as_const(allTagInNote)) {
                increaseChildNotesCountTag(tagId);
            }
        }
        increaseChildNotesCountFolder(target.id());
    }
}

void DBManager::searchForNotes(const QString &keyword, const ListViewInfo &inf)
{
    QVector<NodeData> nodeList;
    QSqlQuery query(m_db);
    if (!inf.isInTag && inf.parentFolderId == ROOT_FOLDER_ID) {
        if (!query.prepare(R"(SELECT )"
                           R"("id",)"
                           R"("title",)"
                           R"("creation_date",)"
                           R"("modification_date",)"
                           R"("deletion_date",)"
                           R"("content",)"
                           R"("node_type",)"
                           R"("parent_id",)"
                           R"("relative_position", )"
                           R"("scrollbar_position",)"
                           R"("absolute_path", )"
                           R"("is_pinned_note", )"
                           R"("relative_position_an", )"
                           R"("child_notes_count" )"
                           R"(FROM node_table )"
                           R"(WHERE node_type = (:node_type) AND parent_id != (:parent_id) )"
                           R"(AND content like  '%' || (:search_expr) || '%';)")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
        query.bindValue(QStringLiteral(":parent_id"), static_cast<int>(TRASH_FOLDER_ID));
        query.bindValue(QStringLiteral(":search_expr"), keyword);

        bool status = query.exec();
        if (status) {
            while (query.next()) {
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
                node.setScrollBarPosition(query.value(9).toInt());
                node.setAbsolutePath(query.value(10).toString());
                node.setIsPinnedNote(static_cast<bool>(query.value(11).toInt()));
                node.setRelativePosAN(query.value(13).toInt());
                node.setChildNotesCount(query.value(14).toInt());
                node.setTagIds(getAllTagForNote(node.id()));
                auto p = getNode(node.parentId());
                node.setParentName(p.fullTitle());
                nodeList.append(node);
            }
        } else {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
    } else if (!inf.isInTag) {
        if (!query.prepare(R"(SELECT )"
                           R"("id",)"
                           R"("title",)"
                           R"("creation_date",)"
                           R"("modification_date",)"
                           R"("deletion_date",)"
                           R"("content",)"
                           R"("node_type",)"
                           R"("parent_id",)"
                           R"("relative_position", )"
                           R"("scrollbar_position",)"
                           R"("absolute_path", )"
                           R"("is_pinned_note", )"
                           R"("relative_position_an", )"
                           R"("child_notes_count" )"
                           R"(FROM node_table )"
                           R"(WHERE node_type = (:node_type) AND parent_id == (:parent_id) )"
                           R"(AND content like  '%' || (:search_expr) || '%';)")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
        query.bindValue(QStringLiteral(":parent_id"), static_cast<int>(inf.parentFolderId));
        query.bindValue(QStringLiteral(":search_expr"), keyword);

        bool status = query.exec();
        if (status) {
            while (query.next()) {
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
                node.setScrollBarPosition(query.value(9).toInt());
                node.setAbsolutePath(query.value(10).toString());
                node.setIsPinnedNote(static_cast<bool>(query.value(11).toInt()));
                node.setRelativePosAN(query.value(13).toInt());
                node.setChildNotesCount(query.value(14).toInt());
                node.setTagIds(getAllTagForNote(node.id()));
                auto p = getNode(node.parentId());
                node.setParentName(p.fullTitle());
                nodeList.append(node);
            }
        } else {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
    } else { // inf.isInTag == true
        QVector<QSet<int>> nds;
        for (const auto &tagId : inf.currentTagList) {
            QSet<int> nd;
            if (!query.prepare(R"(SELECT "node_id" FROM tag_relationship WHERE tag_id = :tag_id;)")) {
                qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
            }
            query.bindValue(QStringLiteral(":tag_id"), tagId);
            bool status = query.exec();
            if (status) {
                while (query.next()) {
                    nd.insert(query.value(0).toInt());
                }
            } else {
                qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
            }
            nds.append(nd);
        }
        if (nds.empty()) {
            emit notesListReceived(nodeList, inf);
            return;
        }
        QSet<int> noteIds;
        int ndsId = 0;
        for (int i = 1; i < nds.size(); ++i) {
            if (nds[i].size() < nds[ndsId].size()) {
                ndsId = i;
            }
        }
        for (const int id : std::as_const(nds[ndsId])) {
            bool ok = true;
            for (int i = 0; i < nds.size(); ++i) {
                if (i == ndsId) {
                    continue;
                }
                if (!nds[i].contains(id)) {
                    ok = false;
                }
            }
            if (ok) {
                noteIds.insert(id);
            }
        }
        for (const auto &id : noteIds) {
            NodeData node = getNode(id);
            if (node.id() != INVALID_NODE_ID && node.nodeType() == NodeData::Type::Note && node.content().contains(keyword)) {
                nodeList.append(node);
            } else {
                qDebug() << __FUNCTION__ << "Note with id" << id << "is not valid";
            }
        }
    }
    ListViewInfo inf2 = inf;
    inf2.isInSearch = true;
    std::sort(nodeList.begin(), nodeList.end(),
              [](const NodeData &a, const NodeData &b) -> bool { return a.lastModificationdateTime() > b.lastModificationdateTime(); });
    emit notesListReceived(nodeList, inf2);
}

void DBManager::clearSearch(const ListViewInfo &inf)
{
    if (inf.isInTag) {
        onNotesListInTagsRequested(inf.currentTagList, inf.needCreateNewNote, inf.scrollToId);
    } else {
        if (inf.parentFolderId == ROOT_FOLDER_ID) {
            onNotesListInFolderRequested(inf.parentFolderId, true, inf.needCreateNewNote, inf.scrollToId);
        } else {
            onNotesListInFolderRequested(inf.parentFolderId, false, inf.needCreateNewNote, inf.scrollToId);
        }
    }
}

void DBManager::updateRelPosNode(int nodeId, int relPos)
{
    QSqlQuery query(m_db);
    if (!query.prepare(QStringLiteral("UPDATE node_table SET relative_position = :relative_position "
                                      "WHERE id = :id;"))) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":relative_position"), relPos);
    query.bindValue(QStringLiteral(":id"), nodeId);
    bool status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

void DBManager::updateRelPosTag(int tagId, int relPos)
{
    QSqlQuery query(m_db);
    if (!query.prepare(QStringLiteral("UPDATE tag_table SET relative_position = :relative_position "
                                      "WHERE id = :id;"))) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":relative_position"), relPos);
    query.bindValue(QStringLiteral(":id"), tagId);
    bool status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

void DBManager::updateRelPosPinnedNote(int nodeId, int relPos)
{
    QSqlQuery query(m_db);
    if (!query.prepare(QStringLiteral("UPDATE node_table SET relative_position = :relative_position "
                                      "WHERE id = :id AND node_type=:node_type;"))) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":relative_position"), relPos);
    query.bindValue(QStringLiteral(":id"), nodeId);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
    bool status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

void DBManager::updateRelPosPinnedNoteAN(int nodeId, int relPos)
{
    QSqlQuery query(m_db);
    if (!query.prepare(QStringLiteral("UPDATE node_table SET relative_position_an = :relative_position_an "
                                      "WHERE id = :id AND node_type=:node_type;"))) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":relative_position_an"), relPos);
    query.bindValue(QStringLiteral(":id"), nodeId);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
    bool status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

void DBManager::setNoteIsPinned(int noteId, bool isPinned)
{
    QSqlQuery query(m_db);
    if (!query.prepare(QStringLiteral("UPDATE node_table SET is_pinned_note = :is_pinned_note "
                                      "WHERE id = :id AND node_type=:node_type;"))) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":is_pinned_note"), isPinned);
    query.bindValue(QStringLiteral(":id"), noteId);
    query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
    bool status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

NodeData DBManager::getChildNotesCountFolder(int folderId)
{
    NodeData d;
    d.setNodeType(NodeData::Type::Folder);
    d.setId(folderId);
    QSqlQuery query(m_db);
    if (!query.prepare(R"(SELECT child_notes_count, absolute_path  FROM "node_table" WHERE id=:id)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(QStringLiteral(":id"), folderId);
    bool status = query.exec();
    int childNotesCount = 0;
    QString absPath;
    if (status) {
        query.next();
        childNotesCount = query.value(0).toInt();
        absPath = query.value(1).toString();
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    d.setChildNotesCount(childNotesCount);
    d.setAbsolutePath(absPath);
    return d;
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
void DBManager::onNotesListInFolderRequested(int parentID, bool isRecursive, bool newNote, int scrollToId)
{
    QVector<NodeData> nodeList;
    QSqlQuery query(m_db);
    if (parentID == ROOT_FOLDER_ID) {
        if (!query.prepare(R"(SELECT )"
                           R"("id",)"
                           R"("title",)"
                           R"("creation_date",)"
                           R"("modification_date",)"
                           R"("deletion_date",)"
                           R"("content",)"
                           R"("node_type",)"
                           R"("parent_id",)"
                           R"("relative_position",)"
                           R"("scrollbar_position",)"
                           R"("absolute_path", )"
                           R"("is_pinned_note", )"
                           R"("relative_position_an", )"
                           R"("child_notes_count" )"
                           R"(FROM node_table )"
                           R"(WHERE node_type = (:node_type) AND parent_id != (:parent_id);)")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
        query.bindValue(QStringLiteral(":parent_id"), static_cast<int>(TRASH_FOLDER_ID));

        bool status = query.exec();
        if (status) {
            while (query.next()) {
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
                node.setScrollBarPosition(query.value(9).toInt());
                node.setAbsolutePath(query.value(10).toString());
                node.setIsPinnedNote(static_cast<bool>(query.value(11).toInt()));
                node.setRelativePosAN(query.value(13).toInt());
                node.setChildNotesCount(query.value(14).toInt());
                node.setTagIds(getAllTagForNote(node.id()));
                auto p = getNode(node.parentId());
                node.setParentName(p.fullTitle());
                nodeList.append(node);
            }
        } else {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
    } else if (!isRecursive) {
        if (!query.prepare(R"(SELECT )"
                           R"("id",)"
                           R"("title",)"
                           R"("creation_date",)"
                           R"("modification_date",)"
                           R"("deletion_date",)"
                           R"("content",)"
                           R"("node_type",)"
                           R"("parent_id",)"
                           R"("relative_position", )"
                           R"("scrollbar_position",)"
                           R"("absolute_path", )"
                           R"("is_pinned_note", )"
                           R"("relative_position_an", )"
                           R"("child_notes_count" )"
                           R"(FROM node_table )"
                           R"(WHERE parent_id = (:parent_id) AND node_type = (:node_type);)")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":parent_id"), parentID);
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));
        bool status = query.exec();
        if (status) {
            while (query.next()) {
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
                node.setScrollBarPosition(query.value(9).toInt());
                node.setAbsolutePath(query.value(10).toString());
                node.setIsPinnedNote(static_cast<bool>(query.value(11).toInt()));
                node.setRelativePosAN(query.value(13).toInt());
                node.setChildNotesCount(query.value(14).toInt());
                node.setTagIds(getAllTagForNote(node.id()));
                nodeList.append(node);
            }
        } else {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
    } else {
        auto parentPath = getNodeAbsolutePath(parentID).path() + PATH_SEPARATOR;
        if (!query.prepare(R"(SELECT )"
                           R"("id",)"
                           R"("title",)"
                           R"("creation_date",)"
                           R"("modification_date",)"
                           R"("deletion_date",)"
                           R"("content",)"
                           R"("node_type",)"
                           R"("parent_id",)"
                           R"("relative_position", )"
                           R"("scrollbar_position",)"
                           R"("absolute_path", )"
                           R"("is_pinned_note", )"
                           R"("relative_position_an", )"
                           R"("child_notes_count" )"
                           R"(FROM node_table )"
                           R"(WHERE absolute_path like (:path_expr) || '%' AND node_type = (:node_type);)")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":path_expr"), parentPath);
        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Note));

        bool status = query.exec();
        if (status) {
            while (query.next()) {
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
                node.setScrollBarPosition(query.value(9).toInt());
                node.setAbsolutePath(query.value(10).toString());
                node.setIsPinnedNote(static_cast<bool>(query.value(11).toInt()));
                node.setRelativePosAN(query.value(13).toInt());
                node.setChildNotesCount(query.value(14).toInt());
                node.setTagIds(getAllTagForNote(node.id()));
                nodeList.append(node);
            }
        } else {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
    }
    ListViewInfo inf;
    inf.isInSearch = false;
    inf.isInTag = false;
    inf.parentFolderId = parentID;
    inf.currentNotesId = { INVALID_NODE_ID };
    inf.needCreateNewNote = newNote;
    inf.scrollToId = scrollToId;
    std::sort(nodeList.begin(), nodeList.end(),
              [](const NodeData &a, const NodeData &b) -> bool { return a.lastModificationdateTime() > b.lastModificationdateTime(); });
    emit notesListReceived(nodeList, inf);
}

void DBManager::onNotesListInTagsRequested(const QSet<int> &tagIds, bool newNote, int scrollToId)
{
    QVector<NodeData> nodeList;
    QVector<QSet<int>> nds;
    ListViewInfo inf;
    inf.isInSearch = false;
    inf.isInTag = true;
    inf.currentTagList = tagIds;
    inf.currentNotesId = { INVALID_NODE_ID };
    inf.needCreateNewNote = newNote;
    inf.scrollToId = scrollToId;
    for (const auto &tagId : tagIds) {
        QSet<int> nd;
        QSqlQuery query(m_db);
        if (!query.prepare(R"(SELECT "node_id" FROM tag_relationship WHERE tag_id = :tag_id;)")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.bindValue(QStringLiteral(":tag_id"), tagId);
        bool status = query.exec();
        if (status) {
            while (query.next()) {
                nd.insert(query.value(0).toInt());
            }
        } else {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        nds.append(nd);
    }
    if (nds.empty()) {
        emit notesListReceived(nodeList, inf);
        return;
    }
    QSet<int> noteIds;
    int ndsId = 0;
    for (int i = 1; i < nds.size(); ++i) {
        if (nds[i].size() < nds[ndsId].size()) {
            ndsId = i;
        }
    }
    for (const int id : std::as_const(nds[ndsId])) {
        bool ok = true;
        for (int i = 0; i < nds.size(); ++i) {
            if (i == ndsId) {
                continue;
            }
            if (!nds[i].contains(id)) {
                ok = false;
            }
        }
        if (ok) {
            noteIds.insert(id);
        }
    }
    for (const auto &id : noteIds) {
        NodeData node = getNode(id);
        if (node.id() != INVALID_NODE_ID && node.nodeType() == NodeData::Type::Note) {
            nodeList.append(node);
        } else {
            qDebug() << __FUNCTION__ << "Note with id" << id << "is not valid";
        }
    }
    std::sort(nodeList.begin(), nodeList.end(),
              [](const NodeData &a, const NodeData &b) -> bool { return a.lastModificationdateTime() > b.lastModificationdateTime(); });
    emit notesListReceived(nodeList, inf);
}

/*!
 * \brief DBManager::onOpenDBManagerRequested
 * \param path
 * \param doCreate
 */
void DBManager::onOpenDBManagerRequested(const QString &path, bool doCreate)
{
    open(path, doCreate);
}

/*!
 * \brief DBManager::onCreateUpdateRequested
 * \param note
 */
void DBManager::onCreateUpdateRequestedNoteContent(const NodeData &note)
{
    if (note.nodeType() != NodeData::Type::Note) {
        qDebug() << "Wrong node type";
        return;
    }
    bool exists = isNodeExist(note);

    if (exists) {
        updateNoteContent(note);
    } else {
        addNode(note);
    }
}

/*!
 * \brief DBManager::onImportNotesRequested
 * \param noteList
 */
void DBManager::onImportNotesRequested(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << __FUNCTION__ << __LINE__ << "fail to open file";
        return;
    }
    auto const magicHeader = file.read(16);
    file.close();
    if (QString::fromUtf8(magicHeader).startsWith(QStringLiteral("SQLite format 3"))) {
        qDebug() << __FUNCTION__;
        auto outsideDatabase = QSqlDatabase::addDatabase("QSQLITE", OUTSIDE_DATABASE_NAME);
        outsideDatabase.setDatabaseName(fileName);
        if (!outsideDatabase.open()) {
            qDebug() << __FUNCTION__ << "Error: connection with database fail";
            return;
        }
        qDebug() << __FUNCTION__ << "Database: connection ok";

        QMap<int, int> folderIdMap;
        QMap<int, int> noteIdMap;
        QMap<int, int> tagIdMap;
        {
            QVector<TagData> tagList;
            QSqlQuery outQuery(outsideDatabase);
            if (!outQuery.prepare(R"(SELECT "id","name","color","relative_position","child_notes_count" FROM tag_table;)")) {
                qDebug() << __FUNCTION__ << __LINE__ << outQuery.lastError();
            }
            bool status = outQuery.exec();
            if (status) {
                while (outQuery.next()) {
                    TagData tag;
                    tag.setId(outQuery.value(0).toInt());
                    tag.setName(outQuery.value(1).toString());
                    tag.setColor(outQuery.value(2).toString());
                    tag.setRelativePosition(outQuery.value(3).toInt());
                    tag.setChildNotesCount(outQuery.value(4).toInt());
                    tagList.append(tag);
                }
            } else {
                qDebug() << __FUNCTION__ << __LINE__ << outQuery.lastError();
            }
            outQuery.finish();
            std::sort(tagList.begin(), tagList.end(), [](auto const &a, auto const &b) { return a.relativePosition() < b.relativePosition(); });
            for (const auto &tag : std::as_const(tagList)) {
                QSqlQuery query(m_db);
                if (!query.prepare(R"(SELECT "id" FROM tag_table WHERE name = :name AND color = :color;)")) {
                    qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
                }
                query.bindValue(QStringLiteral(":name"), tag.name());
                query.bindValue(QStringLiteral(":color"), tag.color());
                QVector<int> ids;
                if (query.exec()) {
                    while (query.next()) {
                        ids.append(query.value(0).toInt());
                    }
                } else {
                    qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
                }
                query.finish();
                if (ids.isEmpty()) {
                    ids.append(addTag(tag));
                }
                tagIdMap[tag.id()] = ids[0];
            }
        }
        {
            folderIdMap[ROOT_FOLDER_ID] = ROOT_FOLDER_ID;
            folderIdMap[TRASH_FOLDER_ID] = TRASH_FOLDER_ID;
            folderIdMap[DEFAULT_NOTES_FOLDER_ID] = DEFAULT_NOTES_FOLDER_ID;
            QSqlQuery outQuery(outsideDatabase);
            if (!outQuery.prepare(R"(SELECT)"
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
                                  R"(FROM node_table WHERE node_type=:node_type;)")) {
                qDebug() << __FUNCTION__ << __LINE__ << outQuery.lastError();
            }
            outQuery.bindValue(":node_type", static_cast<int>(NodeData::Type::Folder));
            bool status = outQuery.exec();
            QMap<int, NodeData> nodeList;
            if (status) {
                while (outQuery.next()) {
                    NodeData node;
                    node.setId(outQuery.value(0).toInt());
                    node.setFullTitle(outQuery.value(1).toString());
                    node.setCreationDateTime(QDateTime::fromMSecsSinceEpoch(outQuery.value(2).toLongLong()));
                    node.setLastModificationDateTime(QDateTime::fromMSecsSinceEpoch(outQuery.value(3).toLongLong()));
                    node.setDeletionDateTime(QDateTime::fromMSecsSinceEpoch(outQuery.value(4).toLongLong()));
                    node.setContent(outQuery.value(5).toString());
                    node.setNodeType(static_cast<NodeData::Type>(outQuery.value(6).toInt()));
                    node.setParentId(outQuery.value(7).toInt());
                    node.setRelativePosition(outQuery.value(8).toInt());
                    node.setAbsolutePath(outQuery.value(9).toString());
                    nodeList[node.id()] = node;
                }
                auto matchFolderFunc = [&](int id, const auto &matchFolderFunctor) {
                    if (!nodeList.contains(id)) {
                        qDebug() << __FUNCTION__ << __LINE__ << "node not found";
                        return;
                    }
                    if (folderIdMap.contains(id)) {
                        return;
                    }
                    auto node = nodeList[id];
                    if (folderIdMap.contains(node.parentId())) {
                        QSqlQuery query(m_db);
                        if (!query.prepare(R"(SELECT "id" FROM node_table WHERE title = :title AND node_type = :node_type AND parent_id = :parent_id;)")) {
                            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
                        }
                        query.bindValue(QStringLiteral(":title"), node.fullTitle());
                        query.bindValue(QStringLiteral(":node_type"), static_cast<int>(NodeData::Type::Folder));
                        query.bindValue(QStringLiteral(":parent_id"), folderIdMap[node.parentId()]);
                        QVector<int> ids;
                        if (query.exec()) {
                            while (query.next()) {
                                ids.append(query.value(0).toInt());
                            }
                        } else {
                            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
                        }
                        query.finish();
                        if (!ids.isEmpty()) {
                            folderIdMap[id] = ids[0];
                        } else {
                            node.setParentId(folderIdMap[node.parentId()]);
                            int newId = addNode(node);
                            folderIdMap[id] = newId;
                        }
                    } else {
                        auto prL = NodePath(node.absolutePath()).separate();
                        for (const auto &pr : std::as_const(prL)) {
                            matchFolderFunctor(pr.toInt(), matchFolderFunctor);
                        }
                    }
                };

                Folder *rootFolder = nullptr;
                QHash<int, Folder> needImportFolderMap;
                for (const auto &node : std::as_const(nodeList)) {
                    Folder f;
                    f.id = node.id();
                    f.parentId = node.parentId();
                    needImportFolderMap[node.id()] = f;
                    if (f.id == ROOT_FOLDER_ID) {
                        rootFolder = &needImportFolderMap[node.id()];
                    }
                }
                if (rootFolder != nullptr) {
                    for (const auto &folder : std::as_const(needImportFolderMap)) {
                        if (folder.id != ROOT_FOLDER_ID) {
                            needImportFolderMap[folder.parentId].children.push_back(&needImportFolderMap[folder.id]);
                        }
                    }
                    auto sortChildFunc = [&](Folder *f) {
                        std::sort(f->children.begin(), f->children.end(), [&](Folder const *f1, Folder const *f2) {
                            return nodeList[f1->id].relativePosition() < nodeList[f2->id].relativePosition();
                        });
                    };
                    auto sortFunc = [&](Folder *f, const auto &sf) -> void {
                        sortChildFunc(f);
                        for (auto *c : f->children) {
                            sf(c, sf);
                        }
                    };
                    sortFunc(rootFolder, sortFunc);
                    auto matchFunc = [&](Folder *f, const auto &mf) -> void {
                        matchFolderFunc(f->id, matchFolderFunc);
                        for (auto *c : f->children) {
                            mf(c, mf);
                        }
                    };
                    matchFunc(rootFolder, matchFunc);
                } else {
                    qDebug() << __FUNCTION__ << "Error while keeping folder position";
                    for (const auto &node : std::as_const(nodeList)) {
                        matchFolderFunc(node.id(), matchFolderFunc);
                    }
                }
            } else {
                qDebug() << __FUNCTION__ << __LINE__ << outQuery.lastError();
            }
        }
        {
            QSqlQuery outQuery(outsideDatabase);
            if (!outQuery.prepare(R"(SELECT)"
                                  R"("id",)"
                                  R"("title",)"
                                  R"("creation_date",)"
                                  R"("modification_date",)"
                                  R"("deletion_date",)"
                                  R"("content",)"
                                  R"("node_type",)"
                                  R"("parent_id",)"
                                  R"("relative_position",)"
                                  R"("scrollbar_position",)"
                                  R"("absolute_path", )"
                                  R"("is_pinned_note", )"
                                  R"("relative_position_an" )"
                                  R"(FROM node_table WHERE node_type=:node_type;)")) {
                qDebug() << __FUNCTION__ << __LINE__ << outQuery.lastError();
            }

            outQuery.bindValue(":node_type", static_cast<int>(NodeData::Type::Note));
            bool status = outQuery.exec();

            QMap<int, std::pair<NodeData, int>> parents;
            for (const auto &id : std::as_const(folderIdMap)) {
                if (parents.contains(id)) {
                    continue;
                }
                auto node = getNode(id);
                auto rel = nextAvailablePosition(id, NodeData::Type::Note);
                parents[id] = std::make_pair(node, rel);
            }
            if (status) {
                QVector<NodeData> nodeList;
                while (outQuery.next()) {
                    NodeData node;
                    node.setId(outQuery.value(0).toInt());
                    node.setFullTitle(outQuery.value(1).toString());
                    node.setCreationDateTime(QDateTime::fromMSecsSinceEpoch(outQuery.value(2).toLongLong()));
                    node.setLastModificationDateTime(QDateTime::fromMSecsSinceEpoch(outQuery.value(3).toLongLong()));
                    node.setDeletionDateTime(QDateTime::fromMSecsSinceEpoch(outQuery.value(4).toLongLong()));
                    node.setContent(outQuery.value(5).toString());
                    node.setNodeType(static_cast<NodeData::Type>(outQuery.value(6).toInt()));
                    node.setParentId(outQuery.value(7).toInt());
                    node.setRelativePosition(outQuery.value(8).toInt());
                    node.setScrollBarPosition(outQuery.value(9).toInt());
                    node.setAbsolutePath(outQuery.value(10).toString());
                    node.setIsPinnedNote(static_cast<bool>(outQuery.value(11).toInt()));
                    node.setRelativePosAN(outQuery.value(13).toInt());
                    node.setChildNotesCount(outQuery.value(14).toInt());
                    node.setTagIds(getAllTagForNote(node.id()));
                    nodeList.append(node);
                }
                if (!m_db.transaction()) {
                    qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
                }
                auto nodeId = nextAvailableNodeId();
                for (auto node : std::as_const(nodeList)) {
                    if (folderIdMap.contains(node.parentId()) && parents.contains(folderIdMap[node.parentId()])) {
                        auto parentId = folderIdMap[node.parentId()];
                        auto parent = parents[parentId].first;
                        auto oldId = node.id();
                        node.setId(nodeId);
                        node.setRelativePosition(parents[parentId].second);
                        node.setAbsolutePath(parent.absolutePath() + PATH_SEPARATOR + QString::number(nodeId));
                        node.setParentId(parentId);
                        addNodePreComputed(node);
                        ++nodeId;
                        parents[parentId].second = node.relativePosition() + 1;
                        noteIdMap[oldId] = node.id();
                    } else {
                        qDebug() << __FUNCTION__ << __LINE__ << "can't find parent for note";
                    }
                }
                QSqlQuery query(m_db);
                if (!query.prepare(R"(UPDATE "metadata" SET "value"=:value WHERE "key"='next_node_id';)")) {
                    qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
                }
                query.bindValue(":value", nodeId + 1);
                if (!query.exec()) {
                    qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
                }
                if (!m_db.commit()) {
                    qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
                }
            } else {
                qDebug() << __FUNCTION__ << __LINE__ << outQuery.lastError();
            }
        }
        {
            QSqlQuery outQuery(outsideDatabase);
            if (!outQuery.prepare(R"(SELECT "tag_id", "node_id" FROM tag_relationship)")) {
                qDebug() << __FUNCTION__ << __LINE__ << outQuery.lastError();
            }
            bool status = outQuery.exec();
            if (status) {
                QVector<std::pair<int, int>> tagRela;
                while (outQuery.next()) {
                    tagRela.append(std::make_pair(outQuery.value(0).toInt(), outQuery.value(1).toInt()));
                }
                if (!m_db.transaction()) {
                    qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
                }
                for (const auto &rel : std::as_const(tagRela)) {
                    if (tagIdMap.contains(rel.first) && noteIdMap.contains(rel.second)) {
                        addNoteToTag(noteIdMap[rel.second], tagIdMap[rel.first]);
                    } else {
                        qDebug() << __FUNCTION__ << __LINE__ << "tag relationship is not valid";
                    }
                }
                if (!m_db.commit()) {
                    qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
                }
            } else {
                qDebug() << __FUNCTION__ << __LINE__ << outQuery.lastError();
            }
        }
        {
            outsideDatabase.close();
            outsideDatabase = QSqlDatabase::database();
        }
    } else {
        auto noteList = readOldNBK(fileName);
        if (noteList.isEmpty()) {
            emit showErrorMessage(tr("Invalid file"), "Please select a valid notes export file");
        } else {
            auto defaultNoteFolder = getNode(DEFAULT_NOTES_FOLDER_ID);
            int nodeId = nextAvailableNodeId();
            int notePos = nextAvailablePosition(defaultNoteFolder.id(), NodeData::Type::Note);
            const QString &parentAbsPath = defaultNoteFolder.absolutePath();
            if (!m_db.transaction()) {
                qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
            }
            for (auto &note : noteList) {
                note.setId(nodeId);
                note.setRelativePosition(notePos);
                note.setAbsolutePath(parentAbsPath + PATH_SEPARATOR + QString::number(nodeId));
                note.setNodeType(NodeData::Type::Note);
                note.setParentId(DEFAULT_NOTES_FOLDER_ID);
                note.setParentName("Notes");
                note.setIsTempNote(false);
                addNodePreComputed(note);
                ++nodeId;
                ++notePos;
            }
            QSqlQuery query(m_db);
            if (!query.prepare(R"(UPDATE "metadata" SET "value"=:value WHERE "key"='next_node_id';)")) {
                qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
            }
            query.bindValue(":value", nodeId + 1);
            if (!query.exec()) {
                qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
            }
            if (!m_db.commit()) {
                qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
            }
        }
    }
    recalculateChildNotesCount();
    onNodeTagTreeRequested();
}

/*!
 * \brief DBManager::onRestoreNotesRequested
 * \param noteList
 */
void DBManager::onRestoreNotesRequested(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << __FUNCTION__ << __LINE__ << "fail to open file";
        return;
    }
    auto const magicHeader = file.read(16);
    file.close();
    if (QString::fromUtf8(magicHeader).startsWith(QStringLiteral("SQLite format 3"))) {
        {
            m_db.close();
            m_db = QSqlDatabase::database();
        }
        QSqlDatabase::removeDatabase(DEFAULT_DATABASE_NAME);
        QFile::remove(m_dbpath);
        if (!QFile::copy(fileName, m_dbpath)) {
            qDebug() << __FUNCTION__ << "Can't import notes";
        };
        open(m_dbpath, false);
    } else {
        auto noteList = readOldNBK(fileName);
        if (noteList.isEmpty()) {
            emit showErrorMessage(tr("Invalid file"), "Please select a valid notes export file");
        } else {
            {
                m_db.close();
                m_db = QSqlDatabase::database();
            }
            QSqlDatabase::removeDatabase(DEFAULT_DATABASE_NAME);
            QFile::remove(m_dbpath);
            open(m_dbpath, true);
            auto defaultNoteFolder = getNode(DEFAULT_NOTES_FOLDER_ID);
            int nodeId = nextAvailableNodeId();
            int notePos = nextAvailablePosition(defaultNoteFolder.id(), NodeData::Type::Note);
            const QString &parentAbsPath = defaultNoteFolder.absolutePath();
            if (!m_db.transaction()) {
                qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
            }
            for (auto &note : noteList) {
                note.setId(nodeId);
                note.setRelativePosition(notePos);
                note.setAbsolutePath(parentAbsPath + PATH_SEPARATOR + QString::number(nodeId));
                note.setNodeType(NodeData::Type::Note);
                note.setParentId(defaultNoteFolder.id());
                note.setParentName("Notes");
                note.setIsTempNote(false);
                addNodePreComputed(note);
                ++nodeId;
                ++notePos;
            }
            QSqlQuery query(m_db);
            if (!query.prepare(R"(UPDATE "metadata" SET "value"=:value WHERE "key"='next_node_id';)")) {
                qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
            }
            query.bindValue(":value", nodeId + 1);
            if (!query.exec()) {
                qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
            }
            if (!m_db.commit()) {
                qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
            }
        }
    }
    recalculateChildNotesCount();
    onNodeTagTreeRequested();
}

/*!
 * \brief DBManager::onExportNotesRequested
 * \param fileName
 */
void DBManager::onExportNotesRequested(const QString &fileName)
{
    QSqlQuery query(m_db);
    if (!query.prepare("BEGIN IMMEDIATE;")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    bool status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    QFile::remove(fileName);
    if (!QFile::copy(m_dbpath, fileName)) {
        qDebug() << __FUNCTION__ << "Can't export notes";
    };

    if (!query.prepare("ROLLBACK;")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    status = query.exec();
    if (!status) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
}

/*!
 * \brief DBManager::onMigrateNotesRequested
 * \param noteList
 */
void DBManager::onMigrateNotesFromV0_9_0Requested(QVector<NodeData> &noteList)
{
    auto defaultNoteFolder = getNode(DEFAULT_NOTES_FOLDER_ID);
    int nodeId = nextAvailableNodeId();
    int notePos = nextAvailablePosition(defaultNoteFolder.id(), NodeData::Type::Note);
    const QString &parentAbsPath = defaultNoteFolder.absolutePath();

    if (!m_db.transaction()) {
        qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
    }
    for (auto &note : noteList) {
        note.setId(nodeId);
        note.setRelativePosition(notePos);
        note.setAbsolutePath(parentAbsPath + PATH_SEPARATOR + QString::number(nodeId));
        note.setNodeType(NodeData::Type::Note);
        note.setParentId(defaultNoteFolder.id());
        note.setParentName("Notes");
        note.setIsTempNote(false);
        addNodePreComputed(note);
        ++nodeId;
        ++notePos;
    }
    QSqlQuery query(m_db);
    if (!query.prepare(R"(UPDATE "metadata" SET "value"=:value WHERE "key"='next_node_id';)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":value", nodeId + 1);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    if (!m_db.commit()) {
        qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
    }
    recalculateChildNotesCount();
}

/*!
 * \brief DBManager::onMigrateTrashRequested
 * \param noteList
 */
void DBManager::onMigrateTrashFrom0_9_0Requested(QVector<NodeData> &noteList)
{
    auto trashFolder = getNode(TRASH_FOLDER_ID);
    int nodeId = nextAvailableNodeId();
    int notePos = nextAvailablePosition(trashFolder.id(), NodeData::Type::Note);
    const QString &parentAbsPath = trashFolder.absolutePath();

    if (!m_db.transaction()) {
        qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
    }
    for (auto &note : noteList) {
        note.setId(nodeId);
        note.setRelativePosition(notePos);
        note.setAbsolutePath(parentAbsPath + PATH_SEPARATOR + QString::number(nodeId));
        note.setNodeType(NodeData::Type::Note);
        note.setParentId(trashFolder.id());
        note.setParentName("Trash");
        note.setIsTempNote(false);
        addNodePreComputed(note);
        ++nodeId;
        ++notePos;
    }
    QSqlQuery query(m_db);
    if (!query.prepare(R"(UPDATE "metadata" SET "value"=:value WHERE "key"='next_node_id';)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":value", nodeId + 1);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    if (!m_db.commit()) {
        qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
    }
    recalculateChildNotesCount();
}

void DBManager::onMigrateNotesFrom1_5_0Requested(const QString &fileName)
{
    auto oldDb = QSqlDatabase::addDatabase("QSQLITE", OUTSIDE_DATABASE_NAME);
    oldDb.setDatabaseName(fileName);
    if (!oldDb.open()) {
        qDebug() << __FUNCTION__ << "Error: connection with database fail";
        return;
    }
    qDebug() << __FUNCTION__ << "Database: connection ok";

    QVector<NodeData> notes;
    QVector<NodeData> trash;
    {
        QSqlQuery query(oldDb);
        if (!query.prepare(R"(SELECT "id", "creation_date", "modification_date", "content", "full_title" FROM "active_notes")")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        bool status = query.exec();
        if (status) {
            while (query.next()) {
                NodeData node;
                int id = query.value(0).toInt();
                qint64 epochDateTimeCreation = query.value(1).toLongLong();
                QDateTime dateTimeCreation = QDateTime::fromMSecsSinceEpoch(epochDateTimeCreation);
                qint64 epochDateTimeModification = query.value(2).toLongLong();
                QDateTime dateTimeModification = QDateTime::fromMSecsSinceEpoch(epochDateTimeModification);
                QString content = query.value(3).toString();
                QString fullTitle = query.value(4).toString();

                node.setId(id);
                node.setCreationDateTime(dateTimeCreation);
                node.setLastModificationDateTime(dateTimeModification);
                node.setContent(content);
                node.setFullTitle(fullTitle);
                notes.append(node);
            }
        } else {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        query.clear();
        if (!query.prepare(R"(SELECT "id", "creation_date", "modification_date", "deletion_date", "content", "full_title" FROM "deleted_notes")")) {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
        status = query.exec();
        if (status) {
            while (query.next()) {
                NodeData node;
                int id = query.value(0).toInt();
                qint64 epochDateTimeCreation = query.value(1).toLongLong();
                QDateTime dateTimeCreation = QDateTime::fromMSecsSinceEpoch(epochDateTimeCreation);
                qint64 epochDateTimeModification = query.value(2).toLongLong();
                QDateTime dateTimeModification = QDateTime::fromMSecsSinceEpoch(epochDateTimeModification);
                qint64 epochDateTimeDeletion = query.value(3).toLongLong();
                QDateTime dateTimeDeletion = QDateTime::fromMSecsSinceEpoch(epochDateTimeDeletion);
                QString content = query.value(4).toString();
                QString fullTitle = query.value(5).toString();

                node.setId(id);
                node.setCreationDateTime(dateTimeCreation);
                node.setLastModificationDateTime(dateTimeModification);
                node.setDeletionDateTime(dateTimeDeletion);
                node.setContent(content);
                node.setFullTitle(fullTitle);
                trash.append(node);
            }
        } else {
            qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
        }
    }
    auto defaultNoteFolder = getNode(DEFAULT_NOTES_FOLDER_ID);
    auto trashFolder = getNode(TRASH_FOLDER_ID);
    int nodeId = nextAvailableNodeId();
    int notePos = nextAvailablePosition(defaultNoteFolder.id(), NodeData::Type::Note);
    QString parentAbsPath = defaultNoteFolder.absolutePath();
    if (!m_db.transaction()) {
        qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
    }
    for (auto &note : notes) {
        note.setId(nodeId);
        note.setRelativePosition(notePos);
        note.setAbsolutePath(parentAbsPath + PATH_SEPARATOR + QString::number(nodeId));
        note.setNodeType(NodeData::Type::Note);
        note.setParentId(defaultNoteFolder.id());
        note.setParentName("Notes");
        note.setIsTempNote(false);
        addNodePreComputed(note);
        ++nodeId;
        ++notePos;
    }
    notePos = nextAvailablePosition(trashFolder.id(), NodeData::Type::Note);
    parentAbsPath = trashFolder.absolutePath();
    for (auto &note : trash) {
        note.setId(nodeId);
        note.setRelativePosition(notePos);
        note.setAbsolutePath(parentAbsPath + PATH_SEPARATOR + QString::number(nodeId));
        note.setNodeType(NodeData::Type::Note);
        note.setParentId(trashFolder.id());
        note.setParentName("Trash");
        note.setIsTempNote(false);
        addNodePreComputed(note);
        ++nodeId;
        ++notePos;
    }
    QSqlQuery query(m_db);
    if (!query.prepare(R"(UPDATE "metadata" SET "value"=:value WHERE "key"='next_node_id';)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":value", nodeId + 1);
    if (!query.exec()) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    if (!m_db.commit()) {
        qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
    }
    {
        oldDb.close();
        oldDb = QSqlDatabase::database();
    }
    QSqlDatabase::removeDatabase(OUTSIDE_DATABASE_NAME);
    recalculateChildNotesCount();
}

void DBManager::onChangeDatabasePathRequested(const QString &newPath)
{
    {
        if (!m_db.commit()) {
            qDebug() << __FUNCTION__ << __LINE__ << m_db.lastError();
        }
        m_db.close();
        m_db = QSqlDatabase::database();
    }
    QFile::rename(m_dbpath, newPath);
    open(newPath, false);
}

void DBManager::addNotesToNewImportedFolder(const QList<QPair<QString, QDateTime>> &fileDatas)
{
    NodeData newFolder;
    newFolder.setNodeType(NodeData::Type::Folder);
    QDateTime currentDate = QDateTime::currentDateTime();
    newFolder.setCreationDateTime(currentDate);
    newFolder.setLastModificationDateTime(currentDate);
    newFolder.setFullTitle("Imported Notes");
    newFolder.setParentId(ROOT_FOLDER_ID);

    int newFolderId = addNode(newFolder);
    if (newFolderId <= 0) {
        qDebug() << "Failed to add 'Imported Notes' folder.";
        return;
    }

    NodeData createdFolder = getNode(newFolderId);
    const QString &parentAbsPath = createdFolder.absolutePath();

    int nodeId = nextAvailableNodeId();
    int notePos = 0;

    for (const auto &noteData : fileDatas) {
        NodeData note;
        note.setFullTitle(noteData.first.section('\n', 0, 0, QString::SectionSkipEmpty));
        note.setContent(noteData.first);
        note.setId(nodeId++);
        note.setRelativePosition(notePos++);
        note.setAbsolutePath(parentAbsPath + PATH_SEPARATOR + QString::number(note.id()));
        note.setNodeType(NodeData::Type::Note);
        note.setParentId(newFolderId);
        note.setCreationDateTime(noteData.second);
        note.setLastModificationDateTime(noteData.second);

        addNode(note);
    }

    recalculateChildNotesCount();
    onNodeTagTreeRequested();
}

void DBManager::exportNotes(const QString &baseExportPath, const QString &extension)
{
    // Ensure the export directory exists
    QString rootFolderName = QStringLiteral("Notes");
    QString exportPathNew = QStringLiteral("%1%2%3").arg(baseExportPath, QDir::separator(), rootFolderName);
    QDir directory;

    int counter = 1;
    while (directory.exists(exportPathNew)) {
        exportPathNew = QStringLiteral("%1%2%3 %4").arg(baseExportPath, QDir::separator(), rootFolderName).arg(counter++);
    }
    qDebug() << "Exporting notes to:" << exportPathNew;
    directory.mkpath(exportPathNew);

    // Retrieve all folders first to create the directory structure
    QVector<NodeData> folders = getAllFolders();

    // Create directories for each folder
    QMap<int, QString> folderPaths;
    for (const NodeData &folder : folders) {
        QString path = exportPathNew;
        if (folder.id() != ROOT_FOLDER_ID) { // Skip root folder
            QStringList folderNames;
            QString currentPath = folder.absolutePath();
            QStringList pathParts = currentPath.split(QDir::separator(), Qt::SkipEmptyParts);
            for (const auto &part : pathParts) {
                int id = part.toInt();
                if (id == ROOT_FOLDER_ID)
                    continue;
                auto folderTemp = getNode(id);
                folderNames << folderTemp.fullTitle();
            }
            QString relativePath = folderNames.join(QDir::separator());
            path += QDir::separator() + relativePath;

            counter = 1;
            while (directory.exists(path)) {
                path = QStringLiteral("%1%2%3 %4").arg(exportPathNew, QDir::separator(), relativePath).arg(counter++);
            }

            QDir().mkpath(path);
        }
        folderPaths.insert(folder.id(), path);
    }

    // Retrieve all notes
    QSqlQuery query(m_db);
    if (!query.prepare(R"(SELECT "id", "title", "content", "parent_id" FROM node_table WHERE node_type = :note_type)")) {
        qDebug() << __FUNCTION__ << __LINE__ << query.lastError();
    }
    query.bindValue(":note_type", static_cast<int>(NodeData::Type::Note));

    if (!query.exec()) {
        qDebug() << "Failed to retrieve notes for export:" << query.lastError();
        return;
    }

    // Export each note as a .txt file in its corresponding directory
    QTextDocument doc;
    while (query.next()) {
        // int noteId = query.value(0).toInt();
        QString title = query.value(1).toString();
        QString content = query.value(2).toString();
        int parentId = query.value(3).toInt();

        QString notePath = folderPaths[parentId];
        QString safeTitle = title;
        if (safeTitle.contains("<br />"))
            safeTitle = safeTitle.section("<br />", 0, 0, QString::SectionSkipEmpty);
        safeTitle = safeTitle.simplified();
        // Convert Markdown to plain text
        doc.setMarkdown(safeTitle);
        safeTitle = doc.toPlainText();
        safeTitle.replace(QRegularExpression(R"([\/\\:*?"<>|])"),
                          "_"); // Make the title filesystem-safe
        QString filePath = QStringLiteral("%1%2%3%4").arg(notePath, QDir::separator(), safeTitle, extension);

        if (safeTitle.isEmpty()) {
            safeTitle = QStringLiteral("Untitled Note");
        }

        counter = 1;
        while (directory.exists(filePath)) {
            filePath = QStringLiteral("%1%2%3 %4%5").arg(notePath, QDir::separator(), safeTitle, QString::number(counter++), extension);
        }

        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << content;
            file.close();
        } else {
            qDebug() << "Failed to export note:" << filePath;
        }
    }

    qDebug() << "Export completed to:" << baseExportPath;
}

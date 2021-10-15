#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "nodedata.h"
#include "tagdata.h"
#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QPair>

struct NodeTagTreeData {
    QVector<NodeData> nodeTreeData;
    QVector<TagData> tagTreeData;
};

class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(QObject *parent = Q_NULLPTR);

private:
    void open(const QString& path, bool doCreate = false);
    void createTables();

    bool isNodeExist(const NodeData& node);

    QVector<NodeData> getAllFolders();
    QVector<TagData> getAllTagInfo();

    bool removeNote(const NodeData& note);
    bool permanantlyRemoveAllNotes();
    bool updateNoteContent(const NodeData &note);
    bool migrateNote(NodeData* note);
    bool migrateTrash(NodeData* note);

    QString getNodeAbsolutePath(int nodeId);
signals:
    void notesListReceived(QVector<NodeData> noteList);
    void nodesTagTreeReceived(const NodeTagTreeData& treeData);

public slots:
    void onNodeTagTreeRequested();
    void onNotesListRequested(int parentID, bool isRecursive);
    void onOpenDBManagerRequested(QString path, bool doCreate);
    void onCreateUpdateRequestedNoteContent(const NodeData& note);
    void onDeleteNoteRequested(const NodeData &note);
    void onImportNotesRequested(QList<NodeData *> noteList);
    void onRestoreNotesRequested(QList<NodeData *> noteList);
    void onExportNotesRequested(QString fileName);
    void onMigrateNotesRequested(QList<NodeData *> noteList);
    void onMigrateTrashRequested(QList<NodeData *> noteList);

    int addNode(const NodeData& node);
    int addTag(const TagData& tag);
    int nextAvailableNodeId();
    int nextAvailableTagId();
    void renameNode(int id, const QString& newName);
private:
    QString m_pathSeperator;
};

#endif // DBMANAGER_H

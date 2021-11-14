#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "nodedata.h"
#include "tagdata.h"
#include "nodepath.h"
#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QPair>
#include <QSet>

struct NodeTagTreeData {
    QVector<NodeData> nodeTreeData;
    QVector<TagData> tagTreeData;
};

struct ListViewInfo {
    bool isInSearch;
    bool isInTag;
    QVector<int> currentTagList;
    int parentFolderId;
    int currentNoteId;
};

class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(QObject *parent = Q_NULLPTR);
    Q_INVOKABLE NodePath getNodeAbsolutePath(int nodeId);
    Q_INVOKABLE NodeData getNode(int nodeId);
    Q_INVOKABLE void moveFolderToTrash(const NodeData& node);
private:
    void open(const QString& path, bool doCreate = false);
    void createTables();

    bool isNodeExist(const NodeData& node);

    QVector<NodeData> getAllFolders();
    QVector<TagData> getAllTagInfo();
    QSet<int> getAllTagForNote(int noteId);
    bool permanantlyRemoveAllNotes();
    bool updateNoteContent(const NodeData &note);
    bool migrateNote(NodeData* note);
    bool migrateTrash(NodeData* note);

signals:
    void notesListReceived(const QVector<NodeData>& noteList, const ListViewInfo& inf);
    void nodesTagTreeReceived(const NodeTagTreeData& treeData);

    void tagAdded(const TagData& tag);
    void tagRemoved(int tagId);
    void tagRenamed(int tagId, const QString& newName);
    void tagColorChanged(int tagId, const QString& tagColor);

public slots:
    void onNodeTagTreeRequested();
    void onNotesListInFolderRequested(int parentID, bool isRecursive);
    void onNotesListInTagsRequested(const QVector<int>& tagIds);
    void onOpenDBManagerRequested(QString path, bool doCreate);
    void onCreateUpdateRequestedNoteContent(const NodeData& note);
    void onImportNotesRequested(QList<NodeData *> noteList);
    void onRestoreNotesRequested(QList<NodeData *> noteList);
    void onExportNotesRequested(QString fileName);
    void onMigrateNotesRequested(QList<NodeData *> noteList);
    void onMigrateTrashRequested(QList<NodeData *> noteList);

    int addNode(const NodeData& node);
    int addTag(const TagData& tag);
    void addNoteToTag(int noteId, int tagId);
    void removeNoteFromTag(int noteId, int tagId);
    int nextAvailableNodeId();
    int nextAvailableTagId();
    void renameNode(int id, const QString& newName);
    void renameTag(int id, const QString& newName);
    void changeTagColor(int id, const QString& newColor);
    void removeNote(const NodeData& note);
    void removeTag(int tagId);
    void moveNode(int nodeId, const NodeData& target);
    void searchForNotes(const QString& keyword, const ListViewInfo& inf);
    void clearSearch(const ListViewInfo& inf);
};

#endif // DBMANAGER_H

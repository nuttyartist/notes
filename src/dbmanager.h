#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "nodedata.h"
#include "tagdata.h"
#include "nodepath.h"
#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QPair>
#include <QSet>
#include <QVector>

struct NodeTagTreeData {
    QVector<NodeData> nodeTreeData;
    QVector<TagData> tagTreeData;
};

struct ListViewInfo {
    bool isInSearch;
    bool isInTag;
    QSet<int> currentTagList;
    int parentFolderId;
    int currentNoteId;
    bool needCreateNewNote;
    int scrollToId;
};

using FolderListType = QMap<int, QString>;

class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(QObject *parent = Q_NULLPTR);
    Q_INVOKABLE NodePath getNodeAbsolutePath(int nodeId);
    Q_INVOKABLE NodeData getNode(int nodeId);
    Q_INVOKABLE void moveFolderToTrash(const NodeData& node);
    Q_INVOKABLE FolderListType getFolderList();

private:
    void open(const QString& path, bool doCreate = false);
    void createTables();

    bool isNodeExist(const NodeData& node);
    QString m_dbpath;
    QSqlDatabase m_db;

    QVector<NodeData> getAllFolders();
    QVector<TagData> getAllTagInfo();
    QSet<int> getAllTagForNote(int noteId);
    bool updateNoteContent(const NodeData &note);
    QList<NodeData> readOldNBK(const QString fileName);
    int nextAvailablePosition(int parentId, NodeData::Type nodeType);
    int addNodePreComputed(const NodeData& node);

signals:
    void notesListReceived(const QVector<NodeData>& noteList, const ListViewInfo& inf);
    void nodesTagTreeReceived(const NodeTagTreeData& treeData);

    void tagAdded(const TagData& tag);
    void tagRemoved(int tagId);
    void tagRenamed(int tagId, const QString& newName);
    void tagColorChanged(int tagId, const QString& tagColor);
    void showErrorMessage(const QString& title, const QString& content);

public slots:
    void onNodeTagTreeRequested();
    void onNotesListInFolderRequested(int parentID, bool isRecursive,
                                      bool newNote = false, int scrollToId = SpecialNodeID::InvalidNodeId);
    void onNotesListInTagsRequested(const QSet<int>& tagIds,
                                    bool newNote = false, int scrollToId = SpecialNodeID::InvalidNodeId);
    void onOpenDBManagerRequested(QString path, bool doCreate);
    void onCreateUpdateRequestedNoteContent(const NodeData& note);
    void onImportNotesRequested(const QString& fileName);
    void onRestoreNotesRequested(const QString& fileName);
    void onExportNotesRequested(const QString& fileName);
    void onMigrateNotesFromV0_9_0Requested(QVector<NodeData> &noteList);
    void onMigrateTrashFrom0_9_0Requested(QVector<NodeData>& noteList);
    void onMigrateNotesFrom1_5_0Requested(const QString& fileName);

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
    void updateRelPosNode(int nodeId, int relPos);
    void updateRelPosTag(int tagId, int relPos);
};

#endif // DBMANAGER_H

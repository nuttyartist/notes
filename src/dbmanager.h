#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "nodedata.h"
#include <QObject>
#include <QtSql/QSqlDatabase>

class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(QObject *parent = Q_NULLPTR);

private:
    void open(const QString& path, bool doCreate = false);
    void createTables();
    int  getLastRowID();
    bool forceLastRowIndexValue(const int indexValue);

    NodeData* getNote(QString id);
    bool isNodeExist(const NodeData& node);

    QList<NodeData *> getAllNotes();
    QVector<NodeData> getAllNodes();

    bool removeNote(const NodeData& note);
    bool permanantlyRemoveAllNotes();
    bool updateNoteContent(const NodeData &note);
    bool migrateNote(NodeData* note);
    bool migrateTrash(NodeData* note);

    QString getNodeAbsolutePath(int nodeId);
signals:
    void notesListReceived(QVector<NodeData> noteList);
    void nodesTreeReceived(QVector<NodeData> nodeTree);
public slots:
    void onNodeTreeRequested();
    void onNotesListRequested(int parentID, bool isRecursive);
    void onOpenDBManagerRequested(QString path, bool doCreate);
    void onCreateUpdateRequestedNoteContent(const NodeData& note);
    void onDeleteNoteRequested(const NodeData &note);
    void onImportNotesRequested(QList<NodeData *> noteList);
    void onRestoreNotesRequested(QList<NodeData *> noteList);
    void onExportNotesRequested(QString fileName);
    void onMigrateNotesRequested(QList<NodeData *> noteList);
    void onMigrateTrashRequested(QList<NodeData *> noteList);
    void onForceLastRowIndexValueRequested(int index);
    int addNode(const NodeData& node);
    int nextAvailableNodeId();
private:
    QString m_pathSeperator;
};

#endif // DBMANAGER_H

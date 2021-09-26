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
    bool isNoteExist(NodeData* note);

    QList<NodeData *> getAllNotes();
    QVector<NodeData> getAllNodes();

    bool addNote(NodeData* note);
    void addNode(const NodeData& node);

    bool removeNote(NodeData* note);
    bool permanantlyRemoveAllNotes();
    bool updateNote(NodeData* note);
    bool migrateNote(NodeData* note);
    bool migrateTrash(NodeData* note);

signals:
    void notesReceived(QList<NodeData*> noteList, int noteCounter);
    void nodesTreeReceived(QVector<NodeData> nodeTree);
public slots:
    void onNodeTreeRequested();
    void onNotesListRequested();
    void onOpenDBManagerRequested(QString path, bool doCreate);
    void onCreateUpdateRequested(NodeData* note);
    void onDeleteNoteRequested(NodeData* note);
    void onImportNotesRequested(QList<NodeData *> noteList);
    void onRestoreNotesRequested(QList<NodeData *> noteList);
    void onExportNotesRequested(QString fileName);
    void onMigrateNotesRequested(QList<NodeData *> noteList);
    void onMigrateTrashRequested(QList<NodeData *> noteList);
    void onForceLastRowIndexValueRequested(int index);
};

#endif // DBMANAGER_H

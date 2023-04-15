#ifndef NODEPATH_H
#define NODEPATH_H

#include <QString>
#include <QList>

#define PATH_SEPARATOR "/"
#define FOLDER_MIME "application/x-foldernode"
#define TAG_MIME "application/x-tagnode"
#define NOTE_MIME "application/x-notenode"

class NodePath
{
public:
    NodePath(const QString &path);
    QStringList separate() const;

    QString path() const;
    NodePath parentPath() const;
    static QString getAllNoteFolderPath();
    static QString getTrashFolderPath();

private:
    QString m_path;
};

#endif // NODEPATH_H

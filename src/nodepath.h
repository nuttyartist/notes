#ifndef NODEPATH_H
#define NODEPATH_H

#include <QString>
#include <QList>

#define PATH_SEPERATOR "☃"

class NodePath
{
public:
    NodePath(const QString& path);
    QStringList seperate() const;

    QString path() const;
    NodePath parentPath() const;
private:
    QString m_path;
};


#endif // NODEPATH_H

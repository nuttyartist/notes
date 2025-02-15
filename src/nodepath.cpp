#include "nodepath.h"
#include "nodedata.h"

NodePath::NodePath(QString path) : m_path(std::move(path)) { }

QStringList NodePath::separate() const
{
    return m_path.split(PATH_SEPARATOR, Qt::SkipEmptyParts);
}

QString const &NodePath::path() const
{
    return m_path;
}

NodePath NodePath::parentPath() const
{
    auto s = separate();
    s.takeLast();
    return { s.join(PATH_SEPARATOR) };
}

QString NodePath::getAllNoteFolderPath()
{
    return QStringLiteral("%1%2").arg(PATH_SEPARATOR).arg(ROOT_FOLDER_ID);
}

QString NodePath::getTrashFolderPath()
{
    return QStringLiteral("%1%2%1%3").arg(PATH_SEPARATOR).arg(ROOT_FOLDER_ID).arg(TRASH_FOLDER_ID);
}

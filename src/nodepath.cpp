#include "nodepath.h"
#include "nodedata.h"

NodePath::NodePath(const QString &path) : m_path(path) { }

QStringList NodePath::separate() const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    return m_path.split(PATH_SEPARATOR, QString::SkipEmptyParts);
#else
    return m_path.split(PATH_SEPARATOR, Qt::SkipEmptyParts);
#endif
}

QString NodePath::path() const
{
    return m_path;
}

NodePath NodePath::parentPath() const
{
    auto s = separate();
    s.takeLast();
    return s.join(PATH_SEPARATOR);
}

QString NodePath::getAllNoteFolderPath()
{
    return PATH_SEPARATOR + QString::number(SpecialNodeID::RootFolder);
}

QString NodePath::getTrashFolderPath()
{
    return PATH_SEPARATOR + QString::number(SpecialNodeID::RootFolder) + PATH_SEPARATOR
            + QString::number(SpecialNodeID::TrashFolder);
}

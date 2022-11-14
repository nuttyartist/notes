#include "nodepath.h"
#include "nodedata.h"

NodePath::NodePath(const QString &path) : m_path(path) { }

QStringList NodePath::seperate() const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    return m_path.split(PATH_SEPERATOR, QString::SkipEmptyParts);
#else
    return m_path.split(PATH_SEPERATOR, Qt::SkipEmptyParts);
#endif
}

QString NodePath::path() const
{
    return m_path;
}

NodePath NodePath::parentPath() const
{
    auto s = seperate();
    s.takeLast();
    return s.join(PATH_SEPERATOR);
}

QString NodePath::getAllNoteFolderPath()
{
    return PATH_SEPERATOR + QString::number(SpecialNodeID::RootFolder);
}

QString NodePath::getTrashFolderPath()
{
    return PATH_SEPERATOR + QString::number(SpecialNodeID::RootFolder) + PATH_SEPERATOR
            + QString::number(SpecialNodeID::TrashFolder);
}

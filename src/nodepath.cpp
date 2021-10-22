#include "nodepath.h"

NodePath::NodePath(const QString &path) :
    m_path(path)
{

}

QStringList NodePath::seperate() const
{
    return m_path.split(PATH_SEPERATOR, Qt::SkipEmptyParts);
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

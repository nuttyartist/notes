#ifndef NODEPATH_H
#define NODEPATH_H

#include <QString>
#include <QList>

#define PATH_SEPERATOR "/"
#define FOLDER_MIME "application/x-foldernode"
#define TAG_MIME "application/x-tagnode"
#define NOTE_MIME "application/x-notenode"

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
template<typename T>
void vector_move(QVector<T>& vec, int from, int to)
{
    Q_ASSERT_X(from >= 0 && from < vec.size(), "QVector::move(int,int)", "'from' is out-of-range");
    Q_ASSERT_X(to >= 0 && to < vec.size(), "QVector::move(int,int)", "'to' is out-of-range");
    if (from == to) // don't detach when no-op
        return;
    vec.detach();
    T * const b = vec.begin();
    if (from < to)
        std::rotate(b + from, b + from + 1, b + to + 1);
    else
        std::rotate(b + to, b + from, b + from + 1);
}
#else
template<typename T>
void vector_move(QVector<T>& vec, int from, int to)
{
    vec.move(from, to);
}
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
    #define QT_AS_CONST(x) x
#else
    #define QT_AS_CONST(x) qAsConst(x)
#endif

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

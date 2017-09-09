#ifndef NOTEDATA_H
#define NOTEDATA_H

#include <QObject>
#include <QDateTime>

class NoteData : public QObject
{
    Q_OBJECT

    friend class tst_NoteData;

public:
    explicit NoteData(QObject *parent = Q_NULLPTR);

    int id() const;
    void setId(const int& id);

    QString fullTitle() const;
    void setFullTitle(const QString &fullTitle);

    QDateTime lastModificationdateTime() const;
    void setLastModificationDateTime(const QDateTime &lastModificationdateTime);

    QDateTime creationDateTime() const;
    void setCreationDateTime(const QDateTime& creationDateTime);

    QString content() const;
    void setContent(const QString &content);

    bool isModified() const;
    void setModified(bool isModified);

    bool isSelected() const;
    void setSelected(bool isSelected);

    int scrollBarPosition() const;
    void setScrollBarPosition(int scrollBarPosition);

    QDateTime deletionDateTime() const;
    void setDeletionDateTime(const QDateTime& deletionDateTime);


private:
    int m_id;
    QString m_fullTitle;
    QDateTime m_lastModificationDateTime;
    QDateTime m_creationDateTime;
    QDateTime m_deletionDateTime;
    QString m_content;
    bool m_isModified;
    bool m_isSelected;
    int m_scrollBarPosition;
};

QDataStream &operator<<(QDataStream &stream, const NoteData* noteData);
QDataStream &operator>>(QDataStream &stream, NoteData *&noteData);

#endif // NOTEDATA_H

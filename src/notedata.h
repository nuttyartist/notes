#ifndef NOTEDATA_H
#define NOTEDATA_H

#include <QObject>
#include <QDateTime>

class NoteData : public QObject
{
    Q_OBJECT
public:
    explicit NoteData(QObject *parent = Q_NULLPTR);

    QString id() const;
    void setId(const QString &id);

    QString fullTitle() const;
    void setFullTitle(const QString &fullTitle);

    QDateTime dateTime() const;
    void setDateTime(const QDateTime &dateTime);

    QString content() const;
    void setContent(const QString &content);

    bool isModified() const;
    void setModified(bool isModified);

    bool isSelected() const;
    void setSelected(bool isSelected);

    int scrollBarPosition() const;
    void setScrollBarPosition(int scrollBarPosition);

private:
    QString m_id;
    QString m_fullTitle;
    QDateTime m_dateTime;
    QString m_content;
    bool m_isModified;
    bool m_isSelected;
    int m_scrollBarPosition;
};


#endif // NOTEDATA_H

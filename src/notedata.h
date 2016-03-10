#ifndef NOTEDATA_H
#define NOTEDATA_H

#include <QWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QDateTime>

class NoteData : public QWidget
{
    Q_OBJECT

public:
    explicit NoteData(const QString& noteName, QWidget *parent = 0);
    ~NoteData();

    QString m_text;
    QString m_noteName;
    QString m_title;
    QDateTime m_dateTime;
    QGroupBox* m_fakeContainer;
    QGroupBox* m_containerBox;
    QPushButton* m_button;
    QLabel* m_titleLabel;
    QLabel* m_dateLabel;
    QFrame* m_seperateLine;

    int m_scrollBarPosition;

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    void setupWidget();


};

#endif // NOTEDATA_H

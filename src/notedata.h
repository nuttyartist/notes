#ifndef NOTEDATA_H
#define NOTEDATA_H

#include <QObject>
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

    void setTitle(QString &title);

    bool isSelected() const;
    void setSelected(bool isSelected);
    void setSelectedWithFocus(bool isSelected, bool focus);

    bool isModified() const;
    void setModified(bool isModified);

    QString noteName() const;
    void setNoteName(const QString &noteName);

    QString text() const;
    void setText(const QString &text);

    void setDateTime(const QDateTime &dateTime);
    QDateTime dateTime() const;

    int scrollBarPosition() const;
    void setScrollBarPosition(int scrollBarPosition);

protected:
    void resizeEvent(QResizeEvent *) override;
    void focusInEvent(QFocusEvent *) override;
    void focusOutEvent(QFocusEvent *) override;
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;

private:
    void setupWidget();
    void setContainerStyle(QColor color, bool doShowSeparator);
    void elideTitle();
    QString parseDateTime(QDateTime dateTimeEdited);

    bool m_isSelected;
    bool m_isModified;
    int m_scrollBarPosition;

    QString m_noteName;
    QString m_fullTitle;
    QString m_text;
    QDateTime m_dateTime;
    QColor m_focusColor;
    QColor m_unfocusColor;
    QColor m_enterColor;
    QColor m_defaultColor;

    QFrame* m_frameContainer;
    QPushButton* m_button;
    QLabel* m_titleLabel;
    QLabel* m_dateLabel;

public slots:
    void onButtonPressed();

signals:
    void pressed();

};

#endif // NOTEDATA_H

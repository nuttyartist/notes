#ifndef ALLNOTEBUTTONTREEDELEGATEEDITOR_H
#define ALLNOTEBUTTONTREEDELEGATEEDITOR_H

#include <QWidget>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QFont>

class QTreeView;

class AllNoteButtonTreeDelegateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit AllNoteButtonTreeDelegateEditor(QTreeView *view,
                                       const QStyleOptionViewItem &option,
                                      const QModelIndex &index,
                                      QWidget *parent = nullptr);

private:
    QStyleOptionViewItem m_option;
    QModelIndex m_index;
    QString m_displayFont;
    QFont m_titleFont;
    QColor m_titleColor;
    QColor m_titleSelectedColor;
    QColor m_activeColor;
    QColor m_hoverColor;
    QTreeView *m_view;
    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // ALLNOTEBUTTONTREEDELEGATEEDITOR_H

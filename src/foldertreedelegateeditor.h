#ifndef FOLDERTREEDELEGATEEDITOR_H
#define FOLDERTREEDELEGATEEDITOR_H

#include <QWidget>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QFont>

class QTreeView;
class QLabel;
class PushButtonType;
class LabelEditType;

class FolderTreeDelegateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit FolderTreeDelegateEditor(QTreeView* view,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index,
                                      QWidget *parent = nullptr);

signals:
    void contextMenuRequested();

private:
    QStyleOptionViewItem m_option;
    QModelIndex m_index;
    QString m_displayFont;
    QFont m_titleFont;
    QColor m_titleColor;
    QColor m_titleSelectedColor;
    QColor m_activeColor;
    QColor m_hoverColor;
    QTreeView* m_view;
    LabelEditType* m_label;
    QPixmap m_expanded;
    QPixmap m_notExpanded;
    QLabel* m_expandIcon;
    PushButtonType* m_contextButton;
    void updateDelegate();

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif // FOLDERTREEDELEGATEEDITOR_H

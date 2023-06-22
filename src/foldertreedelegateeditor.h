#ifndef FOLDERTREEDELEGATEEDITOR_H
#define FOLDERTREEDELEGATEEDITOR_H

#include <QWidget>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QFont>
#include "editorsettingsoptions.h"

class QTreeView;
class QLabel;
class PushButtonType;
class LabelEditType;
class QListView;

class FolderTreeDelegateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit FolderTreeDelegateEditor(QTreeView *view, const QStyleOptionViewItem &option,
                                      const QModelIndex &index, QListView *listView,
                                      QWidget *parent = nullptr);
    void setTheme(Theme::Value theme);

private:
    QStyleOptionViewItem m_option;
    QModelIndex m_index;
    QString m_displayFont;
    QFont m_titleFont;
    QColor m_titleColor;
    QColor m_titleSelectedColor;
    QColor m_activeColor;
    QColor m_hoverColor;
    QColor m_folderIconColor;
    QTreeView *m_view;
    QListView *m_listView;
    LabelEditType *m_label;
    PushButtonType *m_folderIcon;
    QPixmap m_expanded;
    QPixmap m_notExpanded;
    QLabel *m_expandIcon;
    PushButtonType *m_contextButton;
    Theme::Value m_theme;
    void updateDelegate();

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif // FOLDERTREEDELEGATEEDITOR_H

#ifndef NODETREEDELEGATE_H
#define NODETREEDELEGATE_H

#include <QStyledItemDelegate>
#include "editorsettingsoptions.h"

class QTreeView;
class QListView;

class NodeTreeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit NodeTreeDelegate(QTreeView *view, QObject *parent = nullptr,
                              QListView *listView = nullptr);
    void setTheme(Theme::Value theme);
signals:
    void addFolderRequested();
    void addTagRequested();
    void themeChanged(Theme::Value theme);

    // QAbstractItemDelegate interface
public:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const override;
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const override;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const override;

private:
    void paintBackgroundSelectable(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const;

private:
    QString m_displayFont;
    QFont m_titleFont;
    QFont m_titleSelectedFont;
    QFont m_dateFont;
    QFont m_separatorFont;
    QFont m_numberOfNotesFont;
    QColor m_titleColor;
    QColor m_titleSelectedColor;
    QColor m_dateColor;
    QColor m_ActiveColor;
    QColor m_notActiveColor;
    QColor m_hoverColor;
    QColor m_applicationInactiveColor;
    QColor m_separatorColor;
    QColor m_defaultColor;
    QColor m_separatorTextColor;
    QColor m_currentBackgroundColor;
    QColor m_numberOfNotesColor;
    QColor m_numberOfNotesSelectedColor;
    QColor m_folderIconColor;
    QTreeView *m_view;
    QListView *m_listView;
    Theme::Value m_theme;
};

#endif // NODETREEDELEGATE_H

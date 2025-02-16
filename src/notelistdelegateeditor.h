#ifndef NOTELISTDELEGATEEDITOR_H
#define NOTELISTDELEGATEEDITOR_H

#include <QWidget>
#include "notelistview.h"
#include <QTimeLine>
#include "editorsettingsoptions.h"

class NoteListDelegate;
class TagListModel;
class TagListView;
class TagListDelegate;
class NoteListModel;
namespace note_list_constants {
auto constexpr LEFT_OFFSET_X = 20;
auto constexpr TOP_OFFSET_Y = 10; // space on top of title
auto constexpr TITLE_DATE_SPACE = 2; // space between title and date
auto constexpr DATE_DESC_SPACE = 5; // space between date and description
auto constexpr DESC_FOLDER_SPACE = 14; // space between description and folder name
auto constexpr LAST_EL_SEP_SPACE = 12; // space between the last element and the separator
auto constexpr NEXT_NOTE_OFFSET = 0; // space between the separator and the next note underneath it
auto constexpr PINNED_HEADER_TO_NOTE_SPACE = 0; // space between Pinned label to the pinned list
auto constexpr UNPINNED_HEADER_TO_NOTE_SPACE = 0; // space between Notes label and the normal notes list
auto constexpr LAST_PINNED_TO_UNPINNED_HEADER = 10; // space between the last pinned note to Notes label
} // namespace note_list_constants

class NoteListDelegateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit NoteListDelegateEditor(const NoteListDelegate *delegate, NoteListView *view, const QStyleOptionViewItem &option, const QModelIndex &index,
                                    TagPool *tagPool, QWidget *parent = nullptr);
    ~NoteListDelegateEditor() override;

    void setRowRightOffset(int rowRightOffset);
    void setActive(bool isActive);
    void recalculateSize();
    void setScrollBarPos(int pos);
    int getScrollBarPos();
    bool underMouseC() const;
    QPixmap renderToPixmap();

public slots:
    void setTheme(Theme::Value theme);
signals:
    void updateSizeHint(int id, const QSize &sz, const QModelIndex &index);
    void nearDestroyed(int id, const QModelIndex &index);

private:
    void paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintLabels(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintSeparator(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    const NoteListDelegate *m_delegate;
    QStyleOptionViewItem m_option;
    int m_id;
    NoteListView *m_view;

    TagPool *m_tagPool;
    QString m_displayFont;
    QFont m_titleFont;
    QFont m_titleSelectedFont;
    QFont m_dateFont;
    QFont m_headerFont;
    QColor m_titleColor;
    QColor m_dateColor;
    QColor m_contentColor;
    QColor m_activeColor;
    QColor m_notActiveColor;
    QColor m_hoverColor;
    QColor m_applicationInactiveColor;
    QColor m_separatorColor;
    QColor m_defaultColor;
    int m_rowHeight;
    int m_rowRightOffset;
    bool m_isActive;
    QImage m_folderIcon;
    Theme::Value m_theme;
    bool m_containsMouse;
    QModelIndex m_animatedIndex;

    TagListView *m_tagListView;
    TagListModel *m_tagListModel;
    TagListDelegate *m_tagListDelegate;
    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void enterEvent(QEnterEvent *event) override;

    void leaveEvent(QEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

#endif // NOTELISTDELEGATEEDITOR_H

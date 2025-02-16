#include "notelistdelegateeditor.h"
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include <QtMath>
#include <QPainterPath>
#include <QScrollBar>
#include <QDragEnterEvent>
#include <QMimeData>
#include "notelistmodel.h"
#include "noteeditorlogic.h"
#include "tagpool.h"
#include "nodepath.h"
#include "notelistdelegate.h"
#include "taglistview.h"
#include "taglistmodel.h"
#include "taglistdelegate.h"
#include "fontloader.h"
#include "utils.h"

NoteListDelegateEditor::NoteListDelegateEditor(const NoteListDelegate *delegate, NoteListView *view, const QStyleOptionViewItem &option,
                                               const QModelIndex &index, TagPool *tagPool, QWidget *parent)
    : QWidget(parent),
      m_delegate(delegate),
      m_option(option),
      m_id(index.data(NoteListModel::NoteID).toInt()),
      m_view(view),
      m_tagPool{ tagPool },
#ifdef __APPLE__
      m_displayFont(QFont(QStringLiteral("SF Pro Text")).exactMatch() ? QStringLiteral("SF Pro Text") : QStringLiteral("Roboto")),
#elif _WIN32
      m_displayFont(QFont(QStringLiteral("Segoe UI")).exactMatch() ? QStringLiteral("Segoe UI") : QStringLiteral("Roboto")),
#else
      m_displayFont(QStringLiteral("Roboto")),
#endif

#ifdef __APPLE__
      m_titleFont(m_displayFont, 13, QFont::DemiBold),
      m_titleSelectedFont(m_displayFont, 13, QFont::DemiBold),
      m_dateFont(m_displayFont, 13),
      m_headerFont(m_displayFont, 10, QFont::DemiBold),
#else
      m_titleFont(m_displayFont, 10, QFont::DemiBold),
      m_titleSelectedFont(m_displayFont, 10),
      m_dateFont(m_displayFont, 10),
      m_headerFont(m_displayFont, 10, QFont::DemiBold),
#endif
      m_titleColor(26, 26, 26),
      m_dateColor(26, 26, 26),
      m_contentColor(142, 146, 150),
      m_activeColor(218, 233, 239),
      m_notActiveColor(175, 212, 228),
      m_hoverColor(207, 207, 207),
      m_applicationInactiveColor(207, 207, 207),
      m_separatorColor(191, 191, 191),
      m_defaultColor(247, 247, 247),
      m_rowHeight(106),
      m_rowRightOffset(0),
      m_isActive(false),
      m_theme(Theme::Light),
      m_containsMouse(false)
{
    setContentsMargins(0, 0, 0, 0);
    m_folderIcon = QImage(":/images/folder.png");
    m_tagListView = new TagListView(this);
    m_tagListModel = new TagListModel(this);
    m_tagListDelegate = new TagListDelegate(this);
    m_tagListView->setFrameStyle(QFrame::NoFrame);
    m_tagListView->setModel(m_tagListModel);
    m_tagListView->setItemDelegate(m_tagListDelegate);
    m_tagListModel->setTagPool(tagPool);
    m_tagListModel->setModelData(index.data(NoteListModel::NoteTagsList).value<QSet<int>>());
    if (m_delegate->isInAllNotes()) {
        int y = 90;
        auto const *noteListModel = static_cast<NoteListModel *>(m_view->model());
        if (noteListModel != nullptr) {
            auto idx = noteListModel->getNoteIndex(m_id);
            if (noteListModel->hasPinnedNote() && (noteListModel->isFirstPinnedNote(idx) || noteListModel->isFirstUnpinnedNote(idx))) {
                y += 25;
            }
        }
        int fourthYOffset = 0;
        if ((noteListModel != nullptr) && noteListModel->isFirstUnpinnedNote(index)) {
            fourthYOffset = note_list_constants::UNPINNED_HEADER_TO_NOTE_SPACE;
        }
        int fifthYOffset = 0;
        if ((noteListModel != nullptr) && noteListModel->hasPinnedNote() && !m_view->isPinnedNotesCollapsed() && noteListModel->isFirstUnpinnedNote(index)) {
            fifthYOffset = note_list_constants::LAST_PINNED_TO_UNPINNED_HEADER;
        }
        int yOffsets = fourthYOffset + fifthYOffset;

        y += yOffsets;
        m_tagListView->setGeometry(10, y - 5, rect().width() - 15, m_tagListView->height());
    } else {
        int y = 70;
        auto const *noteListModel = static_cast<NoteListModel *>(m_view->model());
        if (noteListModel != nullptr) {
            auto idx = noteListModel->getNoteIndex(m_id);
            if (noteListModel->hasPinnedNote() && (noteListModel->isFirstPinnedNote(idx) || noteListModel->isFirstUnpinnedNote(idx))) {
                y += 25;
            }
        }
        int fourthYOffset = 0;
        if ((noteListModel != nullptr) && noteListModel->isFirstUnpinnedNote(index)) {
            fourthYOffset = note_list_constants::UNPINNED_HEADER_TO_NOTE_SPACE;
        }
        int fifthYOffset = 0;
        if ((noteListModel != nullptr) && noteListModel->hasPinnedNote() && !m_view->isPinnedNotesCollapsed() && noteListModel->isFirstUnpinnedNote(index)) {
            fifthYOffset = note_list_constants::LAST_PINNED_TO_UNPINNED_HEADER;
        }
        int yOffsets = fourthYOffset + fifthYOffset;

        y += yOffsets;
        m_tagListView->setGeometry(10, y - 5, rect().width() - 15, m_tagListView->height());
    }
    connect(m_tagListView->verticalScrollBar(), &QScrollBar::valueChanged, this, [this] {
        auto idx = static_cast<NoteListModel *>(m_view->model())->getNoteIndex(m_id);
        static_cast<NoteListModel *>(m_view->model())->setData(idx, getScrollBarPos(), NoteListModel::NoteTagListScrollbarPos);
    });
    QTimer::singleShot(0, this, [this] {
        auto idx = static_cast<NoteListModel *>(m_view->model())->getNoteIndex(m_id);
        setScrollBarPos(idx.data(NoteListModel::NoteTagListScrollbarPos).toInt());
    });
    m_view->setEditorWidget(m_id, this);
    setMouseTracking(true);
    setAcceptDrops(true);
}

NoteListDelegateEditor::~NoteListDelegateEditor()
{
    m_view->unsetEditorWidget(m_id, nullptr);
}

void NoteListDelegateEditor::paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto bufferSize = rect().size();
    QPixmap buffer{ bufferSize };
    buffer.fill(Qt::transparent);
    QPainter bufferPainter{ &buffer };
    bufferPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QRect bufferRect = buffer.rect();
    auto const *noteListModel = static_cast<NoteListModel *>(m_view->model());
    if (noteListModel->hasPinnedNote() && (noteListModel->isFirstPinnedNote(index) || noteListModel->isFirstUnpinnedNote(index))) {
        int fifthYOffset = 0;
        if (!m_view->isPinnedNotesCollapsed() && noteListModel->isFirstUnpinnedNote(index)) {
            fifthYOffset = note_list_constants::LAST_PINNED_TO_UNPINNED_HEADER;
        }
        bufferRect.setY(bufferRect.y() + 25 + fifthYOffset);
    }
    auto isPinned = index.data(NoteListModel::NoteIsPinned).toBool();
    if (m_view->selectionModel()->isSelected(index)) {
        if (qApp->applicationState() == Qt::ApplicationActive) {
            if (m_isActive) {
                bufferPainter.fillRect(bufferRect, QBrush(m_activeColor));
                m_tagListView->setBackground(m_activeColor);
            } else {
                bufferPainter.fillRect(bufferRect, QBrush(m_notActiveColor));
                m_tagListView->setBackground(m_notActiveColor);
            }
        } else if (qApp->applicationState() == Qt::ApplicationInactive) {
            bufferPainter.fillRect(bufferRect, QBrush(m_applicationInactiveColor));
            m_tagListView->setBackground(m_applicationInactiveColor);
        }
    } else if (underMouseC()) {
        if (m_view->isDragging()) {
            if (isPinned) {
                auto rect = bufferRect;
                rect.setTop(rect.bottom() - 5);
                bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            }
        } else {
            bufferPainter.fillRect(bufferRect, QBrush(m_hoverColor));
            m_tagListView->setBackground(m_hoverColor);
        }
    } else {
        bufferPainter.fillRect(bufferRect, QBrush(m_defaultColor));
        m_tagListView->setBackground(m_defaultColor);
    }
    if (m_view->isDragging() && !isPinned && !m_view->isDraggingInsidePinned()) {
        if ((noteListModel != nullptr) && noteListModel->isFirstUnpinnedNote(index) && (index.row() == (noteListModel->rowCount() - 1))) {
            auto rect = bufferRect;
            rect.setHeight(4);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setWidth(3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setLeft(rect.right() - 3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setTop(rect.bottom() - 3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
        } else if ((noteListModel != nullptr) && noteListModel->isFirstUnpinnedNote(index)) {
            auto rect = bufferRect;
            rect.setHeight(4);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setWidth(3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setLeft(rect.right() - 3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
        } else if ((noteListModel != nullptr) && (index.row() == (noteListModel->rowCount() - 1))) {
            auto rect = bufferRect;
            rect.setTop(rect.bottom() - 3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setWidth(3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setLeft(rect.right() - 3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
        } else {
            auto rect = bufferRect;
            rect.setWidth(3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setLeft(rect.right() - 3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
        }
    }
    if ((noteListModel != nullptr) && (m_delegate != nullptr) && m_delegate->shouldPaintSeparator(index, *noteListModel)) {
        paintSeparator(&bufferPainter, option, index);
    }

    auto rowHeight = rect().height();
    painter->drawPixmap(rect(), buffer, QRect{ 0, bufferSize.height() - rowHeight, rect().width(), rowHeight });
}

void NoteListDelegateEditor::paintLabels(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);

    QString title{ index.data(NoteListModel::NoteFullTitle).toString() };
    QFont titleFont = m_view->selectionModel()->isSelected(index) ? m_titleSelectedFont : m_titleFont;
    QFontMetrics fmTitle(titleFont);
    QRect fmRectTitle = fmTitle.boundingRect(title);

    QString date = utils::parseDateTime(index.data(NoteListModel::NoteLastModificationDateTime).toDateTime());
    QFontMetrics fmDate(m_dateFont);
    QRect fmRectDate = fmDate.boundingRect(date);

    QString parentName{ index.data(NoteListModel::NoteParentName).toString() };
    QFontMetrics fmParentName(titleFont);
    QRect fmRectParentName = fmParentName.boundingRect(parentName);

    QString content{ index.data(NoteListModel::NoteContent).toString() };
    content = NoteEditorLogic::getSecondLine(content);
    QFontMetrics fmContent(titleFont);
    QRect fmRectContent = fmContent.boundingRect(content);

    double rowPosX = rect().x();
    double rowPosY = rect().y();
    double rowWidth = rect().width();
    auto const *noteListModel = static_cast<NoteListModel *>(m_view->model());
    int fifthYOffset = 0;
    if (noteListModel->hasPinnedNote() && !m_view->isPinnedNotesCollapsed() && noteListModel->isFirstUnpinnedNote(index)) {
        fifthYOffset = note_list_constants::LAST_PINNED_TO_UNPINNED_HEADER;
    }
    if (noteListModel->hasPinnedNote()) {
        if (noteListModel->isFirstPinnedNote(index)) {
            QRect headerRect(rowPosX + (note_list_constants::LEFT_OFFSET_X / 2), rowPosY, rowWidth - (note_list_constants::LEFT_OFFSET_X / 2), 25);
#ifdef __APPLE__
            int iconPointSizeOffset = 0;
#else
            int iconPointSizeOffset = -4;
#endif
            painter->setFont(font_loader::loadFont("Font Awesome 6 Free Solid", "", 14 + iconPointSizeOffset));
            painter->setPen(QColor(68, 138, 201));
            if (m_view->isPinnedNotesCollapsed()) {
                painter->drawText(QRect(headerRect.right() - 25, headerRect.y() + 5, 16, 16),
                                  u8"\uf054"); // fa-chevron-right
            } else {
                painter->drawText(QRect(headerRect.right() - 25, headerRect.y() + 5, 16, 16),
                                  u8"\uf078"); // fa-chevron-down
            }
            painter->setPen(m_contentColor);
            painter->setFont(m_headerFont);
            painter->drawText(headerRect, Qt::AlignLeft | Qt::AlignVCenter, "Pinned");
            rowPosY += 25;
        } else if (noteListModel->isFirstUnpinnedNote(index)) {
            rowPosY += fifthYOffset;
            QRect headerRect(rowPosX + (note_list_constants::LEFT_OFFSET_X / 2), rowPosY, rowWidth - (note_list_constants::LEFT_OFFSET_X / 2), 25);
            painter->setPen(m_contentColor);
            painter->setFont(m_headerFont);
            painter->drawText(headerRect, Qt::AlignLeft | Qt::AlignVCenter, "Notes");
            rowPosY += 25;
        }
    }
    if (m_view->isPinnedNotesCollapsed()) {
        auto isPinned = index.data(NoteListModel::NoteIsPinned).value<bool>();
        if (isPinned) {
            return;
        }
    }
    int secondYOffset = 0;
    if (index.row() > 0) {
        secondYOffset = note_list_constants::NEXT_NOTE_OFFSET;
    }
    int thirdYOffset = 0;
    if ((noteListModel != nullptr) && noteListModel->isFirstPinnedNote(index)) {
        thirdYOffset = note_list_constants::PINNED_HEADER_TO_NOTE_SPACE;
    }
    int fourthYOffset = 0;
    if ((noteListModel != nullptr) && noteListModel->isFirstUnpinnedNote(index)) {
        fourthYOffset = note_list_constants::UNPINNED_HEADER_TO_NOTE_SPACE;
    }

    int yOffsets = secondYOffset + thirdYOffset + fourthYOffset;
    double titleRectPosX = rowPosX + note_list_constants::LEFT_OFFSET_X;
    double titleRectPosY = rowPosY;
    double titleRectWidth = rowWidth - (2.0 * note_list_constants::LEFT_OFFSET_X);
    double titleRectHeight = fmRectTitle.height() + note_list_constants::TOP_OFFSET_Y + yOffsets;

    double dateRectPosX = rowPosX + note_list_constants::LEFT_OFFSET_X;
    double dateRectPosY = rowPosY + fmRectTitle.height() + note_list_constants::TOP_OFFSET_Y + yOffsets;
    double dateRectWidth = rowWidth - (2.0 * note_list_constants::LEFT_OFFSET_X);
    double dateRectHeight = fmRectDate.height() + note_list_constants::TITLE_DATE_SPACE;

    double contentRectPosX = rowPosX + note_list_constants::LEFT_OFFSET_X;
    double contentRectPosY = rowPosY + fmRectTitle.height() + fmRectDate.height() + note_list_constants::TOP_OFFSET_Y + yOffsets;
    double contentRectWidth = rowWidth - (2.0 * note_list_constants::LEFT_OFFSET_X);
    double contentRectHeight = fmRectContent.height() + note_list_constants::DATE_DESC_SPACE;

    double folderNameRectPosX = 0;
    double folderNameRectPosY = 0;
    double folderNameRectWidth = 0;
    double folderNameRectHeight = 0;

    if (m_delegate->isInAllNotes()) {
        folderNameRectPosX = rowPosX + note_list_constants::LEFT_OFFSET_X + 20;
        folderNameRectPosY = rowPosY + fmRectContent.height() + fmRectTitle.height() + fmRectDate.height() + note_list_constants::TOP_OFFSET_Y + yOffsets;
        folderNameRectWidth = rowWidth - 2.0 * note_list_constants::LEFT_OFFSET_X;
        folderNameRectHeight = fmRectParentName.height() + note_list_constants::DESC_FOLDER_SPACE;
    }
    auto drawStr = [painter](double posX, double posY, double width, double height, QColor color, const QFont &font, const QString &str) {
        QRectF rect(posX, posY, width, height);
        painter->setPen(color);
        painter->setFont(font);
        painter->drawText(rect, Qt::AlignBottom, str);
    };

    // draw title & date
    title = fmTitle.elidedText(title, Qt::ElideRight, int(titleRectWidth));
    content = fmContent.elidedText(content, Qt::ElideRight, int(titleRectWidth));
    drawStr(titleRectPosX, titleRectPosY, titleRectWidth, titleRectHeight, m_titleColor, titleFont, title);
    drawStr(dateRectPosX, dateRectPosY, dateRectWidth, dateRectHeight, m_dateColor, m_dateFont, date);
    if (m_delegate->isInAllNotes()) {
        painter->drawImage(QRect(rowPosX + note_list_constants::LEFT_OFFSET_X, folderNameRectPosY + note_list_constants::DESC_FOLDER_SPACE, 16, 16),
                           m_folderIcon);
        drawStr(folderNameRectPosX, folderNameRectPosY, folderNameRectWidth, folderNameRectHeight, m_contentColor, titleFont, parentName);
    }
    drawStr(contentRectPosX, contentRectPosY, contentRectWidth, contentRectHeight, m_contentColor, titleFont, content);
}

void NoteListDelegateEditor::paintSeparator(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    Q_UNUSED(option);

    painter->setPen(QPen(m_separatorColor));
    const int leftOffsetX = note_list_constants::LEFT_OFFSET_X;
    int posX1 = rect().x() + leftOffsetX;
    int posX2 = rect().x() + rect().width() - leftOffsetX - 1;
    int posY = rect().y() + rect().height() - 1;

    painter->drawLine(QPoint(posX1, posY), QPoint(posX2, posY));
}

bool NoteListDelegateEditor::underMouseC() const
{
    return m_containsMouse;
}

QPixmap NoteListDelegateEditor::renderToPixmap()
{
    QPixmap result{ rect().size() };
    result.fill(Qt::yellow);
    QPainter painter(&result);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QStyleOptionViewItem opt = m_option;
    opt.rect.setWidth(m_option.rect.width() - m_rowRightOffset);
    auto const idx = static_cast<NoteListModel *>(m_view->model())->getNoteIndex(m_id);
    paintBackground(&painter, opt, idx);
    paintLabels(&painter, m_option, idx);
    return result;
}

void NoteListDelegateEditor::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QStyleOptionViewItem opt = m_option;
    opt.rect.setWidth(m_option.rect.width() - m_rowRightOffset);
    auto const idx = static_cast<NoteListModel *>(m_view->model())->getNoteIndex(m_id);
    paintBackground(&painter, opt, idx);
    paintLabels(&painter, m_option, idx);
    QWidget::paintEvent(event);
}

void NoteListDelegateEditor::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_delegate->isInAllNotes()) {
        int y = 90;
        auto const *noteListModel = static_cast<NoteListModel *>(m_view->model());
        auto const idx = noteListModel->getNoteIndex(m_id);
        if (noteListModel->hasPinnedNote() && (noteListModel->isFirstPinnedNote(idx) || noteListModel->isFirstUnpinnedNote(idx))) {
            y += 25;
        }
        int fourthYOffset = 0;
        if (noteListModel->isFirstUnpinnedNote(idx)) {
            fourthYOffset = note_list_constants::UNPINNED_HEADER_TO_NOTE_SPACE;
        }
        int fifthYOffset = 0;
        if (noteListModel->hasPinnedNote() && !m_view->isPinnedNotesCollapsed() && noteListModel->isFirstUnpinnedNote(idx)) {
            fifthYOffset = note_list_constants::LAST_PINNED_TO_UNPINNED_HEADER;
        }
        int yOffsets = fourthYOffset + fifthYOffset;
        y += yOffsets;

        m_tagListView->setGeometry(note_list_constants::LEFT_OFFSET_X - 5, y + 5, rect().width() - 15, m_tagListView->height());
    } else {
        int y = 70;
        auto const *noteListModel = static_cast<NoteListModel *>(m_view->model());
        auto const idx = noteListModel->getNoteIndex(m_id);
        if (noteListModel->hasPinnedNote() && (noteListModel->isFirstPinnedNote(idx) || noteListModel->isFirstUnpinnedNote(idx))) {
            y += 25;
        }
        int fourthYOffset = 0;
        if (noteListModel->isFirstUnpinnedNote(idx)) {
            fourthYOffset = note_list_constants::UNPINNED_HEADER_TO_NOTE_SPACE;
        }
        int fifthYOffset = 0;
        if (noteListModel->hasPinnedNote() && !m_view->isPinnedNotesCollapsed() && noteListModel->isFirstUnpinnedNote(idx)) {
            fifthYOffset = note_list_constants::LAST_PINNED_TO_UNPINNED_HEADER;
        }
        int yOffsets = fourthYOffset + fifthYOffset;

        y += yOffsets;

        m_tagListView->setGeometry(note_list_constants::LEFT_OFFSET_X - 5, y, rect().width() - 15, m_tagListView->height());
    }
    recalculateSize();
}

void NoteListDelegateEditor::dragEnterEvent(QDragEnterEvent *event)
{
    Q_UNUSED(event);
    m_containsMouse = true;
}

void NoteListDelegateEditor::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_containsMouse = false;
    Q_UNUSED(event);
}

void NoteListDelegateEditor::enterEvent(QEnterEvent *event)
{
    m_containsMouse = true;
    QWidget::enterEvent(event);
}

void NoteListDelegateEditor::leaveEvent(QEvent *event)
{
    m_containsMouse = false;
    QWidget::leaveEvent(event);
}

void NoteListDelegateEditor::dropEvent(QDropEvent *event)
{
    auto *noteListModel = static_cast<NoteListModel *>(m_view->model());
    if (noteListModel != nullptr) {
        auto index = noteListModel->getNoteIndex(m_id);
        if (index.isValid()) {
            noteListModel->dropMimeData(event->mimeData(), event->proposedAction(), index.row(), 0, QModelIndex());
        }
    }
}

void NoteListDelegateEditor::setActive(bool isActive)
{
    m_isActive = isActive;
}

void NoteListDelegateEditor::recalculateSize()
{
    QSize result;
    int rowHeight = 70;
    result.setHeight(rowHeight);
    if (m_delegate->isInAllNotes()) {
        result.setHeight(result.height() + 20);
    }
    result.setHeight(result.height() + m_tagListView->height() + 2);
    result.setWidth(rect().width());
    auto const *noteListModel = static_cast<NoteListModel *>(m_view->model());
    auto idx = noteListModel->getNoteIndex(m_id);
    if (noteListModel->hasPinnedNote() && (noteListModel->isFirstPinnedNote(idx) || noteListModel->isFirstUnpinnedNote(idx))) {
        result.setHeight(result.height() + 25);
    }
    if (noteListModel->hasPinnedNote() && m_view->isPinnedNotesCollapsed()) {
        auto isPinned = idx.data(NoteListModel::NoteIsPinned).value<bool>();
        if (isPinned) {
            if (noteListModel->isFirstPinnedNote(idx)) {
                result.setHeight(25);
            } else {
                result.setHeight(0);
            }
        } /*else if (model->hasPinnedNote() && model->isFirstUnpinnedNote(m_index)) {
            result.setHeight(result.height() + 25);
        }*/
    }
    int secondYOffset = 0;
    if (idx.row() > 0) {
        secondYOffset = note_list_constants::NEXT_NOTE_OFFSET;
    }
    int thirdYOffset = 0;
    if (noteListModel->isFirstPinnedNote(idx)) {
        thirdYOffset = note_list_constants::PINNED_HEADER_TO_NOTE_SPACE;
    }
    int fourthYOffset = 0;
    if (noteListModel->isFirstUnpinnedNote(idx)) {
        fourthYOffset = note_list_constants::UNPINNED_HEADER_TO_NOTE_SPACE;
    }

    int fifthYOffset = 0;
    if (noteListModel->hasPinnedNote() && !m_view->isPinnedNotesCollapsed() && noteListModel->isFirstUnpinnedNote(idx)) {
        fifthYOffset = note_list_constants::LAST_PINNED_TO_UNPINNED_HEADER;
    }

    int yOffsets = secondYOffset + thirdYOffset + fourthYOffset + fifthYOffset;
    if (m_delegate->isInAllNotes()) {
        result.setHeight(result.height() - 2 + note_list_constants::LAST_EL_SEP_SPACE + yOffsets);
    } else {
        result.setHeight(result.height() - 10 + note_list_constants::LAST_EL_SEP_SPACE + yOffsets);
    }
    emit updateSizeHint(m_id, result, idx);
}

void NoteListDelegateEditor::setScrollBarPos(int pos)
{
    m_tagListView->verticalScrollBar()->setValue(pos);
}

int NoteListDelegateEditor::getScrollBarPos()
{
    return m_tagListView->verticalScrollBar()->value();
}

void NoteListDelegateEditor::setRowRightOffset(int rowRightOffset)
{
    m_rowRightOffset = rowRightOffset;
}

void NoteListDelegateEditor::setTheme(Theme::Value theme)
{
    m_theme = theme;
    switch (m_theme) {
    case Theme::Light: {
        m_titleColor = QColor(26, 26, 26);
        m_dateColor = QColor(26, 26, 26);
        m_defaultColor = QColor(247, 247, 247);
        m_activeColor = QColor(218, 233, 239);
        m_notActiveColor = QColor(175, 212, 228);
        m_hoverColor = QColor(207, 207, 207);
        m_applicationInactiveColor = QColor(207, 207, 207);
        m_separatorColor = QColor(191, 191, 191);
        break;
    }
    case Theme::Dark: {
        m_titleColor = QColor(212, 212, 212);
        m_dateColor = QColor(212, 212, 212);
        m_defaultColor = QColor(25, 25, 25);
        m_activeColor = QColor(35, 52, 69, 127);
        m_notActiveColor = QColor(35, 52, 69);
        m_hoverColor = QColor(35, 52, 69);
        m_applicationInactiveColor = QColor(35, 52, 69);
        m_separatorColor = QColor(255, 255, 255, 127);
        break;
    }
    case Theme::Sepia: {
        m_titleColor = QColor(26, 26, 26);
        m_dateColor = QColor(26, 26, 26);
        m_defaultColor = QColor(251, 240, 217);
        m_activeColor = QColor(218, 233, 239);
        m_notActiveColor = QColor(175, 212, 228);
        m_hoverColor = QColor(207, 207, 207);
        m_applicationInactiveColor = QColor(207, 207, 207);
        m_separatorColor = QColor(191, 191, 191);
        break;
    }
    }
}

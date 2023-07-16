#include "notelistdelegateeditor.h"
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include <QFontDatabase>
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

NoteListDelegateEditor::NoteListDelegateEditor(const NoteListDelegate *delegate, NoteListView *view,
                                               const QStyleOptionViewItem &option,
                                               const QModelIndex &index, TagPool *tagPool,
                                               QWidget *parent)
    : QWidget(parent),
      m_delegate(delegate),
      m_option(option),
      m_id(index.data(NoteListModel::NoteID).toInt()),
      m_view(view),
      m_tagPool{ tagPool },
#ifdef __APPLE__
      m_displayFont(QFont(QStringLiteral("SF Pro Text")).exactMatch()
                            ? QStringLiteral("SF Pro Text")
                            : QStringLiteral("Roboto")),
#elif _WIN32
      m_displayFont(QFont(QStringLiteral("Segoe UI")).exactMatch() ? QStringLiteral("Segoe UI")
                                                                   : QStringLiteral("Roboto")),
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
      m_ActiveColor(218, 233, 239),
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
        auto model = dynamic_cast<NoteListModel *>(m_view->model());
        if (model) {
            auto m_index = model->getNoteIndex(m_id);
            if (model->hasPinnedNote()
                && (model->isFirstPinnedNote(m_index) || model->isFirstUnpinnedNote(m_index))) {
                y += 25;
            }
        }
        int fourthYOffset = 0;
        if (model && model->isFirstUnpinnedNote(index)) {
            fourthYOffset = NoteListConstant::unpinnedHeaderToNoteSpace;
        }
        int fifthYOffset = 0;
        if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
            && model->isFirstUnpinnedNote(index)) {
            fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;
        }
        int yOffsets = fourthYOffset + fifthYOffset;

        y += yOffsets;
        m_tagListView->setGeometry(10, y - 5, rect().width() - 15, m_tagListView->height());
    } else {
        int y = 70;
        auto model = dynamic_cast<NoteListModel *>(m_view->model());
        if (model) {
            auto m_index = model->getNoteIndex(m_id);
            if (model->hasPinnedNote()
                && (model->isFirstPinnedNote(m_index) || model->isFirstUnpinnedNote(m_index))) {
                y += 25;
            }
        }
        int fourthYOffset = 0;
        if (model && model->isFirstUnpinnedNote(index)) {
            fourthYOffset = NoteListConstant::unpinnedHeaderToNoteSpace;
        }
        int fifthYOffset = 0;
        if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
            && model->isFirstUnpinnedNote(index)) {
            fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;
        }
        int yOffsets = fourthYOffset + fifthYOffset;

        y += yOffsets;
        m_tagListView->setGeometry(10, y - 5, rect().width() - 15, m_tagListView->height());
    }
    connect(m_tagListView->verticalScrollBar(), &QScrollBar::valueChanged, this, [this] {
        auto idx = dynamic_cast<NoteListModel *>(m_view->model())->getNoteIndex(m_id);
        dynamic_cast<NoteListModel *>(m_view->model())
                ->setData(idx, getScrollBarPos(), NoteListModel::NoteTagListScrollbarPos);
    });
    QTimer::singleShot(0, this, [this] {
        auto idx = dynamic_cast<NoteListModel *>(m_view->model())->getNoteIndex(m_id);
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

void NoteListDelegateEditor::paintBackground(QPainter *painter, const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    auto bufferSize = rect().size();
    QPixmap buffer{ bufferSize };
    buffer.fill(Qt::transparent);
    QPainter bufferPainter{ &buffer };
    bufferPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QRect bufferRect = buffer.rect();
    auto model = dynamic_cast<NoteListModel *>(m_view->model());
    if (model && model->hasPinnedNote()
        && (model->isFirstPinnedNote(index) || model->isFirstUnpinnedNote(index))) {
        int fifthYOffset = 0;
        if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
            && model->isFirstUnpinnedNote(index)) {
            fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;
        }
        bufferRect.setY(bufferRect.y() + 25 + fifthYOffset);
    }
    auto isPinned = index.data(NoteListModel::NoteIsPinned).toBool();
    if (m_view->selectionModel()->isSelected(index)) {
        if (qApp->applicationState() == Qt::ApplicationActive) {
            if (m_isActive) {
                bufferPainter.fillRect(bufferRect, QBrush(m_ActiveColor));
                m_tagListView->setBackground(m_ActiveColor);
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
        if (m_view->isPinnedNotesCollapsed() && !isPinned) {
            bufferPainter.fillRect(bufferRect, QBrush(m_defaultColor));
            m_tagListView->setBackground(m_defaultColor);
        } else {
            bufferPainter.fillRect(bufferRect, QBrush(m_defaultColor));
            m_tagListView->setBackground(m_defaultColor);
        }
    }
    if (m_view->isDragging() && !isPinned && !m_view->isDraggingInsidePinned()) {
        if (model && model->isFirstUnpinnedNote(index)
            && (index.row() == (model->rowCount() - 1))) {
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
        } else if (model && model->isFirstUnpinnedNote(index)) {
            auto rect = bufferRect;
            rect.setHeight(4);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setWidth(3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            rect = bufferRect;
            rect.setLeft(rect.right() - 3);
            bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
        } else if (model && (index.row() == (model->rowCount() - 1))) {
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
    if (model && m_delegate && m_delegate->shouldPaintSeparator(index, *model)) {
        paintSeparator(&bufferPainter, option, index);
    }

    auto rowHeight = rect().height();
    painter->drawPixmap(rect(), buffer,
                        QRect{ 0, bufferSize.height() - rowHeight, rect().width(), rowHeight });
}

void NoteListDelegateEditor::paintLabels(QPainter *painter, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(option);

    QString title{ index.data(NoteListModel::NoteFullTitle).toString() };
    QFont titleFont =
            m_view->selectionModel()->isSelected(index) ? m_titleSelectedFont : m_titleFont;
    QFontMetrics fmTitle(titleFont);
    QRect fmRectTitle = fmTitle.boundingRect(title);

    QString date =
            parseDateTime(index.data(NoteListModel::NoteLastModificationDateTime).toDateTime());
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
    auto model = dynamic_cast<NoteListModel *>(m_view->model());
    int fifthYOffset = 0;
    if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
        && model->isFirstUnpinnedNote(index)) {
        fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;
    }
    if (model && model->hasPinnedNote()) {
        if (model->isFirstPinnedNote(index)) {
            QRect headerRect(rowPosX + NoteListConstant::leftOffsetX / 2, rowPosY,
                             rowWidth - NoteListConstant::leftOffsetX / 2, 25);
#ifdef __APPLE__
            int iconPointSizeOffset = 0;
#else
            int iconPointSizeOffset = -4;
#endif
            painter->setFont(QFont("Font Awesome 6 Free Solid", 14 + iconPointSizeOffset));
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
        } else if (model->isFirstUnpinnedNote(index)) {
            rowPosY += fifthYOffset;
            QRect headerRect(rowPosX + NoteListConstant::leftOffsetX / 2, rowPosY,
                             rowWidth - NoteListConstant::leftOffsetX / 2, 25);
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
        secondYOffset = NoteListConstant::nextNoteOffset;
    }
    int thirdYOffset = 0;
    if (model && model->isFirstPinnedNote(index)) {
        thirdYOffset = NoteListConstant::pinnedHeaderToNoteSpace;
    }
    int fourthYOffset = 0;
    if (model && model->isFirstUnpinnedNote(index)) {
        fourthYOffset = NoteListConstant::unpinnedHeaderToNoteSpace;
    }

    int yOffsets = secondYOffset + thirdYOffset + fourthYOffset;
    double titleRectPosX = rowPosX + NoteListConstant::leftOffsetX;
    double titleRectPosY = rowPosY;
    double titleRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
    double titleRectHeight = fmRectTitle.height() + NoteListConstant::topOffsetY + yOffsets;

    double dateRectPosX = rowPosX + NoteListConstant::leftOffsetX;
    double dateRectPosY = rowPosY + fmRectTitle.height() + NoteListConstant::topOffsetY + yOffsets;
    double dateRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
    double dateRectHeight = fmRectDate.height() + NoteListConstant::titleDateSpace;

    double contentRectPosX = rowPosX + NoteListConstant::leftOffsetX;
    double contentRectPosY = rowPosY + fmRectTitle.height() + fmRectDate.height()
            + NoteListConstant::topOffsetY + yOffsets;
    double contentRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
    double contentRectHeight = fmRectContent.height() + NoteListConstant::dateDescSpace;

    double folderNameRectPosX = 0;
    double folderNameRectPosY = 0;
    double folderNameRectWidth = 0;
    double folderNameRectHeight = 0;

    if (m_delegate->isInAllNotes()) {
        folderNameRectPosX = rowPosX + NoteListConstant::leftOffsetX + 20;
        folderNameRectPosY = rowPosY + fmRectContent.height() + fmRectTitle.height()
                + fmRectDate.height() + NoteListConstant::topOffsetY + yOffsets;
        folderNameRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
        folderNameRectHeight = fmRectParentName.height() + NoteListConstant::descFolderSpace;
    }
    auto drawStr = [painter](double posX, double posY, double width, double height, QColor color,
                             const QFont &font, const QString &str) {
        QRectF rect(posX, posY, width, height);
        painter->setPen(color);
        painter->setFont(font);
        painter->drawText(rect, Qt::AlignBottom, str);
    };

    // draw title & date
    title = fmTitle.elidedText(title, Qt::ElideRight, int(titleRectWidth));
    content = fmContent.elidedText(content, Qt::ElideRight, int(titleRectWidth));
    drawStr(titleRectPosX, titleRectPosY, titleRectWidth, titleRectHeight, m_titleColor, titleFont,
            title);
    drawStr(dateRectPosX, dateRectPosY, dateRectWidth, dateRectHeight, m_dateColor, m_dateFont,
            date);
    if (m_delegate->isInAllNotes()) {
        painter->drawImage(QRect(rowPosX + NoteListConstant::leftOffsetX,
                                 folderNameRectPosY + NoteListConstant::descFolderSpace, 16, 16),
                           m_folderIcon);
        drawStr(folderNameRectPosX, folderNameRectPosY, folderNameRectWidth, folderNameRectHeight,
                m_contentColor, titleFont, parentName);
    }
    drawStr(contentRectPosX, contentRectPosY, contentRectWidth, contentRectHeight, m_contentColor,
            titleFont, content);
}

void NoteListDelegateEditor::paintSeparator(QPainter *painter, const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    Q_UNUSED(index)
    Q_UNUSED(option);

    painter->setPen(QPen(m_separatorColor));
    const int leftOffsetX = NoteListConstant::leftOffsetX;
    int posX1 = rect().x() + leftOffsetX;
    int posX2 = rect().x() + rect().width() - leftOffsetX - 1;
    int posY = rect().y() + rect().height() - 1;

    painter->drawLine(QPoint(posX1, posY), QPoint(posX2, posY));
}

QString NoteListDelegateEditor::parseDateTime(const QDateTime &dateTime) const
{
    QLocale usLocale(QLocale("en_US"));

    auto currDateTime = QDateTime::currentDateTime();

    if (dateTime.date() == currDateTime.date()) {
        return usLocale.toString(dateTime.time(), "h:mm A");
    } else if (dateTime.daysTo(currDateTime) == 1) {
        return "Yesterday";
    } else if (dateTime.daysTo(currDateTime) >= 2 && dateTime.daysTo(currDateTime) <= 7) {
        return usLocale.toString(dateTime.date(), "dddd");
    }

    return dateTime.date().toString("M/d/yy");
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
    auto m_index = dynamic_cast<NoteListModel *>(m_view->model())->getNoteIndex(m_id);
    paintBackground(&painter, opt, m_index);
    paintLabels(&painter, m_option, m_index);
    return result;
}

void NoteListDelegateEditor::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QStyleOptionViewItem opt = m_option;
    opt.rect.setWidth(m_option.rect.width() - m_rowRightOffset);
    auto m_index = dynamic_cast<NoteListModel *>(m_view->model())->getNoteIndex(m_id);
    paintBackground(&painter, opt, m_index);
    paintLabels(&painter, m_option, m_index);
    QWidget::paintEvent(event);
}

void NoteListDelegateEditor::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_delegate->isInAllNotes()) {
        int y = 90;
        auto model = dynamic_cast<NoteListModel *>(m_view->model());
        if (model) {
            auto m_index = model->getNoteIndex(m_id);
            if (model->hasPinnedNote()
                && (model->isFirstPinnedNote(m_index) || model->isFirstUnpinnedNote(m_index))) {
                y += 25;
            }
            int fourthYOffset = 0;
            if (model && model->isFirstUnpinnedNote(m_index)) {
                fourthYOffset = NoteListConstant::unpinnedHeaderToNoteSpace;
            }
            int fifthYOffset = 0;
            if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
                && model->isFirstUnpinnedNote(m_index)) {
                fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;
            }
            int yOffsets = fourthYOffset + fifthYOffset;

            y += yOffsets;
        }
        m_tagListView->setGeometry(NoteListConstant::leftOffsetX - 5, y + 5, rect().width() - 15,
                                   m_tagListView->height());
    } else {
        int y = 70;
        auto model = dynamic_cast<NoteListModel *>(m_view->model());
        if (model) {
            auto m_index = model->getNoteIndex(m_id);
            if (model->hasPinnedNote()
                && (model->isFirstPinnedNote(m_index) || model->isFirstUnpinnedNote(m_index))) {
                y += 25;
            }
            int fourthYOffset = 0;
            if (model && model->isFirstUnpinnedNote(m_index)) {
                fourthYOffset = NoteListConstant::unpinnedHeaderToNoteSpace;
            }
            int fifthYOffset = 0;
            if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
                && model->isFirstUnpinnedNote(m_index)) {
                fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;
            }
            int yOffsets = fourthYOffset + fifthYOffset;

            y += yOffsets;
        }
        m_tagListView->setGeometry(NoteListConstant::leftOffsetX - 5, y, rect().width() - 15,
                                   m_tagListView->height());
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void NoteListDelegateEditor::enterEvent(QEnterEvent *event)
#else
void NoteListDelegateEditor::enterEvent(QEvent *event)
#endif
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
    auto model = dynamic_cast<NoteListModel *>(m_view->model());
    if (model) {
        auto index = model->getNoteIndex(m_id);
        if (index.isValid()) {
            model->dropMimeData(event->mimeData(), event->proposedAction(), index.row(), 0,
                                QModelIndex());
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
    auto model = dynamic_cast<NoteListModel *>(m_view->model());
    if (model) {
        auto m_index = model->getNoteIndex(m_id);
        if (model->hasPinnedNote()
            && (model->isFirstPinnedNote(m_index) || model->isFirstUnpinnedNote(m_index))) {
            result.setHeight(result.height() + 25);
        }
        if (model->hasPinnedNote() && m_view->isPinnedNotesCollapsed()) {
            auto isPinned = m_index.data(NoteListModel::NoteIsPinned).value<bool>();
            if (isPinned) {
                if (model->isFirstPinnedNote(m_index)) {
                    result.setHeight(25);
                } else {
                    result.setHeight(0);
                }
            } /*else if (model->hasPinnedNote() && model->isFirstUnpinnedNote(m_index)) {
                result.setHeight(result.height() + 25);
            }*/
        }
        int secondYOffset = 0;
        if (m_index.row() > 0) {
            secondYOffset = NoteListConstant::nextNoteOffset;
        }
        int thirdYOffset = 0;
        if (model && model->isFirstPinnedNote(m_index)) {
            thirdYOffset = NoteListConstant::pinnedHeaderToNoteSpace;
        }
        int fourthYOffset = 0;
        if (model && model->isFirstUnpinnedNote(m_index)) {
            fourthYOffset = NoteListConstant::unpinnedHeaderToNoteSpace;
        }

        int fifthYOffset = 0;
        if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
            && model->isFirstUnpinnedNote(m_index)) {
            fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;
        }

        int yOffsets = secondYOffset + thirdYOffset + fourthYOffset + fifthYOffset;
        if (m_delegate->isInAllNotes()) {
            result.setHeight(result.height() - 2 + NoteListConstant::lastElSepSpace + yOffsets);
        } else {
            result.setHeight(result.height() - 10 + NoteListConstant::lastElSepSpace + yOffsets);
        }
        emit updateSizeHint(m_id, result, m_index);
    }
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
        m_ActiveColor = QColor(218, 233, 239);
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
        m_ActiveColor = QColor(35, 52, 69, 127);
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
        m_ActiveColor = QColor(218, 233, 239);
        m_notActiveColor = QColor(175, 212, 228);
        m_hoverColor = QColor(207, 207, 207);
        m_applicationInactiveColor = QColor(207, 207, 207);
        m_separatorColor = QColor(191, 191, 191);
        break;
    }
    }
}

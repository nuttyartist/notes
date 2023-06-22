#include "notelistdelegate.h"
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include <QFontDatabase>
#include <QtMath>
#include <QPainterPath>
#include "notelistmodel.h"
#include "noteeditorlogic.h"
#include "tagpool.h"
#include "nodepath.h"
#include "notelistdelegateeditor.h"

NoteListDelegate::NoteListDelegate(NoteListView *view, TagPool *tagPool, QObject *parent)
    : QStyledItemDelegate(parent),
      m_view{ view },
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
      m_maxFrame(200),
      m_rowRightOffset(0),
      m_state(NoteListState::Normal),
      m_isActive(false),
      m_isInAllNotes(false),
      m_theme(Theme::Light)
{
    m_timeLine = new QTimeLine(300, this);
    m_timeLine->setFrameRange(0, m_maxFrame);
    m_timeLine->setUpdateInterval(10);
    m_timeLine->setEasingCurve(QEasingCurve::InCurve);
    m_folderIcon = QImage(":/images/folder.png");
    connect(m_timeLine, &QTimeLine::frameChanged, this, [this]() {
        for (const auto &index : qAsConst(m_animatedIndexes)) {
            emit sizeHintChanged(index);
        }
    });

    connect(m_timeLine, &QTimeLine::finished, this, [this]() {
        emit animationFinished(m_state);
        for (const auto &index : qAsConst(m_animatedIndexes)) {
            m_view->openPersistentEditorC(index);
        }
        if (!animationQueue.empty()) {
            auto a = animationQueue.front();
            animationQueue.pop_front();
            QModelIndexList indexes;
            for (const auto &id : qAsConst(a.first)) {
                auto model = dynamic_cast<NoteListModel *>(m_view->model());
                if (model) {
                    auto index = model->getNoteIndex(id);
                    if (index.isValid()) {
                        indexes.push_back(index);
                    }
                }
            }
            setStateI(a.second, indexes);
        } else {
            m_animatedIndexes.clear();
            m_state = NoteListState::Normal;
        }
    });
}

void NoteListDelegate::setState(NoteListState NewState, QModelIndexList indexes)
{
    if (animationState() != QTimeLine::NotRunning) {
        QSet<int> ids;
        for (const auto &index : qAsConst(indexes)) {
            auto noteId = index.data(NoteListModel::NoteID).toInt();
            if (noteId != SpecialNodeID::InvalidNodeId) {
                ids.insert(noteId);
            }
        }
        if (!ids.empty()) {
            animationQueue.push_back(qMakePair(ids, NewState));
        }
    } else {
        setStateI(NewState, indexes);
    }
}

void NoteListDelegate::setAnimationDuration(const int duration)
{
    m_timeLine->setDuration(duration);
}

void NoteListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    bool isHaveTags = index.data(NoteListModel::NoteTagsList).value<QSet<int>>().size() > 0;
    if ((!m_animatedIndexes.contains(index)) && isHaveTags) {
        return;
    }
    if (m_view->isPinnedNotesCollapsed()) {
        auto model = dynamic_cast<NoteListModel *>(m_view->model());
        auto isPinned = index.data(NoteListModel::NoteIsPinned).value<bool>();
        if (isPinned) {
            if (model && (!model->isFirstPinnedNote(index))) {
                return;
            }
        }
    }

    painter->setRenderHint(QPainter::Antialiasing);
    QStyleOptionViewItem opt = option;
    opt.rect.setWidth(option.rect.width() - m_rowRightOffset);

    int currentFrame = m_timeLine->currentFrame();
    double rate = (currentFrame / (m_maxFrame * 1.0));
    double height = m_rowHeight * rate;

    switch (m_state) {
    case NoteListState::Insert:
    case NoteListState::Remove:
    case NoteListState::MoveOut:
        if (m_animatedIndexes.contains(index)) {
            opt.rect.setHeight(int(height));
            opt.backgroundBrush.setColor(m_notActiveColor);
        }
        break;
    case NoteListState::MoveIn:
        break;
    case NoteListState::Normal:
        break;
    }

    paintBackground(painter, opt, index);
    paintLabels(painter, option, index);
}

QSize NoteListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize result; // = QStyledItemDelegate::sizeHint(option, index);
    result.setWidth(option.rect.width());
    auto model = dynamic_cast<NoteListModel *>(m_view->model());
    const auto &note = model->getNote(index);
    auto id = note.id();
    bool isHaveTags = note.tagIds().size() > 0;
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    if ((!m_animatedIndexes.contains(index)) && isHaveTags) {
#else
    if (m_view->isPersistentEditorOpen(index) && (!m_animatedIndexes.contains(index))
        && isHaveTags) {
#endif
        if (szMap.contains(id)) {
            result.setHeight(szMap[id].height());
            return result;
        }
    }
    int rowHeight = 70;
    if (isHaveTags) {
        rowHeight = m_rowHeight;
    }
    if (m_animatedIndexes.contains(index)) {
        if (m_state == NoteListState::MoveIn) {
            result.setHeight(rowHeight);
        } else {
            double rate = m_timeLine->currentFrame() / (m_maxFrame * 1.0);
            double height = rowHeight * rate;
            result.setHeight(int(height));
        }
    } else {
        result.setHeight(rowHeight);
    }
    if (m_isInAllNotes) {
        result.setHeight(result.height() + 20);
    }
    if (m_view->isPinnedNotesCollapsed()) {
        auto isPinned = note.isPinnedNote();
        if (isPinned) {
            if (model && model->isFirstPinnedNote(index)) {
                result.setHeight(25);
                return result;
            } else {
                result.setHeight(0);
                return result;
            }
        } else if (model->hasPinnedNote() && model->isFirstUnpinnedNote(index)) {
            result.setHeight(result.height() + 25);
        }
    } else {
        if (model->hasPinnedNote()
            && (model->isFirstPinnedNote(index) || model->isFirstUnpinnedNote(index))) {
            result.setHeight(result.height() + 25);
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
    int fifthYOffset = 0;
    if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
        && model->isFirstUnpinnedNote(index)) {
        fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;
    }

    int yOffsets = secondYOffset + thirdYOffset + fourthYOffset + fifthYOffset;
    if (m_isInAllNotes) {
        result.setHeight(result.height() - 2 + NoteListConstant::lastElSepSpace + yOffsets);
    } else {
        result.setHeight(result.height() - 10 + NoteListConstant::lastElSepSpace + yOffsets);
    }
    return result;
}

QSize NoteListDelegate::bufferSizeHint(const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    QSize result = QStyledItemDelegate::sizeHint(option, index);
    result.setWidth(option.rect.width());
    auto id = index.data(NoteListModel::NoteID).toInt();
    bool isHaveTags = index.data(NoteListModel::NoteTagsList).value<QSet<int>>().size() > 0;
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    if (!m_animatedIndexes.contains(index) && isHaveTags) {
#else
    if (m_view->isPersistentEditorOpen(index) && (!m_animatedIndexes.contains(index))
        && isHaveTags) {
#endif
        if (szMap.contains(id)) {
            result.setHeight(szMap[id].height());
            return result;
        }
    }
    int rowHeight = 70;
    if (isHaveTags) {
        rowHeight = m_rowHeight;
    }
    result.setHeight(rowHeight);
    if (m_isInAllNotes) {
        result.setHeight(result.height() + 20);
    }
    auto model = dynamic_cast<NoteListModel *>(m_view->model());
    //    if (model) {
    //        if (model->hasPinnedNote() &&
    //                (model->isFirstPinnedNote(index) || model->isFirstUnpinnedNote(index))) {
    //            result.setHeight(result.height() + 25);
    //        }
    //    }
    if (m_view->isPinnedNotesCollapsed()) {
        auto isPinned = index.data(NoteListModel::NoteIsPinned).value<bool>();
        if (isPinned) {
            if (model && model->isFirstPinnedNote(index)) {
                result.setHeight(25);
                return result;
            } else {
                result.setHeight(0);
                return result;
            }
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
    //    int fifthYOffset = 0;
    //    if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
    //            && model->isFirstUnpinnedNote(index)) {
    //        fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;
    //    }

    int yOffsets = secondYOffset + thirdYOffset + fourthYOffset; // + fifthYOffset;
    if (m_isInAllNotes) {
        result.setHeight(result.height() - 2 + NoteListConstant::lastElSepSpace + yOffsets);
    } else {
        result.setHeight(result.height() - 10 + NoteListConstant::lastElSepSpace + yOffsets);
    }
    return result;
}

QTimeLine::State NoteListDelegate::animationState()
{
    return m_timeLine->state();
}

void NoteListDelegate::paintBackground(QPainter *painter, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    auto bufferSize = bufferSizeHint(option, index);
    QPixmap buffer{ bufferSize };
    buffer.fill(Qt::transparent);
    QPainter bufferPainter{ &buffer };
    bufferPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QRect bufferRect = buffer.rect();
    auto isPinned = index.data(NoteListModel::NoteIsPinned).toBool();
    auto model = dynamic_cast<NoteListModel *>(m_view->model());
    if (model && model->hasPinnedNote() && model->isFirstPinnedNote(index)
        && dynamic_cast<NoteListView *>(m_view)->isPinnedNotesCollapsed()) {
        bufferPainter.fillRect(bufferRect, QBrush(m_defaultColor));
    } else if ((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
        if (qApp->applicationState() == Qt::ApplicationActive) {
            if (m_isActive) {
                bufferPainter.fillRect(bufferRect, QBrush(m_ActiveColor));
            } else {
                bufferPainter.fillRect(bufferRect, QBrush(m_notActiveColor));
            }
        } else if (qApp->applicationState() == Qt::ApplicationInactive) {
            bufferPainter.fillRect(bufferRect, QBrush(m_applicationInactiveColor));
        }
    } else if ((option.state & QStyle::State_MouseOver) == QStyle::State_MouseOver) {
        if (dynamic_cast<NoteListView *>(m_view)->isDragging()) {
            if (isPinned) {
                auto rect = bufferRect;
                rect.setTop(rect.bottom() - 5);
                bufferPainter.fillRect(rect, QBrush("#d6d5d5"));
            }
        } else {
            bufferPainter.fillRect(bufferRect, QBrush(m_hoverColor));
        }
    } else {
        if (m_view->isPinnedNotesCollapsed()) {
            if (!isPinned) {
                bufferPainter.fillRect(bufferRect, QBrush(m_defaultColor));
            }
        } else {
            bufferPainter.fillRect(bufferRect, QBrush(m_defaultColor));
        }
    }
    if (dynamic_cast<NoteListView *>(m_view)->isDragging() && !isPinned
        && !dynamic_cast<NoteListView *>(m_view)->isDraggingInsidePinned()) {
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

    if (model && shouldPaintSeparator(index, *model)) {
        paintSeparator(&bufferPainter, bufferRect, index);
    }
    int rowHeight;
    if (m_animatedIndexes.contains(index)) {
        if (m_state != NoteListState::MoveIn) {
            double rowRate = m_timeLine->currentFrame() / (m_maxFrame * 1.0);
            rowHeight = bufferSize.height() * rowRate;
        } else {
            double rowRate = 1.0 - m_timeLine->currentFrame() / (m_maxFrame * 1.0);
            rowHeight = bufferSize.height() * rowRate;
        }
    } else {
        rowHeight = option.rect.height();
    }
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (m_animatedIndexes.contains(index)) {
        if (m_state == NoteListState::MoveIn) {
            if (model && model->hasPinnedNote()
                && (model->isFirstPinnedNote(index) || model->isFirstUnpinnedNote(index))) {
                painter->drawPixmap(QRect{ option.rect.x(),
                                           option.rect.y() + bufferSize.height() - rowHeight + 25,
                                           option.rect.width(), rowHeight },
                                    buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                           rowHeight });
            } else {
                painter->drawPixmap(QRect{ option.rect.x(),
                                           option.rect.y() + bufferSize.height() - rowHeight,
                                           option.rect.width(), rowHeight },
                                    buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                           rowHeight });
            }
        } else {
            if (model && model->hasPinnedNote()
                && (model->isFirstPinnedNote(index) || model->isFirstUnpinnedNote(index))) {
                painter->drawPixmap(QRect{ option.rect.x(), option.rect.y() + 25,
                                           option.rect.width(), option.rect.height() },
                                    buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                           rowHeight });
            } else {
                painter->drawPixmap(option.rect, buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                           rowHeight });
            }
        }
    } else {
        painter->drawPixmap(
                option.rect, buffer,
                QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(), rowHeight });
    }
}

void NoteListDelegate::paintLabels(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    if (m_animatedIndexes.contains(index)) {
        auto bufferSize = bufferSizeHint(option, index);
        QPixmap buffer{ bufferSize };
        buffer.fill(Qt::transparent);
        QPainter bufferPainter{ &buffer };
        bufferPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        QString title{ index.data(NoteListModel::NoteFullTitle).toString() };
        QFont titleFont = (option.state & QStyle::State_Selected) == QStyle::State_Selected
                ? m_titleSelectedFont
                : m_titleFont;
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
        double rowPosX = 0; // option.rect.x();
        double rowPosY = 0; // option.rect.y();
        auto model = dynamic_cast<NoteListModel *>(m_view->model());
        if (m_view->isPinnedNotesCollapsed()) {
            auto isPinned = index.data(NoteListModel::NoteIsPinned).value<bool>();
            if (isPinned) {
                return;
            }
        }
        double rowWidth = option.rect.width();
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

        int fifthYOffset = 0;
        if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
            && model->isFirstUnpinnedNote(index)) {
            fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;
        }

        int yOffsets = secondYOffset + thirdYOffset + fourthYOffset + fifthYOffset;
        double titleRectPosX = rowPosX + NoteListConstant::leftOffsetX;
        double titleRectPosY = rowPosY;
        double titleRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
        double titleRectHeight = fmRectTitle.height() + NoteListConstant::topOffsetY + yOffsets;

        double dateRectPosX = rowPosX + NoteListConstant::leftOffsetX;
        double dateRectPosY =
                rowPosY + fmRectTitle.height() + NoteListConstant::topOffsetY + yOffsets;
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

        if (m_isInAllNotes) {
            folderNameRectPosX = rowPosX + NoteListConstant::leftOffsetX + 20;
            folderNameRectPosY = rowPosY + fmRectContent.height() + fmRectTitle.height()
                    + fmRectDate.height() + NoteListConstant::topOffsetY + yOffsets;
            folderNameRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
            folderNameRectHeight = fmRectParentName.height() + NoteListConstant::descFolderSpace;
        }

        auto drawStr = [&bufferPainter](double posX, double posY, double width, double height,
                                        QColor color, const QFont &font, const QString &str) {
            QRectF rect(posX, posY, width, height);
            bufferPainter.setPen(color);
            bufferPainter.setFont(font);
            bufferPainter.drawText(rect, Qt::AlignBottom, str);
        };
        // draw title & date
        title = fmTitle.elidedText(title, Qt::ElideRight, int(titleRectWidth));
        content = fmContent.elidedText(content, Qt::ElideRight, int(titleRectWidth));
        drawStr(titleRectPosX, titleRectPosY, titleRectWidth, titleRectHeight, m_titleColor,
                titleFont, title);
        drawStr(dateRectPosX, dateRectPosY, dateRectWidth, dateRectHeight, m_dateColor, m_dateFont,
                date);
        if (m_isInAllNotes) {
            bufferPainter.drawImage(QRect(rowPosX + NoteListConstant::leftOffsetX,
                                          folderNameRectPosY + NoteListConstant::descFolderSpace,
                                          16, 16),
                                    m_folderIcon);
            drawStr(folderNameRectPosX, folderNameRectPosY, folderNameRectWidth,
                    folderNameRectHeight, m_contentColor, titleFont, parentName);
        }
        drawStr(contentRectPosX, contentRectPosY, contentRectWidth, contentRectHeight,
                m_contentColor, titleFont, content);
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        int rowHeight;
        if (m_animatedIndexes.contains(index)) {
            if (m_state != NoteListState::MoveIn) {
                double rowRate = m_timeLine->currentFrame() / (m_maxFrame * 1.0);
                rowHeight = bufferSize.height() * rowRate;
            } else {
                double rowRate = 1.0 - m_timeLine->currentFrame() / (m_maxFrame * 1.0);
                rowHeight = bufferSize.height() * rowRate;
            }
        } else {
            rowHeight = option.rect.height();
        }

        if (m_state == NoteListState::MoveIn) {
            if (model && model->hasPinnedNote()
                && (model->isFirstPinnedNote(index) || model->isFirstUnpinnedNote(index))) {
                painter->drawPixmap(QRect{ option.rect.x(),
                                           option.rect.y() + bufferSize.height() - rowHeight + 25,
                                           option.rect.width(), rowHeight },
                                    buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                           rowHeight });
            } else {
                painter->drawPixmap(QRect{ option.rect.x(),
                                           option.rect.y() + bufferSize.height() - rowHeight,
                                           option.rect.width(), rowHeight },
                                    buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                           rowHeight });
            }
        } else {
            if (model && model->hasPinnedNote()
                && (model->isFirstPinnedNote(index) || model->isFirstUnpinnedNote(index))) {
                painter->drawPixmap(QRect{ option.rect.x(), option.rect.y() + 25,
                                           option.rect.width(), rowHeight },
                                    buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                           rowHeight });
            } else {
                painter->drawPixmap(option.rect, buffer,
                                    QRect{ 0, bufferSize.height() - rowHeight, option.rect.width(),
                                           rowHeight });
            }
        }
        if (model && model->hasPinnedNote()) {
            if (model->isFirstPinnedNote(index)) {
                QRect headerRect(option.rect.x() + NoteListConstant::leftOffsetX / 2,
                                 option.rect.y(),
                                 option.rect.width() - NoteListConstant::leftOffsetX / 2, 25);
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
            } else if (model->hasPinnedNote() && model->isFirstUnpinnedNote(index)) {
                QRect headerRect(option.rect.x() + NoteListConstant::leftOffsetX / 2,
                                 option.rect.y() + fifthYOffset,
                                 option.rect.width() - NoteListConstant::leftOffsetX / 2, 25);
                painter->setPen(m_contentColor);
                painter->setFont(m_headerFont);
                painter->drawText(headerRect, Qt::AlignLeft | Qt::AlignVCenter, "Notes");
            }
        }
    } else {
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

        double rowPosX = option.rect.x();
        double rowPosY = option.rect.y();
        auto model = dynamic_cast<NoteListModel *>(m_view->model());
        int fifthYOffset = 0;
        if (model && model->hasPinnedNote() && !m_view->isPinnedNotesCollapsed()
            && model->isFirstUnpinnedNote(index)) {
            fifthYOffset = NoteListConstant::lastPinnedToUnpinnedHeader;
        }
        if (model) {
            if (model->isFirstPinnedNote(index)) {
                QRect headerRect(rowPosX + NoteListConstant::leftOffsetX / 2, rowPosY,
                                 option.rect.width() - NoteListConstant::leftOffsetX / 2, 25);
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
            } else if (model->hasPinnedNote() && model->isFirstUnpinnedNote(index)) {
                rowPosY += fifthYOffset;
                QRect headerRect(rowPosX + NoteListConstant::leftOffsetX / 2, rowPosY,
                                 option.rect.width() - NoteListConstant::leftOffsetX / 2, 25);
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
        double rowWidth = option.rect.width();
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
        double dateRectPosY =
                rowPosY + fmRectTitle.height() + NoteListConstant::topOffsetY + yOffsets;
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

        if (isInAllNotes()) {
            folderNameRectPosX = rowPosX + NoteListConstant::leftOffsetX + 20;
            folderNameRectPosY = rowPosY + fmRectContent.height() + fmRectTitle.height()
                    + fmRectDate.height() + NoteListConstant::topOffsetY + yOffsets;
            folderNameRectWidth = rowWidth - 2.0 * NoteListConstant::leftOffsetX;
            folderNameRectHeight = fmRectParentName.height() + NoteListConstant::descFolderSpace;
        }
        auto drawStr = [painter](double posX, double posY, double width, double height,
                                 QColor color, const QFont &font, const QString &str) {
            QRectF rect(posX, posY, width, height);
            painter->setPen(color);
            painter->setFont(font);
            painter->drawText(rect, Qt::AlignBottom, str);
        };

        // draw title & date
        title = fmTitle.elidedText(title, Qt::ElideRight, int(titleRectWidth));
        content = fmContent.elidedText(content, Qt::ElideRight, int(titleRectWidth));
        drawStr(titleRectPosX, titleRectPosY, titleRectWidth, titleRectHeight, m_titleColor,
                titleFont, title);
        drawStr(dateRectPosX, dateRectPosY, dateRectWidth, dateRectHeight, m_dateColor, m_dateFont,
                date);
        if (isInAllNotes()) {
            painter->drawImage(QRect(rowPosX + NoteListConstant::leftOffsetX,
                                     folderNameRectPosY + NoteListConstant::descFolderSpace, 16,
                                     16),
                               m_folderIcon);
            drawStr(folderNameRectPosX, folderNameRectPosY, folderNameRectWidth,
                    folderNameRectHeight, m_contentColor, titleFont, parentName);
        }
        drawStr(contentRectPosX, contentRectPosY, contentRectWidth, contentRectHeight,
                m_contentColor, titleFont, content);
    }
}

void NoteListDelegate::paintSeparator(QPainter *painter, QRect rect, const QModelIndex &index) const
{
    Q_UNUSED(index);
    painter->setPen(QPen(m_separatorColor));
    const int leftOffsetX = NoteListConstant::leftOffsetX;
    int posX1 = rect.x() + leftOffsetX;
    int posX2 = rect.x() + rect.width() - leftOffsetX - 1;
    int posY = rect.y() + rect.height();

    painter->drawLine(QPoint(posX1, posY), QPoint(posX2, posY));
}

void NoteListDelegate::paintTagList(int top, QPainter *painter, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    auto tagIds = index.data(NoteListModel::NoteTagsList).value<QSet<int>>();
    int left = option.rect.x() + 10;

    for (const auto &id : qAsConst(tagIds)) {
        if (left >= option.rect.width()) {
            break;
        }
        auto tag = m_tagPool->getTag(id);
        QFontMetrics fmName(m_titleFont);
        QRect fmRectName = fmName.boundingRect(tag.name());

        auto rect = option.rect;
        rect.setTop(top);
        rect.setLeft(left);
        rect.setHeight(20);
        rect.setWidth(5 + 12 + 5 + fmRectName.width() + 7);
        left += rect.width() + 5;
        QPainterPath path;
        path.addRoundedRect(rect, 10, 10);
        if (m_theme == Theme::Dark) {
            painter->fillPath(path, QColor(76, 85, 97));
        } else if ((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
            painter->fillPath(path, QColor(218, 235, 248));
        } else {
            painter->fillPath(path, QColor(227, 234, 243));
        }
        auto iconRect = QRect(rect.x() + 5, rect.y() + (rect.height() - 12) / 2, 14, 14);
        painter->setPen(QColor(tag.color()));
#ifdef __APPLE__
        int iconPointSizeOffset = 0;
#else
        int iconPointSizeOffset = -4;
#endif
        painter->setFont(QFont("Font Awesome 6 Free Solid", 14 + iconPointSizeOffset));
        painter->drawText(iconRect, u8"\uf111"); // fa-circle
        painter->setBrush(m_titleColor);
        painter->setPen(m_titleColor);

        QRect nameRect(rect);
        nameRect.setLeft(iconRect.x() + iconRect.width() + 5);
        painter->setFont(m_titleFont);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, tag.name());
    }
}

bool NoteListDelegate::shouldPaintSeparator(const QModelIndex &index,
                                            const NoteListModel &model) const
{
    if (index.row() == model.rowCount() - 1) {
        return false;
    }
    const auto &selectedIndexes = m_view->selectedIndex();
    bool isCurrentSelected = selectedIndexes.contains(index);
    bool isNextRowSelected = false;
    for (const auto &selected : selectedIndexes) {
        if (index.row() == selected.row() - 1) {
            isNextRowSelected = true;
        }
    }
    if (!isCurrentSelected
        && ((index.row() == m_hoveredIndex.row() - 1) || index.row() == m_hoveredIndex.row())) {
        return false;
    }
    if (isCurrentSelected == isNextRowSelected) {
        if (index.row() != model.getFirstUnpinnedNote().row() - 1) {
            if (m_view && m_view->isPinnedNotesCollapsed()) {
                auto isPinned = index.data(NoteListModel::NoteIsPinned).value<bool>();
                if (!isPinned) {
                    return true;
                }
            } else {
                return true;
            }
        }
    }
    return false;
}

QString NoteListDelegate::parseDateTime(const QDateTime &dateTime) const
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

void NoteListDelegate::setStateI(NoteListState NewState, const QModelIndexList &indexes)
{
    m_animatedIndexes = indexes;

    auto startAnimation = [this](QTimeLine::Direction diretion, int duration) {
        for (const auto &index : qAsConst(m_animatedIndexes)) {
            m_view->closePersistentEditorC(index);
        }
        m_timeLine->setDirection(diretion);
        m_timeLine->setDuration(duration);
        m_timeLine->start();
    };

    switch (NewState) {
    case NoteListState::Insert:
        startAnimation(QTimeLine::Forward, m_maxFrame);
        break;
    case NoteListState::Remove:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case NoteListState::MoveOut:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case NoteListState::MoveIn:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case NoteListState::Normal:
        m_animatedIndexes.clear();
        break;
    }

    m_state = NewState;
}

const QModelIndex &NoteListDelegate::hoveredIndex() const
{
    return m_hoveredIndex;
}

bool NoteListDelegate::isInAllNotes() const
{
    return m_isInAllNotes;
}

Theme::Value NoteListDelegate::theme() const
{
    return m_theme;
}

void NoteListDelegate::setIsInAllNotes(bool newIsInAllNotes)
{
    m_isInAllNotes = newIsInAllNotes;
}

void NoteListDelegate::clearSizeMap()
{
    szMap.clear();
}

void NoteListDelegate::updateSizeMap(int id, QSize sz, const QModelIndex &index)
{
    szMap[id] = sz;
    emit sizeHintChanged(index);
}

void NoteListDelegate::editorDestroyed(int id, const QModelIndex &index)
{
    szMap.remove(id);
    emit sizeHintChanged(index);
}

QWidget *NoteListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    if (!index.isValid()) {
        return nullptr;
    }
    auto model = dynamic_cast<NoteListModel *>(m_view->model());
    if (m_view->isPinnedNotesCollapsed()) {
        auto isPinned = index.data(NoteListModel::NoteIsPinned).value<bool>();
        if (isPinned) {
            if (model && (!model->isFirstPinnedNote(index))) {
                return nullptr;
            }
        }
    }
    bool isHaveTags = index.data(NoteListModel::NoteTagsList).value<QSet<int>>().size() > 0;
    if (!isHaveTags) {
        return nullptr;
    }
    auto w = new NoteListDelegateEditor(this, m_view, option, index, m_tagPool, parent);
    w->setTheme(m_theme);
    connect(this, &NoteListDelegate::themeChanged, w, &NoteListDelegateEditor::setTheme);
    connect(w, &NoteListDelegateEditor::updateSizeHint, this, &NoteListDelegate::updateSizeMap);
    connect(w, &NoteListDelegateEditor::nearDestroyed, this, &NoteListDelegate::editorDestroyed);
    w->recalculateSize();
    return w;
}

void NoteListDelegate::setActive(bool isActive)
{
    m_isActive = isActive;
}

void NoteListDelegate::setRowRightOffset(int rowRightOffset)
{
    m_rowRightOffset = rowRightOffset;
}

void NoteListDelegate::setHoveredIndex(const QModelIndex &hoveredIndex)
{
    m_hoveredIndex = hoveredIndex;
}

void NoteListDelegate::setTheme(Theme::Value theme)
{
    m_theme = theme;
    switch (m_theme) {
    case Theme::Light: {
        m_titleColor = QColor(26, 26, 26);
        m_dateColor = QColor(26, 26, 26);
        m_contentColor = QColor(142, 146, 150);
        m_defaultColor = QColor(247, 247, 247);
        m_ActiveColor = QColor(218, 233, 239);
        m_notActiveColor = QColor(175, 212, 228);
        m_hoverColor = QColor(207, 207, 207);
        m_applicationInactiveColor = QColor(207, 207, 207);
        m_separatorColor = QColor(191, 191, 191);
        break;
    }
    case Theme::Dark: {
        m_titleColor = QColor(255, 255, 255);
        m_dateColor = QColor(255, 255, 255);
        m_contentColor = QColor(255, 255, 255, 127);
        m_defaultColor = QColor(25, 25, 25);
        m_ActiveColor = QColor(35, 52, 69, 127);
        m_notActiveColor = QColor(35, 52, 69);
        m_hoverColor = QColor(35, 52, 69, 127);
        m_applicationInactiveColor = QColor(35, 52, 69);
        m_separatorColor = QColor(255, 255, 255, 127);
        break;
    }
    case Theme::Sepia: {
        m_titleColor = QColor(26, 26, 26);
        m_dateColor = QColor(26, 26, 26);
        m_contentColor = QColor(142, 146, 150);
        m_defaultColor = QColor(251, 240, 217);
        m_ActiveColor = QColor(218, 233, 239);
        m_notActiveColor = QColor(175, 212, 228);
        m_hoverColor = QColor(207, 207, 207);
        m_applicationInactiveColor = QColor(207, 207, 207);
        m_separatorColor = QColor(191, 191, 191);
        break;
    }
    }
    emit themeChanged(m_theme);
}

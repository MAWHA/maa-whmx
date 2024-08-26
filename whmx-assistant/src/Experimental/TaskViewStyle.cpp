/* Copyright 2024 周上行Ryer

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "TaskViewStyle.h"

#include <QtCore/QDebug>
#include <QtWidgets/QStyleOption>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <ElaTheme.h>

const auto theme        = ElaThemeType::Light;
const int  left_padding = 10;
const int  item_height  = 33;

namespace Experimental {

void TaskViewStyle::drawPrimitive(
    PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
    switch (element) {
        case QStyle::PE_PanelItemViewItem: {
            const auto opt = qstyleoption_cast<const QStyleOptionViewItem*>(option);
            Q_ASSERT(opt);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            const auto   item_bb = opt->rect.adjusted(0, 2, 0, -2);
            QPainterPath path;
            path.addRoundedRect(item_bb, 4, 4);
            if (opt->state.testFlag(QStyle::State_Selected)) {
                if (opt->state.testFlag(QStyle::State_MouseOver)) {
                    painter->fillPath(path, ElaThemeColor(theme, ListViewItemSelectedHover));
                } else {
                    painter->fillPath(path, ElaThemeColor(theme, ListViewItemSelected));
                }
            } else if (opt->state.testFlag(QStyle::State_MouseOver)) {
                painter->fillPath(path, ElaThemeColor(theme, ListViewItemHover));
            }
            painter->restore();
        } break;
        case QStyle::PE_IndicatorBranch: {
            const auto opt = qstyleoption_cast<const QStyleOptionViewItem*>(option);
            Q_ASSERT(opt);
            if (!opt->state.testFlag(QStyle::State_Children)) { break; }
            painter->save();
            const auto indicator_bb = option->rect.adjusted(0, 0, -2, 0);
            QFont      icon_font    = QFont("ElaAwesome");
            icon_font.setPixelSize(17);
            painter->setFont(icon_font);
            painter->setPen(ElaThemeColor(theme, WindowText));
            const auto indicator = opt->state.testFlag(QStyle::State_Open) ? ElaIconType::AngleDown : ElaIconType::AngleRight;
            painter->drawText(indicator_bb, Qt::AlignVCenter | Qt::AlignRight, QChar(indicator));
            painter->restore();
        } break;
        case QStyle::PE_PanelItemViewRow: {
        } break;
        case QStyle::PE_Widget: {
        } break;
        case QStyle::PE_Frame: {
        } break;
        default: {
            QProxyStyle::drawPrimitive(element, option, painter, widget);
        } break;
    }
}

void TaskViewStyle::drawControl(
    ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
    switch (element) {
        case QStyle::CE_ShapedFrame: {
            const auto frame_bb = option->rect.adjusted(1, 1, -1, -1);
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing);
            painter->setPen(ElaThemeColor(theme, TableViewBorder));
            painter->setBrush(ElaThemeColor(theme, TableViewBase));
            painter->drawRoundedRect(frame_bb, 3, 3);
            painter->restore();
        } break;
        case QStyle::CE_HeaderLabel: {
        } break;
        case QStyle::CE_HeaderSection: {
        } break;
        case QStyle::CE_ItemViewItem: {
            const auto opt = qstyleoption_cast<const QStyleOptionViewItem*>(option);
            Q_ASSERT(opt);

            //! background
            drawPrimitive(QStyle::PE_PanelItemViewItem, option, painter, widget);

            // content
            const auto item_bb  = option->rect;
            const auto check_bb = proxy()->subElementRect(SE_ItemViewItemCheckIndicator, opt, widget);
            const auto icon_bb  = proxy()->subElementRect(SE_ItemViewItemDecoration, opt, widget);
            const auto text_bb  = proxy()->subElementRect(SE_ItemViewItemText, opt, widget);

            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

            // check state
            if (check_bb.isValid()) {
                painter->save();
                painter->setPen(ElaThemeColor(theme, WindowText));
                if (opt->checkState == Qt::Checked) {
                    painter->setPen(QPen(ElaThemeColor(theme, CheckBoxCheckedBorder), 1.2));
                    painter->setBrush(ElaThemeColor(theme, CheckBoxCheckedBase));
                    painter->drawRoundedRect(check_bb, 2, 2);
                    QFont iconFont = QFont("ElaAwesome");
                    iconFont.setPixelSize(check_bb.width() * 0.85);
                    painter->setFont(iconFont);
                    painter->setPen(ElaThemeColor(ElaThemeType::Dark, WindowText));
                    painter->drawText(check_bb, Qt::AlignCenter, QChar((unsigned short)ElaIconType::Check));
                } else if (opt->checkState == Qt::PartiallyChecked) {
                    painter->setPen(QPen(ElaThemeColor(theme, CheckBoxCheckedBorder), 1.2));
                    painter->setBrush(ElaThemeColor(theme, CheckBoxCheckedBase));
                    painter->drawRoundedRect(check_bb, 2, 2);
                    painter->setPen(ElaThemeColor(ElaThemeType::Dark, WindowText));
                    QLine checkLine(check_bb.x() + 3, check_bb.center().y(), check_bb.right() - 3, check_bb.center().y());
                    painter->drawLine(checkLine);
                } else {
                    painter->setPen(QPen(ElaThemeColor(theme, WindowText), 1.2));
                    painter->setBrush(Qt::transparent);
                    painter->drawRoundedRect(check_bb, 2, 2);
                }
                painter->restore();
            }

            // text
            if (!opt->text.isEmpty()) {
                painter->setPen(ElaThemeColor(theme, WindowText));
                painter->drawText(text_bb, opt->displayAlignment, opt->text);
            }

            // selected mark
            int heightOffset = item_bb.height() / 4;
            painter->setPen(Qt::NoPen);
            painter->setBrush(ElaThemeColor(theme, NavigationMark));
            if (opt->state.testFlag(QStyle::State_Selected)
                && (opt->viewItemPosition == QStyleOptionViewItem::Beginning
                    || opt->viewItemPosition == QStyleOptionViewItem::OnlyOne)) {
                painter->drawRoundedRect(
                    QRectF(item_bb.x() + 3, item_bb.y() + heightOffset, 3, item_bb.height() - 2 * heightOffset), 3, 3);
            }
            painter->restore();
        } break;
        default: {
            QProxyStyle::drawControl(element, option, painter, widget);
        } break;
    }
}

QSize TaskViewStyle::sizeFromContents(
    ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const {
    switch (type) {
        case QStyle::CT_ItemViewItem: {
            QSize size = QProxyStyle::sizeFromContents(type, option, size, widget);
            size.setHeight(item_height);
            return size;
        } break;
        default: {
        } break;
    }
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

QRect TaskViewStyle::subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const {
    switch (element) {
        case QStyle::SE_ItemViewItemCheckIndicator: {
            const auto vopt = qstyleoption_cast<const QStyleOptionViewItem*>(option);
            Q_ASSERT(vopt);
            if (vopt->viewItemPosition != QStyleOptionViewItem::Middle && vopt->viewItemPosition != QStyleOptionViewItem::End) {
                QRect indicator_bb = QProxyStyle::subElementRect(element, option, widget);
                indicator_bb.adjust(left_padding, 0, left_padding, 0);
                return indicator_bb;
            }
        } break;
        case QStyle::SE_ItemViewItemText: {
            const auto vopt = qstyleoption_cast<const QStyleOptionViewItem*>(option);
            Q_ASSERT(vopt);
            if (vopt->viewItemPosition != QStyleOptionViewItem::Middle && vopt->viewItemPosition != QStyleOptionViewItem::End) {
                QRect text_bb = QProxyStyle::subElementRect(element, option, widget);
                text_bb.adjust(left_padding + 10, 0, 0, 0);
                return text_bb;
            }
        } break;
        default: {
        } break;
    }
    return QProxyStyle::subElementRect(element, option, widget);
}

} // namespace Experimental

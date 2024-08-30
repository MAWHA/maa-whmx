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

#include "NavItemDelegate.h"
#include "NavModel.h"
#include "NavNode.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <ElaTheme.h>

namespace UI {

void NavItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    const auto model = static_cast<const NavModel*>(index.model());
    const auto node  = index.data(Qt::UserRole).value<NavNode*>();

    const auto theme  = ElaThemeType::Light;
    const int  vspace = 2;

    const auto item_bb    = option.rect.adjusted(0, vspace, 0, -vspace);
    const int  item_width = item_bb.width();

    const int icon_pixel_size = 20;
    const int text_pixel_size = 10;

    auto icon_bb = item_bb;
    icon_bb.setHeight(item_width);

    auto text_bb = icon_bb;
    text_bb.translate(0, icon_bb.height() + vspace);
    text_bb.setHeight(text_pixel_size);

    const bool has_icon = node->font_icon() != ElaIconType::None;
    const bool has_text = !node->text().isEmpty();

    if (option.state.testFlag(QStyle::State_HasFocus)) { opt.state &= ~QStyle::State_HasFocus; }

    QStyledItemDelegate::paint(painter, opt, index);

    painter->save();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    //! draw background
    {
        QPainterPath path;
        path.addRoundedRect(item_bb, 8, 8);
        if (node == model->selected_node() && (option.state & QStyle::State_MouseOver)) {
            painter->fillPath(path, ElaThemeColor(theme, NavigationSelectedHover));
        } else if (node == model->selected_node() || (option.state & QStyle::State_MouseOver)) {
            painter->fillPath(path, ElaThemeColor(theme, NavigationHover));
        }
    }

    painter->setPen(node->foreground_color());

    int flags = Qt::AlignCenter;

    //! draw icon
    {
        painter->save();
        auto font = QFont("ElaAwesome");
        font.setPixelSize(icon_pixel_size);
        painter->setFont(font);

        const QChar text_icon(has_icon ? node->font_icon() : ElaIconType::IconName::ObjectsColumn);

        if (has_text) {
            QRect icon_br;
            painter->drawText(item_bb, flags | Qt::TextDontPrint, text_icon, &icon_br);
            const int total_height = icon_br.height() + text_pixel_size + vspace;
            const int margin       = (item_bb.height() - total_height) / 2;
            icon_bb.setTop(item_bb.top() + margin);
            text_bb.setTop(icon_bb.top() + icon_br.height() + vspace);
            flags = Qt::AlignHCenter | Qt::AlignTop;
        }

        painter->drawText(icon_bb, flags, text_icon);

        painter->restore();
    }

    //! draw text
    if (has_text) {
        painter->save();
        auto font = painter->font();
        font.setPixelSize(text_pixel_size);
        painter->setFont(font);
        painter->drawText(text_bb, flags, node->text());
        painter->restore();
    }

    //! draw mark
    if (node == model->selected_node()) {
        const int margin = 10;
        painter->setPen(Qt::NoPen);
        painter->setBrush(ElaThemeColor(theme, NavigationMark));
        auto mark_bb = item_bb;

        //! TODO: better solution
        if (true || !has_text) {
            mark_bb.adjust(3, margin, 0, -margin);
            mark_bb.setWidth(3);
            painter->drawRoundedRect(mark_bb, 3, 3);
        } else {
            mark_bb.adjust(margin, item_bb.height() - 6, -margin, 0);
            mark_bb.setHeight(3);
            painter->drawRoundedRect(mark_bb, 3, 3);
        }
    }

    painter->restore();
}

QSize NavItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    const auto node     = index.data(Qt::UserRole).value<NavNode*>();
    const bool has_text = !node->text().isEmpty();
    const bool has_icon = node->font_icon() != ElaIconType::None;

    const int spacing     = 2;
    const int text_height = 10;

    QSize size(40, 40);
    if (has_text) { size.setHeight(size.height() + text_height + spacing); }

    return size + QSize(0, spacing * 2);
}

} // namespace UI

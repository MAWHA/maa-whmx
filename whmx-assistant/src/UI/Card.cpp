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

#include "Card.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QTextLayout>
#include <ElaTheme.h>

namespace UI {

Card::Card(QWidget* parent)
    : QWidget(parent) {
    set_hover_offset(0);
    set_hover_opacity(0);
    set_border_radius(8);
    set_pin_color(QColor(42, 109, 197));

    setContextMenuPolicy(Qt::CustomContextMenu);
    setMouseTracking(true);
    setFixedSize(320, 120);

    connect(this, &Card::on_border_radius_change, this, QOverload<>::of(&QWidget::update));
    connect(this, &Card::on_pin_color_change, this, QOverload<>::of(&QWidget::update));
    connect(this, &Card::on_card_pixmap_change, this, QOverload<>::of(&QWidget::update));
    connect(this, &Card::on_title_change, this, QOverload<>::of(&QWidget::update));
    connect(this, &Card::on_brief_change, this, QOverload<>::of(&QWidget::update));
}

bool Card::event(QEvent* event) {
    switch (event->type()) {
        case QEvent::Enter: {
            auto anim_hover = new QPropertyAnimation(this, "hover_offset");
            connect(anim_hover, &QPropertyAnimation::valueChanged, this, QOverload<>::of(&QWidget::update));
            anim_hover->setDuration(130);
            anim_hover->setStartValue(hover_offset());
            anim_hover->setEndValue(6);
            anim_hover->start(QAbstractAnimation::DeleteWhenStopped);
            auto anim_opacity = new QPropertyAnimation(this, "hover_opacity");
            anim_opacity->setDuration(130);
            anim_opacity->setStartValue(hover_opacity());
            anim_opacity->setEndValue(1);
            anim_opacity->start(QAbstractAnimation::DeleteWhenStopped);
        } break;
        case QEvent::Leave: {
            auto anim_hover = new QPropertyAnimation(this, "hover_offset");
            connect(anim_hover, &QPropertyAnimation::valueChanged, this, QOverload<>::of(&QWidget::update));
            anim_hover->setDuration(130);
            anim_hover->setStartValue(hover_offset());
            anim_hover->setEndValue(0);
            anim_hover->start(QAbstractAnimation::DeleteWhenStopped);
            auto anim_opacity = new QPropertyAnimation(this, "hover_opacity");
            anim_opacity->setDuration(130);
            anim_opacity->setStartValue(hover_opacity());
            anim_opacity->setEndValue(0);
            anim_opacity->start(QAbstractAnimation::DeleteWhenStopped);
        } break;
        case QEvent::MouseButtonRelease: {
            if (static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton) { emit clicked(); }
        } break;
        default: {
        } break;
    }
    return QWidget::event(event);
}

void Card::paintEvent(QPaintEvent* event) {
    QPainter p(this);
    p.save();
    p.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing | QPainter::TextAntialiasing);

    const int  shadow_border_width = 6;
    const int  text_h_space        = 20;
    const int  text_v_space        = 5;
    const auto theme_mode          = ElaThemeType::Light;

    //! draw hover shadow
    if (underMouse()) {
        p.setOpacity(hover_opacity());
        const auto shadow_bb = rect().adjusted(0, 0, 0, -hover_offset());
        eTheme->drawEffectShadow(&p, shadow_bb, shadow_border_width, border_radius());
    }

    //! draw background
    const QRectF fg_bb(
        shadow_border_width,
        shadow_border_width - hover_offset() + 1,
        width() - 2 * shadow_border_width,
        height() - 2 * shadow_border_width);

    p.setOpacity(1.0);
    p.setPen(underMouse() ? ElaThemeColor(theme_mode, PopularCardFloaterBorder) : ElaThemeColor(theme_mode, PopularCardBorder));
    p.setBrush(ElaThemeColor(theme_mode, PopularCardBase));
    p.drawRoundedRect(fg_bb, border_radius(), border_radius());

    //! draw card image
    const double margin = fg_bb.height() * 0.15;

    QRectF     pix_bb(fg_bb.x() + margin, fg_bb.y() + margin, fg_bb.height() * 0.7, fg_bb.height() * 0.7);
    const auto card_pix = card_pixmap();
    if (!card_pix.isNull()) {
        p.save();
        QPainterPath path;
        path.addRoundedRect(pix_bb, 4, 4);
        p.setClipPath(path);
        if (const double dpr = devicePixelRatioF(); qAbs(dpr - 1.0) < 1e-6) {
            p.drawPixmap(pix_bb, card_pix, card_pix.rect());
        } else {
            const auto aspect = Qt::KeepAspectRatio;
            const auto mode   = Qt::SmoothTransformation;
            const auto pix    = card_pix.scaled(pix_bb.width() * dpr, pix_bb.height() * dpr, aspect, mode);
            p.drawPixmap(pix_bb.toRect(), pix);
        }
        p.restore();
    }

    //! draw title
    const int text_bb_x_start = card_pix.isNull() ? pix_bb.left() : pix_bb.right() + text_h_space;
    const int text_bb_width   = fg_bb.right() - text_bb_x_start - margin;

    p.setPen(ElaThemeColor(theme_mode, WindowText));
    {
        QFont font = p.font();
        font.setWeight(QFont::Bold);
        font.setPixelSize(15);
        p.setFont(font);
    }
    const int    title_height = p.fontMetrics().height();
    const QRectF title_bb(text_bb_x_start, pix_bb.y(), text_bb_width, title_height);
    {
        const QString title_text = p.fontMetrics().elidedText(title(), Qt::ElideRight, title_bb.width());
        p.drawText(title_bb, Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, title_text);
    }

    //! draw brief
    {
        QFont font = p.font();
        font.setWeight(QFont::DemiBold);
        font.setPixelSize(13);
        p.setFont(font);
    }
    const int brief_height = p.fontMetrics().height();
    QRectF    brief_bb(text_bb_x_start, title_bb.bottom() + text_v_space, text_bb_width, brief_height);
    {
        const auto  fm               = p.fontMetrics();
        const auto  brief_text       = brief();
        const int   max_brief_bottom = pix_bb.bottom();
        const int   spacing          = fm.lineSpacing();
        QTextLayout text_layout(brief_text, p.font());
        {
            auto opt = text_layout.textOption();
            opt.setWrapMode(QTextOption::WrapAnywhere);
            text_layout.setTextOption(opt);
        }
        text_layout.beginLayout();
        while (true) {
            auto line = text_layout.createLine();
            if (!line.isValid()) { break; }
            line.setLineWidth(brief_bb.width());
            if (brief_bb.bottom() + spacing > max_brief_bottom) {
                const auto lastline    = brief_text.mid(line.textStart());
                const auto elided_text = fm.elidedText(lastline, Qt::ElideRight, brief_bb.width());
                p.drawText(brief_bb.topLeft() + QPoint(0, fm.ascent()), elided_text);
                break;
            }
            line.draw(&p, brief_bb.topLeft());
            brief_bb.translate(0, spacing);
        }
        text_layout.endLayout();
    }

    //! draw pin
    const double pin_radius = 5.0;
    const double pin_width  = pin_radius * 2;
    const double pin_margin = margin * 0.5;
    const QRectF pin_bb(fg_bb.right() - pin_margin - pin_width, fg_bb.top() + pin_margin, pin_width, pin_width);
    p.setPen(pin_color());
    p.setBrush(pin_color());
    p.drawEllipse(pin_bb);

    p.restore();
}

} // namespace UI

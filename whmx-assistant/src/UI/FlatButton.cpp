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

#include "FlatButton.h"

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

namespace UI {

void FlatButton::set_border_visible(bool visble) {
    border_visible_ = visble;
    update();
}

void FlatButton::set_text(const QString &text) {
    text_ = text;
    update();
}

void FlatButton::set_radius(int radius) {
    ui_radius_ = radius;
    update();
}

void FlatButton::set_text_alignment(Qt::Alignment alignment) {
    ui_alignment_ = alignment;
    update();
}

FlatButton::FlatButton(QWidget *parent)
    : QWidget(parent)
    , border_visible_{false}
    , ui_radius_{0}
    , ui_alignment_{Qt::AlignLeft}
    , ui_mouse_entered_{false} {
    setupUi();
}

QSize FlatButton::minimumSizeHint() const {
    const auto margins      = contentsMargins();
    const int  mw           = margins.left() + margins.right();
    const int  mh           = margins.top() + margins.bottom();
    const auto fm           = fontMetrics();
    const int  inner_width  = text_.isEmpty() ? fm.averageCharWidth() : fm.horizontalAdvance(text_);
    const int  inner_height = fm.height() + fm.descent() + 8;
    return QSize(inner_width + mw, inner_height + mh);
}

QSize FlatButton::sizeHint() const {
    return minimumSize();
}

void FlatButton::setupUi() {
    setContentsMargins(10, 0, 10, 0);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    set_text_alignment(Qt::AlignCenter);
    set_radius(6);
}

void FlatButton::paintEvent(QPaintEvent *e) {
    QPainter   p(this);
    const auto pal = palette();

    const auto brush = !ui_mouse_entered_ ? pal.base() : pal.button();

    p.save();
    p.setBrush(brush);
    auto rect = this->rect();
    if (border_visible_) {
        rect.adjust(1, 1, -1, -1);
        p.setPen(QPen(pal.color(QPalette::Window), 0.5));
    } else {
        p.setPen(Qt::transparent);
    }
    p.drawRoundedRect(rect, ui_radius_, ui_radius_);
    p.restore();

    const auto bb    = contentsRect();
    const auto flags = ui_alignment_ | Qt::AlignVCenter;
    p.drawText(bb, flags, text_);
}

void FlatButton::enterEvent(QEnterEvent *event) {
    setCursor(Qt::PointingHandCursor);
    ui_mouse_entered_ = true;
    update();
}

void FlatButton::leaveEvent(QEvent *event) {
    setCursor(Qt::ArrowCursor);
    ui_mouse_entered_ = false;
    update();
}

void FlatButton::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) { emit pressed(); }
}

} // namespace UI

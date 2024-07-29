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

#include "IconButton.h"

#include <QToolTip>
#include <QPainter>

namespace UI {

int IconButton::get_icon_size() const {
    return icon_size_;
}

void IconButton::set_icon_size(int size) {
    Q_ASSERT(size > 0);
    icon_size_ = size;
    adjustSize();
    reload_icon();
}

void IconButton::set_icon(const QString &default_icon, const QString &hover_icon) {
    Q_ASSERT(!default_icon.isEmpty());
    default_icon_name_ = default_icon;
    hover_icon_name_   = hover_icon;
    reload_icon();
}

void IconButton::reload_icon() {
    const auto icon_path = ui_hover_ && !hover_icon_name_.isEmpty() ? hover_icon_name_ : default_icon_name_;
    QIcon      icon(icon_path);
    Q_ASSERT(!icon.isNull());
    ui_icon_ = icon.pixmap(icon_size_, icon_size_);
    update();
}

void IconButton::set_action(std::unique_ptr<QAction> action) {
    if (action_ == action.get()) { return; }
    reset_action();
    if (action) { connect(this, &IconButton::clicked, action.get(), &QAction::trigger); }
    action_ = action.release();
}

void IconButton::reset_action() {
    if (!action_) { return; }
    disconnect(this, &IconButton::clicked, action_, nullptr);
    if (auto parent = action_->parent(); !parent || parent == this) { action_->deleteLater(); }
    action_ = nullptr;
}

void IconButton::set_tool_tip(const QString &tip) {
    setToolTip(tip);
}

void IconButton::trigger() {
    if (action_) { action_->trigger(); }
}

IconButton::IconButton(const QString &default_icon, const QString &hover_icon, QWidget *parent)
    : ClickableLabel(parent)
    , action_{nullptr}
    , icon_size_{12}
    , ui_hover_{false} {
    set_icon(default_icon, hover_icon);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

IconButton::~IconButton() {
    reset_action();
}

QSize IconButton::minimumSizeHint() const {
    const int padding = 4;
    return QSize(icon_size_ + padding * 2, icon_size_ + padding * 2);
}

QSize IconButton::sizeHint() const {
    return minimumSizeHint();
}

void IconButton::enterEvent(QEnterEvent *event) {
    ClickableLabel::enterEvent(event);
    ui_hover_ = true;
    reload_icon();
}

void IconButton::leaveEvent(QEvent *event) {
    ClickableLabel::leaveEvent(event);
    ui_hover_ = false;
    reload_icon();
}

void IconButton::paintEvent(QPaintEvent *event) {
    if (ui_icon_.isNull()) { return; }

    QPainter    p(this);
    const auto &pal = palette();

    const auto bb     = contentsRect();
    const int  extent = get_icon_size();
    const auto pos    = QPointF(bb.width() - extent, bb.height() - extent) / 2;
    p.drawPixmap(pos, ui_icon_);
}

} // namespace UI

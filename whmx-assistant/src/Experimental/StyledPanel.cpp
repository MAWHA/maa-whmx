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

#include "StyledPanel.h"

#include <QtWidgets/QHBoxLayout>
#include <QtGui/QPainter>
#include <ElaTheme.h>

namespace Experimental {

void StyledPanel::set_shadow_width(int width) {
    shadow_width_ = width;
    setContentsMargins(shadow_width_, shadow_width_, shadow_width_, shadow_width_);
}

StyledPanel::StyledPanel(gsl::not_null<gsl::owner<QWidget *>> widget)
    : widget_(widget) {
    set_shadow_width(6);
    auto ui_layout = new QHBoxLayout(this);
    ui_layout->setContentsMargins(8, 8, 8, 8);
    ui_layout->addWidget(widget_);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void StyledPanel::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing);

    eTheme->drawEffectShadow(&p, rect(), shadow_width_, shadow_width_);

    const auto bg_color = widget_->palette().color(widget_->backgroundRole());
    p.setPen(bg_color);
    p.setBrush(bg_color);
    p.drawRoundedRect(contentsRect(), 8, 8);
}

} // namespace Experimental

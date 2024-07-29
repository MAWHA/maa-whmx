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

#include "ElidedLabel.h"

#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>

namespace UI {

void ElidedLabel::set_elide_mode(Qt::TextElideMode elide_mode) {
    elide_mode_ = elide_mode;
    cached_text_.clear();
    update();
}

void ElidedLabel::refresh_elided_text() {
    const auto raw_text = text();
    if (cached_text_ == raw_text) { return; }
    cached_text_        = raw_text;
    const auto fm       = fontMetrics();
    cached_elided_text_ = fm.elidedText(cached_text_, elide_mode_, width(), Qt::TextSingleLine);
    if (!cached_text_.isEmpty()) {
        const auto min_text = cached_text_.at(0) + QString("...");
        setMinimumWidth(fm.horizontalAdvance(min_text) + 1);
    }
}

ElidedLabel::ElidedLabel(QWidget *parent, Qt::WindowFlags f)
    : ElidedLabel("", parent, f) {}

ElidedLabel::ElidedLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(text, parent, f) {
    set_elide_mode(Qt::ElideRight);
}

void ElidedLabel::paintEvent(QPaintEvent *event) {
    if (elide_mode_ == Qt::ElideNone) { return QLabel::paintEvent(event); }

    refresh_elided_text();
    setText(cached_elided_text_);
    QLabel::paintEvent(event);
    setText(cached_text_);
}

void ElidedLabel::resizeEvent(QResizeEvent *event) {
    QLabel::resizeEvent(event);
    cached_text_.clear();
}

} // namespace UI

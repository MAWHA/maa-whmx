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

#include "LogPanel.h"
#include "Scrollbar.h"

#include <QtWidgets/QVBoxLayout>
#include <qtmaterialscrollbar.h>
#include <shared_mutex>

namespace UI {

std::shared_mutex GLOBAL_LOG_PANELS_LOCK;
QList<LogPanel *> GLOBAL_LOG_PANELS;

QList<LogPanel *> LogPanel::global_logger_panels() {
    std::shared_lock lock(GLOBAL_LOG_PANELS_LOCK);
    const auto       resp = GLOBAL_LOG_PANELS;
    return std::move(resp);
}

void LogPanel::attach_to_global_logger() {
    std::unique_lock lock(GLOBAL_LOG_PANELS_LOCK);
    if (!GLOBAL_LOG_PANELS.contains(this)) { GLOBAL_LOG_PANELS.append(this); }
}

void LogPanel::detach_from_global_logger() const {
    std::unique_lock lock(GLOBAL_LOG_PANELS_LOCK);
    if (GLOBAL_LOG_PANELS.contains(this)) { GLOBAL_LOG_PANELS.remove(GLOBAL_LOG_PANELS.indexOf(this)); }
}

void LogPanel::log(QString message) {
    std::lock_guard lock(log_mutex_);

    auto      vert_scrollbar   = log_text_container_->verticalScrollBar();
    auto      hori_scrollbar   = log_text_container_->horizontalScrollBar();
    const int last_scroll_vpos = vert_scrollbar->sliderPosition();
    const int last_scroll_hpos = hori_scrollbar->sliderPosition();
    bool      should_scroll    = last_scroll_vpos == vert_scrollbar->maximum();

    if (log_text_container_->toPlainText().length() > max_log_len_) {
        flush();
        should_scroll = true;
    }

    log_text_container_->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    log_text_container_->insertPlainText(message);

    if (should_scroll) {
        vert_scrollbar->setSliderPosition(vert_scrollbar->maximum());
    } else {
        vert_scrollbar->setSliderPosition(last_scroll_vpos);
        hori_scrollbar->setSliderPosition(last_scroll_hpos);
    }
}

void LogPanel::clear() {
    log_text_container_->clear();
}

QString LogPanel::take() {
    auto loggings = log_text_container_->toPlainText();
    clear();
    return loggings;
}

void LogPanel::flush() {
    emit on_flush(take());
}

void LogPanel::set_max_len(int max_len) {
    max_log_len_ = max_len;
}

LogPanel::LogPanel(QWidget *parent)
    : QWidget(parent)
    , max_log_len_(10000) {
    setup();
}

LogPanel::~LogPanel() {
    detach_from_global_logger();
}

void LogPanel::setup() {
    log_text_container_ = new QPlainTextEdit;
    log_text_container_->setReadOnly(true);
    log_text_container_->setLineWrapMode(QPlainTextEdit::NoWrap);
    log_text_container_->setHorizontalScrollBar(new Scrollbar);
    log_text_container_->setVerticalScrollBar(new Scrollbar);
    log_text_container_->setFrameShadow(QFrame::Raised);
    log_text_container_->setFrameShape(QFrame::Box);
    log_text_container_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins({});
    layout->addWidget(log_text_container_);
}

} // namespace UI

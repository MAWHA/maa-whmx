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

#include "QueuedTaskItem.h"

#include <QtWidgets/QHBoxLayout>

namespace UI {

QueuedTaskItem::QueuedTaskItem(const QString& name, QWidget* parent)
    : QWidget(parent)
    , task_type_(GeneralTask)
    , task_name_(name) {
    setup();
}

QueuedTaskItem::QueuedTaskItem(Task::MajorTask task, QWidget* parent)
    : QWidget(parent)
    , task_type_(MajorTask)
    , major_task_(task)
    , task_name_(Task::get_task_info(task).name) {
    setup();
}

QSize QueuedTaskItem::minimumSizeHint() const {
    const auto label_hint  = ui_task_name_->minimumSizeHint();
    const auto button_hint = ui_drop_->minimumSizeHint();
    return QSize(0, qMax(label_hint.height(), button_hint.height()) + 4);
}

QSize QueuedTaskItem::sizeHint() const {
    return minimumSizeHint();
}

void QueuedTaskItem::setup() {
    ui_drop_           = new IconButton(":/icons/close-circle.png", ":/icons/close-circle-hover.png");
    ui_task_name_      = new ElidedLabel;
    ui_move_up_        = new IconButton(":/icons/arrow-up.png", ":/icons/arrow-up-hover.png");
    ui_move_down_      = new IconButton(":/icons/arrow-down.png", ":/icons/arrow-down-hover.png");
    ui_move_to_top_    = new IconButton(":/icons/skip-up.png", ":/icons/skip-up-hover.png");
    ui_move_to_bottom_ = new IconButton(":/icons/skip-down.png", ":/icons/skip-down-hover.png");

    ui_task_name_->setText(task_name_);
    ui_drop_->set_icon_size(16);
    ui_move_up_->set_icon_size(16);
    ui_move_down_->set_icon_size(16);
    ui_move_to_top_->set_icon_size(16);
    ui_move_to_bottom_->set_icon_size(16);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins({});
    layout->setSpacing(4);
    layout->addWidget(ui_drop_);
    layout->addWidget(ui_task_name_);
    layout->addStretch();
    layout->addWidget(ui_move_up_);
    layout->addWidget(ui_move_down_);
    layout->addWidget(ui_move_to_top_);
    layout->addWidget(ui_move_to_bottom_);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    const auto on_request_action = std::bind(&QueuedTaskItem::on_request_action, this, std::placeholders::_1);
    connect(ui_drop_, &IconButton::clicked, this, std::bind(on_request_action, ActionType::Drop));
    connect(ui_move_up_, &IconButton::clicked, this, std::bind(on_request_action, ActionType::MoveUp));
    connect(ui_move_down_, &IconButton::clicked, this, std::bind(on_request_action, ActionType::MoveDown));
    connect(ui_move_to_top_, &IconButton::clicked, this, std::bind(on_request_action, ActionType::MoveToTop));
    connect(ui_move_to_bottom_, &IconButton::clicked, this, std::bind(on_request_action, ActionType::MoveToBottom));
}

} // namespace UI

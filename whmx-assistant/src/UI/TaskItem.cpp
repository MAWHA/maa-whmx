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

#include "TaskItem.h"

#include <QtWidgets/QHBoxLayout>
#include <QtGui/QMouseEvent>

namespace UI {

TaskItem::TaskItem(Task::MajorTask task, QWidget *parent)
    : QWidget(parent)
    , task_(task) {
    setup();
}

QSize TaskItem::minimumSizeHint() const {
    return QSize(0, 32);
}

QSize TaskItem::sizeHint() const {
    return minimumSizeHint();
}

void TaskItem::setup() {
    const auto task_info = Task::get_task_info(task_);

    ui_select_    = new QCheckBox;
    ui_task_name_ = new ElidedLabel;

    if (task_info.routine) { ui_select_->setCheckState(Qt::Checked); }
    ui_task_name_->setText(task_info.name);

    if (task_info.has_config) {
        ui_config_ = new IconButton(":/icons/settings.png", ":/icons/settings-hover.png");
        ui_config_->set_icon_size(16);
    }

    auto layout = new QHBoxLayout(this);
    layout->addWidget(ui_select_);
    layout->addWidget(ui_task_name_);
    layout->addStretch();
    if (ui_config_) { layout->addWidget(ui_config_); }

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    ui_task_name_->installEventFilter(this);

    connect(ui_config_, &IconButton::clicked, this, &TaskItem::post_config_request);
}

bool TaskItem::eventFilter(QObject *watched, QEvent *event) {
    Q_ASSERT(watched == ui_task_name_);
    if (event->type() == QEvent::MouseButtonRelease) {
        if (const auto e = static_cast<QMouseEvent *>(event);
            e->button() == Qt::LeftButton && contentsRect().contains(mapFromGlobal(e->globalPosition().toPoint()))) {
            ui_select_->toggle();
        }
    }
    return false;
}

void TaskItem::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && contentsRect().contains(mapFromGlobal(event->globalPosition().toPoint()))) {
        ui_select_->toggle();
    }
}

} // namespace UI

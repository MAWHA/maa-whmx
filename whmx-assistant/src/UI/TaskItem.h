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

#pragma once

#include "IconButton.h"
#include "ElidedLabel.h"
#include "../Task/MajorTask.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>

namespace UI {

class TaskItem : public QWidget {
    Q_OBJECT

public:
    Task::MajorTask task() const {
        return task_;
    }

    bool has_config() const {
        return Task::get_task_info(task_).has_config;
    }

    bool checked() const {
        return ui_select_->isChecked();
    }

    void set_checked(bool on) {
        ui_select_->setChecked(on);
    }

protected slots:

    void post_config_request() {
        emit on_request_config(task_);
    }

signals:
    void on_request_config(Task::MajorTask task);

public:
    TaskItem(Task::MajorTask task, QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void setup();

    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    const Task::MajorTask task_;
    QCheckBox            *ui_select_    = nullptr;
    ElidedLabel          *ui_task_name_ = nullptr;
    IconButton           *ui_config_    = nullptr;
};

} // namespace UI

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

#include <optional>

namespace UI {

class QueuedTaskItem : public QWidget {
    Q_OBJECT

public:
    enum TaskType {
        MajorTask,
        GeneralTask,
    };

    enum ActionType {
        Drop,
        MoveUp,
        MoveDown,
        MoveToTop,
        MoveToBottom,
    };

public:
    TaskType task_type() const {
        return task_type_;
    }

    QString task_name() const {
        return task_name_;
    }

    std::optional<Task::MajorTask> major_task() const {
        return major_task_;
    }

signals:
    void on_request_action(ActionType action);

public:
    QueuedTaskItem(const QString& name, QWidget* parent = nullptr);
    QueuedTaskItem(Task::MajorTask task, QWidget* parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void setup();

private:
    const TaskType                 task_type_;
    std::optional<Task::MajorTask> major_task_;
    QString                        task_name_;
    IconButton*                    ui_drop_           = nullptr;
    ElidedLabel*                   ui_task_name_      = nullptr;
    IconButton*                    ui_move_up_        = nullptr;
    IconButton*                    ui_move_down_      = nullptr;
    IconButton*                    ui_move_to_top_    = nullptr;
    IconButton*                    ui_move_to_bottom_ = nullptr;
};

} // namespace UI

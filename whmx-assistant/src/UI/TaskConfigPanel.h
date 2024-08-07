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

#include "../Task/MajorTask.h"

#include <QtWidgets/QVBoxLayout>
#include <QtCore/QVariant>
#include <QtWidgets/QWidget>

namespace UI {

class TaskConfigPanel : public QWidget {
    Q_OBJECT

public:
    static TaskConfigPanel* build(Task::MajorTask task, QVariant config = QVariant());

public:
    void append_config_item(const QString& name, const QString& desc, QWidget* widget);
    void put_notice(const QString& notice);

    template <typename ConfigType>
    ConfigType& config() const {
        return *reinterpret_cast<ConfigType*>(const_cast<TaskConfigPanel*>(this)->config_data_.data());
    }

public slots:
    void notify_config_changed();

signals:
    void on_config_change(Task::MajorTask task, QVariant config);

protected:
    TaskConfigPanel(Task::MajorTask task, QVariant config = QVariant());

    void setup();

private:
    const Task::MajorTask task_;
    QVariant              config_data_;
    QVBoxLayout*          ui_layout_ = nullptr;
};

} // namespace UI

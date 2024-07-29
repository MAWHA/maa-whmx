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

#include "Config.h"

#include <QtCore/QFile>
#include <QtCore/QMap>
#include <meojson/json.hpp>
#include <magic_enum.hpp>

namespace Task {

bool load_task_config(Config& config, const QString& file_path) {
    //! TODO: parse task_params

    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly)) { return false; }

    const auto data = json::parse(QString::fromUtf8(file.readAll()).toStdString());
    if (!data.has_value()) { return false; }

    if (!data->contains("bindings")) { return false; }

    QMap<QString, MajorTask> rev_index_table;
    for (const auto& major_task : magic_enum::enum_values<MajorTask>()) {
        rev_index_table.insert(QString::fromLatin1(magic_enum::enum_name(major_task)), major_task);
    }

    const auto bindings = data->at("bindings");
    for (const auto& binding : bindings.as_array()) {
        if (!binding.contains("major_task")) { continue; }
        if (!binding.contains("entry")) { continue; }
        const auto major_task_name = QString::fromUtf8(binding.at("major_task").as_string());
        const auto task_entry      = QString::fromUtf8(binding.at("entry").as_string());
        if (task_entry.isEmpty()) { continue; }
        if (!rev_index_table.contains(major_task_name)) { continue; }
        const auto major_task           = rev_index_table.value(major_task_name);
        config.task_entries[major_task] = task_entry;
    }

    return true;
}

} // namespace Task

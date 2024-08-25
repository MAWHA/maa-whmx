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

#include "PropertyContext.h"

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QList>
#include <meojson/json.hpp>
#include <memory>

struct TaskInterface : public std::enable_shared_from_this<TaskInterface> {
    QMap<QString, QString>           tasks;
    std::shared_ptr<PropertyContext> prop_context;

    static std::shared_ptr<TaskInterface> load(const char* utf8_path);
    static std::shared_ptr<TaskInterface> load(const QString& path);
    static std::shared_ptr<TaskInterface> load(const json::value& interface);

    json::object dump_props() const;
    bool         load_props(const json::object& props);

    QStringList task_entries() const {
        return tasks.keys();
    }

    QStringList task_properties(const QString& task_name) {
        return prop_context->properties(task_name);
    }

    std::optional<std::reference_wrapper<Property>> property(const QString& task_name, const QString& prop_name) {
        return prop_context->property(task_name, prop_name);
    }
};

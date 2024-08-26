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

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <meojson/json.hpp>
#include <optional>

namespace Experimental {

QVariant make_variant_from_json_value(const json::value &value);

struct TaskRouteCondition {
    QString  key;
    QString  op;
    QVariant operand;

    static TaskRouteCondition parse(const json::object &data);
};

struct TaskRouteTaskInfo {
    std::optional<QString>            action;
    std::optional<QStringList>        action_args;
    std::optional<QString>            task_entry;
    std::optional<json::object>       override_task_params;
    std::optional<TaskRouteCondition> trigger_condition;
    bool                              fold_params;

    static TaskRouteTaskInfo parse(const json::object &data);
};

struct TaskRouteTaskUnit {
    std::optional<TaskRouteTaskInfo>        direct_entry;
    std::optional<QList<TaskRouteTaskInfo>> exclusive_task_group;

    static TaskRouteTaskUnit parse(const json::value &data);
};

struct TaskRoutePipelineUnit {
    std::optional<TaskRouteCondition> trigger_condition;
    QList<TaskRouteTaskUnit>          tasks;

    static TaskRoutePipelineUnit parse(const json::object &data);
};

struct TaskRoute {
    std::optional<QString>                      bound_task;
    std::optional<QList<TaskRoutePipelineUnit>> bound_pipeline;

    static TaskRoute parse(const json::value &data);
};

} // namespace Experimental

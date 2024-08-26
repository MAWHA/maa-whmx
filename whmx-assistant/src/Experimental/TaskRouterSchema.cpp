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

#include "TaskRouterSchema.h"

namespace Experimental {

QVariant make_variant_from_json_value(const json::value &value) {
    if (false) {
    } else if (value.is_null()) {
        return QVariant();
    } else if (value.is_boolean()) {
        return QVariant(value.as_boolean());
    } else if (value.is_number()) {
        const int    intval = value.as_integer();
        const double fltval = value.as_double();
        if (static_cast<double>(intval) == fltval) {
            return QVariant(intval);
        } else {
            return QVariant(fltval);
        }
    } else if (value.is_string()) {
        return QVariant(QString::fromUtf8(value.as_string()));
    } else if (value.is_array()) {
        QVariantList list;
        for (const auto &value : value.as_array()) { list.append(make_variant_from_json_value(value)); }
        return QVariant(list);
    } else if (value.is_object()) {
        QVariantMap map;
        for (const auto &[key, value] : value.as_object()) {
            map.insert(QString::fromUtf8(key), make_variant_from_json_value(value));
        }
        return QVariant(map);
    } else {
        Q_UNREACHABLE();
    }
}

TaskRouteCondition TaskRouteCondition::parse(const json::object &data) {
    Q_ASSERT(data.contains("key") && data.at("key").is_string());
    Q_ASSERT(data.contains("op") && data.at("op").is_string());

    TaskRouteCondition condition;
    condition.key     = QString::fromUtf8(data.at("key").as_string());
    condition.op      = QString::fromUtf8(data.at("op").as_string());
    condition.operand = make_variant_from_json_value(data.get("operand", json::value()));

    return condition;
}

TaskRouteTaskInfo TaskRouteTaskInfo::parse(const json::object &data) {
    TaskRouteTaskInfo task_info;

    if (data.contains("action")) {
        const auto action = data.at("action");
        if (action.is_string()) {
            task_info.action = QString::fromUtf8(action.as_string());
        } else if (action.is_object()) {
            Q_ASSERT(action.contains("name") && action.at("name").is_string());
            Q_ASSERT(action.contains("param") && action.at("param").is_array());
            task_info.action = QString::fromUtf8(action.at("name").as_string());
            QStringList args;
            for (const auto &arg : action.at("param").as_array()) {
                Q_ASSERT(arg.is_string());
                args.append(QString::fromUtf8(arg.as_string()));
            }
            task_info.action_args = args;
        } else {
            Q_UNREACHABLE();
        }
    }

    if (data.contains("entry")) {
        Q_ASSERT(data.at("entry").is_string());
        task_info.task_entry = QString::fromUtf8(data.at("entry").as_string());
        if (data.contains("param")) {
            Q_ASSERT(data.at("param").is_object());
            task_info.override_task_params = data.at("param").as_object();
        }
    }

    if (data.contains("on")) {
        Q_ASSERT(data.at("on").is_object());
        task_info.trigger_condition = TaskRouteCondition::parse(data.at("on").as_object());
    }

    Q_ASSERT(!data.contains("fold_params") || data.at("fold_params").is_boolean());
    task_info.fold_params = data.get("fold_params", true);

    return task_info;
}

TaskRouteTaskUnit TaskRouteTaskUnit::parse(const json::value &data) {
    TaskRouteTaskUnit task_unit;

    if (data.is_object()) {
        task_unit.direct_entry = TaskRouteTaskInfo::parse(data.as_object());
    } else if (data.is_array()) {
        QList<TaskRouteTaskInfo> task_group;
        for (const auto &entry : data.as_array()) {
            Q_ASSERT(entry.is_object());
            task_group.append(TaskRouteTaskInfo::parse(entry.as_object()));
        }
        task_unit.exclusive_task_group = std::move(task_group);
    } else {
        Q_UNREACHABLE();
    }

    return task_unit;
}

TaskRoutePipelineUnit TaskRoutePipelineUnit::parse(const json::object &data) {
    TaskRoutePipelineUnit pipeline_unit;

    if (data.contains("on")) {
        Q_ASSERT(data.at("on").is_object());
        pipeline_unit.trigger_condition = TaskRouteCondition::parse(data.at("on").as_object());
    }

    if (data.contains("pipeline")) {
        Q_ASSERT(data.at("pipeline").is_array());
        for (auto &entry : data.at("pipeline").as_array()) {
            Q_ASSERT(entry.is_array() || entry.is_object());
            pipeline_unit.tasks.append(TaskRouteTaskUnit::parse(entry));
        }
    }

    return pipeline_unit;
}

TaskRoute TaskRoute::parse(const json::value &data) {
    TaskRoute task_route;

    if (data.is_string()) {
        task_route.bound_task = QString::fromUtf8(data.as_string());
    } else if (data.is_object()) {
        QList<TaskRoutePipelineUnit> pipeline_units;
        pipeline_units.append(TaskRoutePipelineUnit::parse(data.as_object()));
        task_route.bound_pipeline = std::move(pipeline_units);
    } else if (data.is_array()) {
        QList<TaskRoutePipelineUnit> pipeline_units;
        for (const auto &entry : data.as_array()) {
            Q_ASSERT(entry.is_object());
            pipeline_units.append(TaskRoutePipelineUnit::parse(entry.as_object()));
        }
        task_route.bound_pipeline = std::move(pipeline_units);
    } else {
        Q_UNREACHABLE();
    }

    return task_route;
}

} // namespace Experimental

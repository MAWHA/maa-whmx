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

#include "MajorTask.h"
#include "PropGetter.h"
#include "TaskGraph.h"
#include "Config.h"

#include <QtCore/QString>
#include <meojson/json.hpp>
#include <functional>

namespace Task {

class RouteContext;

class Router : public std::enable_shared_from_this<Router> {
protected:
    friend class RouteContext;

public:
    using OperatorMethod = std::function<bool(const Prop &prop, const QVariant &operand)>;

    struct Condition {
        QString  key;
        QString  op;
        QVariant operand;

        static Condition parse(const json::object &data);
    };

    struct TaskInfo {
        std::optional<QString>      action;
        std::optional<QStringList>  action_args;
        std::optional<QString>      task_entry;
        std::optional<json::object> override_task_params;
        std::optional<Condition>    trigger_condition;

        static TaskInfo parse(const json::object &data);
    };

    struct TaskUnit {
        std::optional<TaskInfo>        direct_entry;
        std::optional<QList<TaskInfo>> exclusive_task_group;

        static TaskUnit parse(const json::value &data);
    };

    struct PipelineUnit {
        std::optional<Condition> trigger_condition;
        QList<TaskUnit>          tasks;

        //! TODO: support `repeat`

        static PipelineUnit parse(const json::object &data);
    };

public:
    static void setup();
    static bool register_operator(const QString &op, OperatorMethod method);
    static bool unregister_operator(const QString &op);
    static void parse(Router &router, const json::object &route_data);

    [[nodiscard]] json::object                origin_task_params(const QString &task_name) const;
    [[nodiscard]] std::shared_ptr<PropGetter> major_task_config(MajorTask major_task) const;

    bool contains_route_of(MajorTask major_task) const {
        return task_routes_.contains(major_task);
    }

    [[nodiscard]] std::shared_ptr<RouteContext> route(MajorTask major_task) const;
    std::reference_wrapper<QList<PipelineUnit>> pipelines(MajorTask major_task);

    Router(std::shared_ptr<Config> config, std::shared_ptr<TaskGraph> task_graph);

private:
    QMap<MajorTask, QList<PipelineUnit>> task_routes_;
    std::shared_ptr<Config>              config_;
    std::shared_ptr<TaskGraph>           task_graph_;
};

class RouteContext : public std::enable_shared_from_this<RouteContext> {
public:
    using Condition = Router::Condition;

    enum class State {
        Idle,
        Running,
        Stop,
    };

    struct Task {
        QString      task_entry;
        json::object params;
    };

public:
    RouteContext(std::shared_ptr<Router> router, MajorTask major_task);

    bool test_condition(const Condition &condition);
    bool test_condition(const QString &op, const Prop &prop, const QVariant &operand);

    State state() const {
        return state_;
    }

    bool                start();
    void                stop();
    bool                has_next() const;
    std::optional<Task> next();

    std::optional<QString> var(const QString &name) const;
    void                   update_var(const QString &name, const QString &value) const;

protected:
    QString to_varid(const QString &name) const {
        return '$' + name;
    }

    void execute_internal_action(const QString &action, const QStringList &args = QStringList());

private:
    const MajorTask         major_task_;
    QMap<QString, QString>  local_vars_;
    std::shared_ptr<Router> router_;
    State                   state_;
    int                     selected_pipeline_index_;
    int                     pipeline_stage_;
};

} // namespace Task

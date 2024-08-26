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

#include "TaskRouterSchema.h"
#include "TaskGraph.h"
#include "../Property.h"

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <meojson/json.hpp>
#include <memory>
#include <functional>

namespace Experimental {

using RouterOperatorMethod = std::function<bool(const Property &prop, const QVariant &operand)>;

class TaskRouteContext;

class TaskRouter : public std::enable_shared_from_this<TaskRouter> {
    friend class TaskRouteContext;

public:
    static void        setup();
    static bool        register_operator(const QString &name, RouterOperatorMethod method);
    static bool        unregister_operator(const QString &name, RouterOperatorMethod method);
    static bool        has_operator(const QString &name);
    static QStringList supported_operators();

    static std::shared_ptr<TaskRouter>
        create(std::shared_ptr<TaskGraph> task_graph, std::shared_ptr<PropertyContext> property_context);

    void reload(const json::object &data);

    bool has_route_for(const QString &task_name) const {
        return routes_.contains(task_name);
    }

    QStringList tasks() {
        return routes_.keys();
    }

    [[nodiscard]] std::shared_ptr<TaskRouteContext>        route(const QString &task_name) const;
    std::optional<std::reference_wrapper<const TaskRoute>> route_info(const QString &task_name) const;

protected:
    TaskRouter(std::shared_ptr<TaskGraph> task_graph, std::shared_ptr<PropertyContext> property_context)
        : task_graph_(task_graph)
        , property_context_(property_context) {}

private:
    QMap<QString, TaskRoute>         routes_;
    std::shared_ptr<TaskGraph>       task_graph_;
    std::shared_ptr<PropertyContext> property_context_;
};

class TaskRouteContext {
public:
    enum State {
        Idle,
        Paused,
        Running,
    };

    struct Task {
        QString      entry;
        json::object params;
    };

public:
    [[nodiscard]] static std::shared_ptr<TaskRouteContext>
        create(std::shared_ptr<const TaskRouter> router, const QString &task_name);

    bool is_running() const {
        return state_ == Running;
    }

    bool is_paused() const {
        return state_ == Paused;
    }

    bool is_idle() const {
        return state_ == Idle;
    }

    bool can_start() const {
        return is_idle();
    }

    bool can_resume() const {
        return is_paused();
    }

    int state() const {
        return state_;
    }

    QString task_name() const {
        return task_name_;
    }

    bool start();
    bool resume();
    bool restart();
    void stop();
    void pause();

    Property prop(const QString &name) const;
    QString  merge_props(const QString &input) const;

    bool test_condition(const TaskRouteCondition &condition);
    bool test_condition(const QString &op, const Property &prop, const QVariant &operand);

    int                 total_stages() const;
    bool                has_next() const;
    std::optional<Task> next();

    void set_var_cleared_on_restart(bool on) {
        var_cleared_on_restart_ = on;
    }

    bool is_var_cleared_on_restart() const {
        return var_cleared_on_restart_;
    }

    std::optional<QString> var(const QString &name) const;
    void                   update_var(const QString &name, const QString &value);

protected:
    void execute_internal_action(const QString &action, const QStringList &args = QStringList());

    TaskRouteContext(std::shared_ptr<const TaskRouter> router, const QString &task_name)
        : router_(router)
        , task_name_(task_name)
        , state_(Idle)
        , var_cleared_on_restart_(true) {}

private:
    const std::shared_ptr<const TaskRouter> router_;
    const QString                           task_name_;
    int                                     state_;
    bool                                    var_cleared_on_restart_;
    std::optional<int>                      opt_next_stage_;
    std::optional<int>                      opt_selected_route_;
    QMap<QString, QString>                  local_vars_;
};

} // namespace Experimental

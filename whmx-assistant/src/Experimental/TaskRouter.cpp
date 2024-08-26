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

#include "TaskRouter.h"
#include "../PropertyContext.h"

#include <QtCore/QRegularExpression>
#include <shared_mutex>

namespace Experimental {

QMap<QString, RouterOperatorMethod> ROUTER_OPERATORS;
std::shared_mutex                   ROUTER_OPERATORS_MUTEX;

static bool operator_eq(const Property &prop, const QVariant &operand) {
    if (prop.is_int() && operand.typeId() == QMetaType::Int) { return prop.as_int() == operand.toInt(); }
    if (prop.is_real() && operand.typeId() == QMetaType::Double) { return prop.as_real() == operand.toDouble(); }
    if (prop.is_str() && operand.typeId() == QMetaType::QString) { return prop.as_str() == operand.toString(); }
    if (prop.is_enum() && operand.typeId() == QMetaType::QString) { return prop.as_enum() == operand.toString(); }
    if (prop.is_opt()) { return operator_eq(prop.as_opt().value(), operand); }
    return false;
}

static bool operator_ne(const Property &prop, const QVariant &operand) {
    if (prop.is_int() && operand.typeId() == QMetaType::Int) { return prop.as_int() != operand.toInt(); }
    if (prop.is_real() && operand.typeId() == QMetaType::Double) { return prop.as_real() != operand.toDouble(); }
    if (prop.is_str() && operand.typeId() == QMetaType::QString) { return prop.as_str() != operand.toString(); }
    if (prop.is_enum() && operand.typeId() == QMetaType::QString) { return prop.as_enum() != operand.toString(); }
    if (prop.is_opt()) { return operator_ne(prop.as_opt().value(), operand); }
    return false;
}

static bool operator_lt(const Property &prop, const QVariant &operand) {
    if (prop.is_int() && operand.typeId() == QMetaType::Int) { return prop.as_int() < operand.toInt(); }
    if (prop.is_real() && operand.typeId() == QMetaType::Double) { return prop.as_real() < operand.toDouble(); }
    return false;
}

static bool operator_le(const Property &prop, const QVariant &operand) {
    if (prop.is_int() && operand.typeId() == QMetaType::Int) { return prop.as_int() <= operand.toInt(); }
    if (prop.is_real() && operand.typeId() == QMetaType::Double) { return prop.as_real() <= operand.toDouble(); }
    return false;
}

static bool operator_gt(const Property &prop, const QVariant &operand) {
    if (prop.is_int() && operand.typeId() == QMetaType::Int) { return prop.as_int() > operand.toInt(); }
    if (prop.is_real() && operand.typeId() == QMetaType::Double) { return prop.as_real() > operand.toDouble(); }
    return false;
}

static bool operator_ge(const Property &prop, const QVariant &operand) {
    if (prop.is_int() && operand.typeId() == QMetaType::Int) { return prop.as_int() >= operand.toInt(); }
    if (prop.is_real() && operand.typeId() == QMetaType::Double) { return prop.as_real() >= operand.toDouble(); }
    return false;
}

static bool operator_empty(const Property &prop, const QVariant &operand) {
    if (prop.is_str()) { return prop.as_str().isEmpty(); }
    if (prop.is_list()) { return prop.as_list().empty(); }
    return false;
}

static bool operator_true(const Property &prop, const QVariant &operand) {
    if (prop.is_bool()) { return prop.as_bool(); }
    if (prop.is_int()) { return prop.as_int() != 0; }
    return false;
}

static bool operator_false(const Property &prop, const QVariant &operand) {
    if (prop.is_bool()) { return !prop.as_bool(); }
    if (prop.is_int()) { return prop.as_int() == 0; }
    return false;
}

static bool operator_between(const Property &prop, const QVariant &operand) {
    if (prop.is_int() && operand.typeId() == QMetaType::QString) {
        const auto bounds = operand.toString().split(',');
        if (bounds.size() != 2) { return false; }
        bool      ok[2];
        const int lower = bounds[0].trimmed().toInt(&ok[0]);
        const int upper = bounds[1].trimmed().toInt(&ok[1]);
        if (!(ok[0] && ok[1])) { return false; }
        return prop.as_int() >= lower && prop.as_int() <= upper;
    }
    if (prop.is_real() && operand.typeId() == QMetaType::QString) {
        const auto bounds = operand.toString().split(',');
        if (bounds.size() != 2) { return false; }
        bool         ok[2];
        const double lower = bounds[0].trimmed().toDouble(&ok[0]);
        const double upper = bounds[1].trimmed().toDouble(&ok[1]);
        if (!(ok[0] && ok[1])) { return false; }
        return prop.as_real() >= lower && prop.as_real() <= upper;
    }
    return false;
}

static bool operator_contains(const Property &prop, const QVariant &operand) {
    if (prop.is_str()) { return operand.typeId() == QMetaType::QString ? prop.as_str().contains(operand.toString()) : false; }
    if (prop.is_list()) {
        for (const auto &value : prop.as_list()) {
            if (operator_eq(value, operand)) { return true; }
        }
        return false;
    }
    if (prop.is_object() && operand.typeId() == QMetaType::QString) { return prop.as_object().contains(operand.toString()); }
    return false;
}

static bool operator_in(const Property &prop, const QVariant &operand) {
    if (operand.typeId() == QMetaType::QString) { return prop.is_str() ? operand.toString().contains(prop.as_str()) : false; }
    if (operand.typeId() == QMetaType::QVariantList) {
        for (const auto &value : operand.value<QVariantList>()) {
            if (operator_eq(prop, value)) { return true; }
        }
        return false;
    }
    if (operand.typeId() == QMetaType::QVariantMap) {
        return prop.is_str() ? operand.value<QVariantMap>().contains(prop.as_str()) : false;
    }
    return false;
}

static bool operator_match(const Property &prop, const QVariant &operand) {
    if (prop.is_str() && operand.typeId() == QMetaType::QString) {
        QRegularExpression re(operand.toString());
        return re.isValid() ? re.match(prop.as_str()).hasMatch() : false;
    }
    return false;
}

static bool operator_null(const Property &prop, const QVariant &operand) {
    return prop.is_opt() && !prop.as_opt().has_value();
}

static bool operator_undefined(const Property &prop, const QVariant &operand) {
    return prop.is_null();
}

void TaskRouter::setup() {
    register_operator("eq", operator_eq);
    register_operator("ne", operator_ne);
    register_operator("lt", operator_lt);
    register_operator("le", operator_le);
    register_operator("gt", operator_gt);
    register_operator("ge", operator_ge);
    register_operator("empty", operator_empty);
    register_operator("true", operator_true);
    register_operator("false", operator_false);
    register_operator("between", operator_between);
    register_operator("contains", operator_contains);
    register_operator("in", operator_in);
    register_operator("match", operator_match);
    register_operator("null", operator_null);
    register_operator("undefined", operator_undefined);
}

bool TaskRouter::register_operator(const QString &name, RouterOperatorMethod method) {
    std::unique_lock lock(ROUTER_OPERATORS_MUTEX);
    if (ROUTER_OPERATORS.contains(name)) { return false; }
    ROUTER_OPERATORS.insert(name, method);
    return true;
}

bool TaskRouter::unregister_operator(const QString &name, RouterOperatorMethod method) {
    std::unique_lock lock(ROUTER_OPERATORS_MUTEX);
    if (!ROUTER_OPERATORS.contains(name)) { return false; }
    ROUTER_OPERATORS.remove(name);
    return true;
}

bool TaskRouter::has_operator(const QString &name) {
    std::shared_lock lock(ROUTER_OPERATORS_MUTEX);
    return ROUTER_OPERATORS.contains(name);
}

QStringList TaskRouter::supported_operators() {
    std::shared_lock lock(ROUTER_OPERATORS_MUTEX);
    return ROUTER_OPERATORS.keys();
}

std::shared_ptr<TaskRouter>
    TaskRouter::create(std::shared_ptr<TaskGraph> task_graph, std::shared_ptr<PropertyContext> property_context) {
    if (!task_graph || !property_context) { return nullptr; }
    return std::shared_ptr<TaskRouter>(new TaskRouter(task_graph, property_context));
}

void TaskRouter::reload(const json::object &data) {
    routes_.clear();
    for (const auto &[task, route] : data) { routes_.insert(QString::fromUtf8(task), TaskRoute::parse(route)); }
}

std::shared_ptr<TaskRouteContext> TaskRouter::route(const QString &task_name) const {
    if (!has_route_for(task_name)) { return nullptr; }
    return TaskRouteContext::create(shared_from_this(), task_name);
}

std::optional<std::reference_wrapper<const TaskRoute>> TaskRouter::route_info(const QString &task_name) const {
    if (!has_route_for(task_name)) { return std::nullopt; }
    return std::cref(routes_.constFind(task_name).value());
}

std::shared_ptr<TaskRouteContext> TaskRouteContext::create(std::shared_ptr<const TaskRouter> router, const QString &task_name) {
    if (!router || !router->has_route_for(task_name)) { return nullptr; }
    return std::shared_ptr<TaskRouteContext>(new TaskRouteContext(router, task_name));
}

bool TaskRouteContext::start() {
    if (!can_start()) { return false; }

    const auto opt_route_info = router_->route_info(task_name_);
    Q_ASSERT(opt_route_info.has_value());
    const auto &route_info = opt_route_info->get();

    std::optional<int> opt_selected_route;
    if (route_info.bound_task.has_value()) {
        opt_selected_route = 0;
    } else if (route_info.bound_pipeline.has_value()) {
        const auto &pipelines = route_info.bound_pipeline.value();
        for (int i = 0; i < pipelines.size(); ++i) {
            const auto &pipeline = pipelines[i];
            if (!pipeline.trigger_condition.has_value()) {
                opt_selected_route = i;
                break;
            }
            if (test_condition(pipeline.trigger_condition.value())) {
                opt_selected_route = i;
                break;
            }
        }
    } else {
        Q_UNREACHABLE();
    }

    if (!opt_selected_route.has_value()) { return false; }

    if (is_var_cleared_on_restart()) { local_vars_.clear(); }

    state_              = Running;
    opt_next_stage_     = 0;
    opt_selected_route_ = opt_selected_route;

    return true;
}

bool TaskRouteContext::resume() {
    if (!can_resume()) { return false; }
    state_ = Running;
    return true;
}

bool TaskRouteContext::restart() {
    stop();
    return start();
}

void TaskRouteContext::stop() {
    opt_next_stage_     = std::nullopt;
    opt_selected_route_ = std::nullopt;
    state_              = Idle;
}

void TaskRouteContext::pause() {
    if (!is_running()) { return; }
    state_ = Paused;
}

Property TaskRouteContext::prop(const QString &name) const {
    if (const auto opt_prop = router_->property_context_->property(task_name_, name)) {
        return opt_prop->get();
    } else if (local_vars_.contains(name)) {
        const auto &value = local_vars_.constFind(name).value();
        return router_->property_context_->create_primary_meta_property<PropertyMetaType::Str>(value);
    } else {
        return Property::nullprop();
    }
}

QString TaskRouteContext::merge_props(const QString &input) const {
    const auto               pattern = R"(\$\{\{ *(\S+) *\}\})";
    const QRegularExpression re(pattern);

    QString merged;
    int     last_pos = 0;
    for (const auto match : re.globalMatch(input)) {
        merged          += input.mid(last_pos, match.capturedStart() - last_pos);
        const auto key   = match.captured(1);
        const auto prop  = this->prop(key);
        QString    replaced;
        if (false) {
        } else if (prop.is_bool()) {
            replaced = prop.as_bool() ? "true" : "false";
        } else if (prop.is_int()) {
            replaced = QString::number(prop.as_int());
        } else if (prop.is_real()) {
            replaced = QString::number(prop.as_real());
        } else if (prop.is_str()) {
            replaced = prop.as_str();
        } else {
            replaced = match.captured();
        }
        merged   += replaced;
        last_pos  = match.capturedEnd();
    }
    merged += input.mid(last_pos);

    return merged;
}

bool TaskRouteContext::test_condition(const TaskRouteCondition &condition) {
    return test_condition(condition.op, prop(condition.key), condition.operand);
}

bool TaskRouteContext::test_condition(const QString &op, const Property &prop, const QVariant &operand) {
    RouterOperatorMethod method;
    {
        std::shared_lock lock(ROUTER_OPERATORS_MUTEX);
        if (!ROUTER_OPERATORS.contains(op)) { return false; }
        method = ROUTER_OPERATORS.find(op).value();
    }
    return std::invoke(method, prop, operand);
}

int TaskRouteContext::total_stages() const {
    Q_ASSERT(is_running());
    Q_ASSERT(opt_selected_route_.has_value());
    const auto opt_route_info = router_->route_info(task_name_);
    Q_ASSERT(opt_route_info.has_value());
    const auto &route_info     = opt_route_info->get();
    const int   selected_route = opt_selected_route_.value();
    if (route_info.bound_task.has_value()) {
        Q_ASSERT(selected_route == 0);
        return 1;
    } else if (route_info.bound_pipeline.has_value()) {
        return route_info.bound_pipeline->at(selected_route).tasks.size();
    } else {
        Q_UNREACHABLE();
    }
}

bool TaskRouteContext::has_next() const {
    Q_ASSERT(is_running());
    Q_ASSERT(opt_next_stage_.has_value());
    const int next_stage = opt_next_stage_.value();
    return next_stage < total_stages();
}

std::optional<TaskRouteContext::Task> TaskRouteContext::next() {
    if (is_idle() || !has_next()) { return std::nullopt; }

    const auto opt_route_info = router_->route_info(task_name_);
    Q_ASSERT(opt_route_info.has_value());
    const auto &route_info = opt_route_info->get();

    Q_ASSERT(opt_next_stage_.has_value());
    Q_ASSERT(opt_selected_route_.has_value());

    Task next_task;

    if (route_info.bound_task.has_value()) {
        const int next_stage = opt_next_stage_.value();
        opt_next_stage_      = next_stage + 1;
        Q_ASSERT(!has_next());
        next_task.entry = route_info.bound_task.value();
        return next_task;
    }

    const auto &pipeline = route_info.bound_pipeline->at(opt_selected_route_.value());
    while (has_next()) {
        const int next_stage  = opt_next_stage_.value();
        opt_next_stage_       = next_stage + 1;
        const auto &task_unit = pipeline.tasks.at(next_stage);

        std::optional<std::reference_wrapper<const TaskRouteTaskInfo>> opt_task_info;
        if (task_unit.direct_entry.has_value()) {
            const auto &task_info = task_unit.direct_entry.value();
            if (!task_info.trigger_condition.has_value() || test_condition(task_info.trigger_condition.value())) {
                opt_task_info = std::cref(task_unit.direct_entry.value());
            }
        } else if (task_unit.exclusive_task_group.has_value()) {
            const auto &task_group = task_unit.exclusive_task_group.value();
            for (const auto &task_info : task_group) {
                if (!task_info.trigger_condition.has_value() || test_condition(task_info.trigger_condition.value())) {
                    opt_task_info = std::cref(task_info);
                    break;
                }
            }
        } else {
            Q_UNREACHABLE();
        }

        if (!opt_task_info.has_value()) { continue; }

        const auto &task_info = opt_task_info->get();

        if (task_info.action.has_value()) {
            execute_internal_action(task_info.action.value(), task_info.action_args.value_or(QStringList()));
        }

        //! NOTE: internal action may break the running state
        if (!is_running()) { break; }

        if (!task_info.task_entry.has_value()) { continue; }

        const auto &task_entry              = task_info.task_entry.value();
        const auto  origin_params_for_entry = router_->task_graph_->task_params(task_entry);

        std::optional<json::object> opt_override_params = std::nullopt;
        if (const auto opt = task_info.override_task_params) {
            const auto merged = merge_props(QString::fromUtf8(opt.value().to_string()));
            //! FIXME: handle parse error?
            opt_override_params = std::make_optional<json::object>(json::parse(merged.toStdString()).value_or(*opt));
        }

        json::object entry_task_params;
        bool         has_unfolded_param_for_entry_task = false;
        if (task_info.fold_params) {
            entry_task_params = opt_override_params.value_or(json::object()) | origin_params_for_entry;
        } else if (opt_override_params.has_value() && opt_override_params->contains(task_entry.toStdString())) {
            has_unfolded_param_for_entry_task = true;
            entry_task_params = opt_override_params->at(task_entry.toStdString()).as_object() | origin_params_for_entry;
        } else {
            entry_task_params = origin_params_for_entry;
        }

        json::object final_task_params{
            {task_entry.toStdString(), entry_task_params},
        };
        if (!task_info.fold_params && opt_override_params.has_value()) {
            auto unfolded_params = opt_override_params.value();
            if (has_unfolded_param_for_entry_task) { unfolded_params.erase(task_entry.toStdString()); }
            final_task_params |= unfolded_params;
        }

        next_task.entry  = std::move(task_entry);
        next_task.params = std::move(final_task_params);
        return next_task;
    }

    return std::nullopt;
}

std::optional<QString> TaskRouteContext::var(const QString &name) const {
    if (local_vars_.contains(name)) {
        return local_vars_.constFind(name).value();
    } else {
        return std::nullopt;
    }
}

void TaskRouteContext::update_var(const QString &name, const QString &value) {
    local_vars_.insert(name, value);
}

void TaskRouteContext::execute_internal_action(const QString &action, const QStringList &args) {
    if (false) {
    } else if (action == "break") {
        stop();
    } else if (action == "set-var") {
        Q_ASSERT(args.size() == 2);
        update_var(args[0], args[1]);
    } else {
        //! TODO: throw exception?
    }
}

} // namespace Experimental

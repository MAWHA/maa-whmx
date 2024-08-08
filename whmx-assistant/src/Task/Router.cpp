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

#include "Router.h"
#include "TaskParam.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QFile>
#include <magic_enum.hpp>

namespace Task {

QMap<QString, Router::OperatorMethod> GLOBAL_ROUTER_OPERATORS;

static bool operator_eq(const Prop &prop, const QVariant &operand) {
    if (prop.type_id() != operand.typeId()) { return false; }
    if (prop.type_id() == QMetaType::Int) { return prop.to_int() == operand.toInt(); }
    if (prop.type_id() == QMetaType::QString) { return prop.as<QString>() == operand.toString(); }
    return false;
}

static bool operator_ne(const Prop &prop, const QVariant &operand) {
    if (prop.type_id() != operand.typeId()) { return false; }
    if (prop.type_id() == QMetaType::Int) { return prop.to_int() != operand.toInt(); }
    if (prop.type_id() == QMetaType::QString) { return prop.as<QString>() != operand.toString(); }
    return false;
}

static bool operator_lt(const Prop &prop, const QVariant &operand) {
    if (prop.type_id() != operand.typeId()) { return false; }
    if (prop.type_id() == QMetaType::Int) { return prop.to_int() < operand.toInt(); }
    return false;
}

static bool operator_le(const Prop &prop, const QVariant &operand) {
    if (prop.type_id() != operand.typeId()) { return false; }
    if (prop.type_id() == QMetaType::Int) { return prop.to_int() <= operand.toInt(); }
    return false;
}

static bool operator_gt(const Prop &prop, const QVariant &operand) {
    if (prop.type_id() != operand.typeId()) { return false; }
    if (prop.type_id() == QMetaType::Int) { return prop.to_int() > operand.toInt(); }
    return false;
}

static bool operator_ge(const Prop &prop, const QVariant &operand) {
    if (prop.type_id() != operand.typeId()) { return false; }
    if (prop.type_id() == QMetaType::Int) { return prop.to_int() >= operand.toInt(); }
    return false;
}

static bool operator_empty(const Prop &prop, const QVariant &operand) {
    if (prop.type_id() == QMetaType::QStringList) { return prop.as<QStringList>().empty(); }
    if (prop.type_id() == QMetaType::QVariantList) { return prop.as<QVariantList>().empty(); }
    return false;
}

static bool operator_true(const Prop &prop, const QVariant &operand) {
    if (prop.type_id() == QMetaType::Bool) { return prop.to_bool(); }
    if (prop.type_id() == QMetaType::Int) { return prop.to_int() != 0; }
    return false;
}

static bool operator_false(const Prop &prop, const QVariant &operand) {
    if (prop.type_id() == QMetaType::Bool) { return !prop.to_bool(); }
    if (prop.type_id() == QMetaType::Int) { return prop.to_int() == 0; }
    return false;
}

static bool operator_between(const Prop &prop, const QVariant &operand) {
    if (prop.type_id() == QMetaType::Int && operand.typeId() == QMetaType::QString) {
        const auto bounds = operand.toString().split(',');
        if (bounds.size() != 2) { return false; }
        const int lower = bounds[0].toInt();
        const int upper = bounds[1].toInt();
        return prop.to_int() >= lower && prop.to_int() <= upper;
    }
    return false;
}

static bool operator_contains(const Prop &prop, const QVariant &operand) {
    if (prop.type_id() == QMetaType::QString && operand.typeId() == QMetaType::QStringList) {
        return prop.as<QStringList>().contains(operand.toString());
    }
    return false;
}

static bool operator_match(const Prop &prop, const QVariant &operand) {
    if (prop.type_id() == QMetaType::QString && operand.typeId() == QMetaType::QString) {
        QRegularExpression regex(operand.toString());
        if (!regex.isValid()) { return false; }
        return regex.match(prop.as<QString>()).hasMatch();
    }
    return false;
}

static bool operator_in(const Prop &prop, const QVariant &operand) {
    if (prop.type_id() == QMetaType::QString && operand.typeId() == QMetaType::QStringList) {
        return operand.toStringList().contains(prop.as<QString>());
    }
    return false;
}

void Router::setup() {
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
    register_operator("match", operator_match);
    register_operator("in", operator_in);
}

bool Router::register_operator(const QString &op, OperatorMethod method) {
    if (GLOBAL_ROUTER_OPERATORS.contains(op)) { return false; }
    GLOBAL_ROUTER_OPERATORS[op] = method;
    return true;
}

bool Router::unregister_operator(const QString &op) {
    if (!GLOBAL_ROUTER_OPERATORS.contains(op)) { return false; }
    GLOBAL_ROUTER_OPERATORS.remove(op);
    return true;
}

Router::Condition Router::Condition::parse(const json::object &data) {
    Condition condition;
    condition.key = QString::fromUtf8(data.get("key", ""));
    condition.op  = QString::fromUtf8(data.get("op", ""));
    if (data.contains("operand")) {
        const auto operand = data.at("operand");
        if (operand.is_string()) {
            condition.operand = QString::fromUtf8(operand.as_string());
        } else if (operand.is_number()) {
            condition.operand = operand.as_integer();
        } else if (operand.is_array()) {
            QStringList values;
            for (const auto &value : operand.as_array()) { values.append(QString::fromUtf8(value.as_string())); }
            condition.operand = values;
        }
    }
    return condition;
}

Router::TaskInfo Router::TaskInfo::parse(const json::object &data) {
    TaskInfo task_info;
    if (data.contains("action")) {
        const auto action = data.at("action");
        if (action.is_string()) {
            task_info.action = QString::fromUtf8(action.as_string());
        } else if (action.is_object()) {
            task_info.action = QString::fromUtf8(action.at("name").as_string());
            QStringList args;
            for (const auto &arg : action.at("param").as_array()) { args.append(QString::fromUtf8(arg.as_string())); }
            task_info.action_args = args;
        }
    }
    if (data.contains("entry")) {
        task_info.task_entry = QString::fromUtf8(data.at("entry").as_string());
        if (data.contains("param")) { task_info.override_task_params = data.at("param").as_object(); }
    }
    if (data.contains("on")) { task_info.trigger_condition = Condition::parse(data.at("on").as_object()); }
    return task_info;
}

Router::TaskUnit Router::TaskUnit::parse(const json::value &data) {
    TaskUnit task;
    if (data.is_object()) {
        task.direct_entry = TaskInfo::parse(data.as_object());
    } else if (data.is_array()) {
        QList<TaskInfo> task_group;
        for (const auto &entry : data.as_array()) { task_group.append(TaskInfo::parse(entry.as_object())); }
        task.exclusive_task_group = std::move(task_group);
    }
    return task;
}

Router::PipelineUnit Router::PipelineUnit::parse(const json::object &data) {
    PipelineUnit pipeline;
    if (!data.contains("pipeline") || !data.at("pipeline").is_array()) { return pipeline; }
    if (data.contains("on") && data.at("on").is_object()) {
        pipeline.trigger_condition = Condition::parse(data.at("on").as_object());
    }
    for (auto &entry : data.at("pipeline").as_array()) { pipeline.tasks.append(TaskUnit::parse(entry)); }
    return pipeline;
}

void Router::parse(Router &router, const json::object &route_data) {
    QMap<QString, MajorTask> rev_index_table;
    for (const auto &major_task : magic_enum::enum_values<MajorTask>()) {
        rev_index_table.insert(QString::fromLatin1(magic_enum::enum_name(major_task)), major_task);
    }

    auto &routes = router.task_routes_;
    routes.clear();

    for (const auto &[key, value] : route_data) {
        const auto major_task_name = QString::fromUtf8(key);
        if (!rev_index_table.contains(major_task_name)) { continue; }
        if (!value.is_array()) { continue; }
        const auto major_task = rev_index_table[major_task_name];
        if (!routes.contains(major_task)) { routes.insert(major_task, {}); }
        auto &route = routes[major_task];
        for (const auto &entry : value.as_array()) { route.append(PipelineUnit::parse(entry.as_object())); }
    }
}

json::object Router::origin_task_params(const QString &task_name) const {
    do {
        if (const auto opt_task = task_graph_->get(task_name)) {
            const auto task = opt_task.value();
            QFile      file(task_graph_->sources[task->source_nr]);
            if (!file.open(QIODevice::ReadOnly)) { break; }
            auto data = json::parse(QString::fromUtf8(file.readAll()).toStdString());
            if (!data.has_value()) { break; }
            Q_ASSERT(data->contains(task_name.toStdString()));
            return data->at(task_name.toStdString()).as_object();
        }
    } while (0);
    return json::object();
}

std::shared_ptr<PropGetter> Router::major_task_config(MajorTask major_task) const {
    const auto param = config_->task_params.value(major_task, get_default_task_param(major_task));
    return dump_task_param_to_prop_getter(param);
}

std::shared_ptr<RouteContext> Router::route(MajorTask major_task) const {
    if (!contains_route_of(major_task)) { return nullptr; }
    return std::make_shared<RouteContext>(const_cast<Router *>(this)->shared_from_this(), major_task);
}

std::reference_wrapper<QList<Router::PipelineUnit>> Router::pipelines(MajorTask major_task) {
    static QList<PipelineUnit> SHARED_NULL_PIPELINES;
    return task_routes_.contains(major_task) ? std::ref(task_routes_[major_task]) : std::ref(SHARED_NULL_PIPELINES);
}

Router::Router(std::shared_ptr<Config> config, std::shared_ptr<TaskGraph> task_graph)
    : config_(config)
    , task_graph_(task_graph) {
    Q_ASSERT(config != nullptr);
    Q_ASSERT(task_graph_ != nullptr);
}

RouteContext::RouteContext(std::shared_ptr<Router> router, MajorTask major_task)
    : major_task_(major_task)
    , router_(router)
    , state_(State::Idle)
    , selected_pipeline_index_(-1)
    , pipeline_stage_(-1) {}

bool RouteContext::test_condition(const Condition &condition) {
    const auto props = router_->major_task_config(major_task_);
    Prop       prop;
    if (props->contains(condition.key)) {
        prop = props->value(condition.key);
    } else if (const auto opt_var = var(condition.key)) {
        prop = opt_var.value();
    }
    if (prop.is_null()) { return false; }
    return test_condition(condition.op, prop, condition.operand);
}

bool RouteContext::test_condition(const QString &op, const Prop &prop, const QVariant &operand) {
    if (!GLOBAL_ROUTER_OPERATORS.contains(op)) { return false; }
    return GLOBAL_ROUTER_OPERATORS[op](prop, operand);
}

bool RouteContext::start() {
    if (state_ != State::Idle) { return false; }

    const auto pipelines = router_->pipelines(major_task_);
    for (int i = 0; i < pipelines.get().size(); ++i) {
        const auto &pipeline = pipelines.get()[i];
        if (!pipeline.trigger_condition.has_value()) {
            selected_pipeline_index_ = i;
            break;
        }
        if (test_condition(pipeline.trigger_condition.value())) {
            selected_pipeline_index_ = i;
            break;
        }
    }

    if (selected_pipeline_index_ != -1) {
        pipeline_stage_ = 0;
        state_          = State::Running;
        return true;
    } else {
        state_ = State::Stop;
        return false;
    }
}

void RouteContext::stop() {
    state_ = State::Stop;
}

bool RouteContext::has_next() const {
    if (selected_pipeline_index_ == -1) { return false; }
    const int total_stages = router_->pipelines(major_task_).get()[selected_pipeline_index_].tasks.size();
    return pipeline_stage_ < total_stages;
}

std::optional<RouteContext::Task> RouteContext::next() {
    if (!has_next() || state_ != State::Running) { return std::nullopt; }

    const auto &pipeline = router_->pipelines(major_task_).get()[selected_pipeline_index_];
    while (has_next()) {
        const auto &task_unit = pipeline.tasks[pipeline_stage_++];

        std::optional<std::reference_wrapper<const Router::TaskInfo>> opt_target_task;
        if (task_unit.direct_entry.has_value()) {
            const auto &task_info = task_unit.direct_entry.value();
            if (!task_info.trigger_condition.has_value() || test_condition(task_info.trigger_condition.value())) {
                opt_target_task = std::cref(task_unit.direct_entry.value());
            }
        } else if (task_unit.exclusive_task_group.has_value()) {
            const auto &task_group = task_unit.exclusive_task_group.value();
            for (const auto &task_info : task_group) {
                if (!task_info.trigger_condition.has_value() || test_condition(task_info.trigger_condition.value())) {
                    opt_target_task = std::cref(task_unit.direct_entry.value());
                    break;
                }
            }
        }

        if (opt_target_task.has_value()) {
            const auto &target_task = opt_target_task.value().get();
            if (target_task.action.has_value()) {
                execute_internal_action(target_task.action.value(), target_task.action_args.value_or(QStringList()));
            }
            if (state_ != State::Running) { break; }
            if (target_task.task_entry.has_value()) {
                const auto        &task_name     = target_task.task_entry.value();
                const auto         origin_params = router_->origin_task_params(task_name);
                const json::object task_params   = target_task.override_task_params.value_or(json::object()) | origin_params;
                return std::make_optional<Task>({task_name, task_params});
            }
        }
    }
    return std::nullopt;
}

std::optional<QString> RouteContext::var(const QString &name) const {
    const auto var_id = to_varid(name);
    if (local_vars_.contains(var_id)) {
        return std::make_optional(local_vars_[var_id]);
    } else {
        return std::nullopt;
    }
}

void RouteContext::update_var(const QString &name, const QString &value) const {
    const auto var_id   = to_varid(name);
    local_vars_[var_id] = value;
}

void RouteContext::execute_internal_action(const QString &action, const QStringList &args) {
    if (false) {
    } else if (action == "break") {
        stop();
    } else if (action == "set-var") {
        if (args.size() == 2) {
            update_var(args[0], args[1]);
        } else {
            qWarning() << "RouteContext: set-var action with invalid args";
        }
    } else {
        qWarning().noquote() << "RouteContext: unknown internal action" << action;
    }
}

} // namespace Task

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

#include "TaskGraph.h"

#include <QtCore/QFile>
#include <QtCore/QSet>
#include <meojson/json.hpp>
#include <initializer_list>

void TaskGraphNode::add_succ(std::shared_ptr<TaskGraphNode> succ) {
    if (contains_succ(succ->task_name)) { return; }
    if (succ->contains_succ(task_name)) {
        succs.emplace_back(succ->weak_from_this());
        succ->preds.emplace_back(weak_from_this());
    } else {
        succs.emplace_back(succ);
        succ->preds.emplace_back(shared_from_this());
    }
}

void TaskGraphNode::add_pred(std::shared_ptr<TaskGraphNode> pred) {
    if (contains_pred(pred->task_name)) { return; }
    if (pred->contains_pred(task_name)) {
        preds.emplace_back(pred->weak_from_this());
        pred->succs.emplace_back(weak_from_this());
    } else {
        preds.emplace_back(pred);
        pred->succs.emplace_back(shared_from_this());
    }
}

bool TaskGraphNode::contains_succ(const QString &succ_name) const {
    for (const auto &succ : succs) {
        if (succ_name == succ->task_name) { return true; }
    }
    return false;
}

bool TaskGraphNode::contains_pred(const QString &pred_name) const {
    for (const auto &pred : preds) {
        if (pred_name == pred->task_name) { return true; }
    }
    return false;
}

void TaskGraph::clear() {
    nodes.clear();
}

bool TaskGraph::contains(const QString &task_name) const {
    for (const auto &node : nodes) {
        if (task_name == node->task_name) { return true; }
    }
    return false;
}

std::optional<std::shared_ptr<TaskGraphNode>> TaskGraph::get(const QString &task_name) const {
    for (const auto &node : nodes) {
        if (task_name == node->task_name) { return std::make_optional(node); }
    }
    return std::nullopt;
}

std::shared_ptr<TaskGraphNode> TaskGraph::add_node(const QString &task_name) {
    if (const auto opt_node = get(task_name)) {
        return opt_node.value();
    } else {
        auto node       = std::make_shared<TaskGraphNode>();
        node->task_name = task_name;
        nodes.append(node);
        return node;
    }
}

void TaskGraph::add_edge(const QString &pred_name, const QString &succ_name) {
    add_node(pred_name)->add_succ(add_node(succ_name));
}

bool TaskGraph::merge_pipeline(const QString &pipeline_file) {
    QFile file(pipeline_file);
    if (!file.open(QIODevice::ReadOnly)) { return false; }

    auto data = json::parse(QString::fromUtf8(file.readAll()).toStdString());
    if (!data.has_value()) { return false; }

    for (const auto &[task_name, params] : data->as_object()) {
        const auto pred = QString::fromUtf8(task_name);
        add_node(pred);
        for (const auto key : std::initializer_list<std::string>{"next", "timeout_next", "runout_next"}) {
            if (!params.contains(key)) { continue; }
            if (const auto &next = params.at(key); next.is_array()) {
                for (const auto &succ : next.as_array()) { add_edge(pred, QString::fromUtf8(succ.as_string())); }
            } else if (next.is_string()) {
                add_edge(pred, QString::fromUtf8(next.as_string()));
            }
        }
    }

    return true;
}

QStringList TaskGraph::root_tasks() const {
    QStringList resp;
    for (const auto &node : nodes) {
        if (node->is_root()) { resp.append(node->task_name); }
    }
    return resp;
}

QStringList TaskGraph::find_left_root_tasks(const QStringList &exclude_tasks) const {
    QSet<QString> excludes(exclude_tasks.begin(), exclude_tasks.end());

    QSet<QPair<QString, QString>> visited_edges;
    QSet<QString>                 visited_succs;
    for (const auto &node : nodes) {
        if (!excludes.contains(node->task_name)) { continue; }
        QList<std::shared_ptr<TaskGraphNode>> stack(node->succs);
        while (!stack.empty()) {
            const auto succ_node = stack.back();
            stack.pop_back();
            const auto edge = qMakePair(node->task_name, succ_node->task_name);
            if (visited_edges.contains(edge)) { continue; }
            stack.append(succ_node->succs);
            visited_edges.insert(edge);
            visited_succs.insert(succ_node->task_name);
        }
    }

    QStringList resp;
    for (const auto &node : nodes) {
        if (visited_succs.contains(node->task_name)) { continue; }
        if (node->is_root()) {
            resp.append(node->task_name);
        } else if (node->preds.size() == 1 && node->preds.at(0) == node) {
            resp.append(node->task_name);
        }
    }
    return resp;
}

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

#include <QtCore/QList>
#include <QtCore/QString>
#include <meojson/json.hpp>
#include <memory>
#include <optional>

namespace Experimental {

struct TaskGraphNode : public std::enable_shared_from_this<TaskGraphNode> {
    QString task_name;
    int     source_nr;

    QList<std::shared_ptr<TaskGraphNode>> preds;
    QList<std::shared_ptr<TaskGraphNode>> succs;

    void add_succ(std::shared_ptr<TaskGraphNode> succ);
    void add_pred(std::shared_ptr<TaskGraphNode> pred);
    bool contains_succ(const QString &succ_name) const;
    bool contains_pred(const QString &pred_name) const;

    bool is_root() const {
        return preds.empty();
    }

    bool is_terminal() const {
        return succs.empty();
    }

    bool is_intermediate() const {
        return !is_root() && !is_terminal();
    }
};

struct TaskGraph {
    QList<std::shared_ptr<TaskGraphNode>> nodes;
    QStringList                           sources;

    void clear();
    bool contains(const QString &task_name) const;

    int                                           make_source_nr(const QString &pipeline_file);
    std::optional<std::shared_ptr<TaskGraphNode>> get(const QString &task_name) const;
    std::shared_ptr<TaskGraphNode>                add_node(const QString &task_name, const QString &pipeline_file = QString());
    void                                          add_edge(const QString &pred_name, const QString &succ_name);

    bool        merge_pipeline(const QString &pipeline_file);
    QStringList root_tasks() const;
    QStringList find_left_root_tasks(const QStringList &exclude_tasks) const;

    [[nodiscard]] json::object task_params(const QString &task_name) const;
};

} // namespace Experimental

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

#include <QtCore/QObject>
#include <gsl/gsl>

#include "../UI/PropertyHelper.h"

namespace Experimental {

class TaskItem : public QObject {
    Q_OBJECT
    Q_DECLARE_PROPERTY(QList<TaskItem*>, sub_items)
    Q_DECLARE_PROPERTY(bool, expanded)
    Q_DECLARE_PROPERTY(TaskItem*, parent)
    Q_DECLARE_PROPERTY(QString, title)
    Q_DECLARE_PROPERTY(bool, checked)

public:
    QString key() const {
        return key_;
    }

    QString task_id() const {
        Expects(leaf_ && opt_task_id_.has_value());
        return opt_task_id_.value();
    }

    bool is_leaf() const {
        return leaf_;
    }

    int            total_sub_items() const;
    void           set_sub_items_checked(bool checked);
    Qt::CheckState sub_items_check_state() const;

    bool      append(gsl::not_null<TaskItem*> item);
    TaskItem* append_non_leaf(const QString& title, bool expanded = false);
    TaskItem* append_leaf(const QString& task_id, const QString& title, bool checked = false);

    TaskItem* row(int index) const;
    int       row_index() const;

    void sort(Qt::SortOrder order = Qt::AscendingOrder);

public:
    TaskItem(const QString& title, TaskItem* parent = nullptr);
    TaskItem(const QString& task_id, const QString& title, TaskItem* parent = nullptr);

private:
    QString                key_         = QString();
    std::optional<QString> opt_task_id_ = std::nullopt;
    bool                   leaf_        = true;
};

} // namespace Experimental

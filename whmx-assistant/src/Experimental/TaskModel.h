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

#include <QtCore/QAbstractItemModel>
#include <QtCore/QObject>
#include <QtCore/QMap>

namespace Experimental {

class TaskItem;

class TaskModel : public QAbstractItemModel {
    Q_OBJECT

public:
    int total_items() const {
        return items_.size();
    }

    std::shared_ptr<TaskItem> root() const {
        return root_;
    }

    TaskItem* item(const QString& key) const {
        return items_.value(key, nullptr);
    }

    QString append(TaskItem* item);
    QString append(TaskItem* parent, TaskItem* item);

    QString append_non_leaf(const QString& title, bool expanded);
    QString append_leaf(const QString& task_id, const QString& title, bool checked);
    QString append_non_leaf(const QString& parent_key, const QString& title, bool expanded);
    QString append_leaf(const QString& parent_key, const QString& task_id, const QString& title, bool checked);
    QString append_non_leaf(TaskItem* parent, const QString& title, bool expanded);
    QString append_leaf(TaskItem* parent, const QString& task_id, const QString& title, bool checked);

public:
    TaskModel(QObject* parent = nullptr);

    QModelIndex   parent(const QModelIndex& child) const override;
    QModelIndex   index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int           rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int           columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant      data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool          setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void          sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
    QMap<QString, TaskItem*>  items_;
    std::shared_ptr<TaskItem> root_;
};

} // namespace Experimental

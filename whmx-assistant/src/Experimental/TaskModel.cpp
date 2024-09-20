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

#include "TaskModel.h"
#include "TaskItem.h"

namespace Experimental {

QString TaskModel::append(TaskItem* item) {
    return append(nullptr, item);
}

QString TaskModel::append(TaskItem* parent, TaskItem* item) {
    if (!item) { return QString(); }
    if (items_.contains(item->key())) { return QString(); }
    if (parent && !items_.contains(parent->key())) { return QString(); }
    if (!parent) { parent = root_.get(); }
    if (parent->is_leaf()) { return QString(); }
    parent->append(item);
    items_.insert(item->key(), item);
    return item->key();
}

QString TaskModel::append_non_leaf(const QString& title, bool expanded) {
    return append_non_leaf(nullptr, title, expanded);
}

QString TaskModel::append_leaf(const QString& task_id, const QString& title, bool checked) {
    return append_leaf(nullptr, task_id, title, checked);
}

QString TaskModel::append_non_leaf(const QString& parent_key, const QString& title, bool expanded) {
    if (auto parent = item(parent_key)) {
        return append_non_leaf(parent, title, expanded);
    } else {
        return QString();
    }
}

QString TaskModel::append_leaf(const QString& parent_key, const QString& task_id, const QString& title, bool checked) {
    if (auto parent = item(parent_key)) {
        return append_leaf(parent, task_id, title, checked);
    } else {
        return QString();
    }
}

QString TaskModel::append_non_leaf(TaskItem* parent, const QString& title, bool expanded) {
    if (parent && !items_.contains(parent->key())) { return QString(); }
    if (!parent) { parent = root_.get(); }
    if (parent->is_leaf()) { return QString(); }
    const auto item = parent->append_non_leaf(title, expanded);
    items_.insert(item->key(), item);
    return item->key();
}

QString TaskModel::append_leaf(TaskItem* parent, const QString& task_id, const QString& title, bool checked) {
    if (parent && !items_.contains(parent->key())) { return QString(); }
    if (!parent) { parent = root_.get(); }
    if (parent->is_leaf()) { return QString(); }
    const auto item = parent->append_leaf(task_id, title, checked);
    items_.insert(item->key(), item);
    return item->key();
}

TaskModel::TaskModel(QObject* parent)
    : QAbstractItemModel(parent)
    , root_(std::make_shared<TaskItem>("$root")) {}

QModelIndex TaskModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) { return QModelIndex(); }
    if (child.column() != 0) { return QModelIndex(); }
    const auto item   = static_cast<TaskItem*>(child.internalPointer());
    const auto parent = item->parent();
    if (!parent || parent == root_.get()) { return QModelIndex(); }
    return createIndex(parent->row_index(), 0, parent);
}

QModelIndex TaskModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) { return QModelIndex(); }
    const auto parent_item = parent.isValid() ? static_cast<TaskItem*>(parent.internalPointer()) : root_.get();
    TaskItem*  child_item  = nullptr;
    if (parent_item->total_sub_items() <= row) {
        return QModelIndex();
    } else {
        Q_ASSERT(column == 0);
        return createIndex(row, column, parent_item->row(row));
    }
}

int TaskModel::rowCount(const QModelIndex& parent) const {
    if (parent.column() > 0) { return 0; }
    const auto parent_item = parent.isValid() ? static_cast<TaskItem*>(parent.internalPointer()) : root_.get();
    return parent_item->total_sub_items();
}

int TaskModel::columnCount(const QModelIndex& parent) const {
    return 1;
}

QVariant TaskModel::data(const QModelIndex& index, int role) const {
    Q_ASSERT(index.isValid());
    const auto item = static_cast<TaskItem*>(index.internalPointer());
    if (role == Qt::DisplayRole) {
        return item->title();
    } else if (role == Qt::CheckStateRole) {
        if (item->is_leaf()) {
            return item->checked() ? Qt::Checked : Qt::Unchecked;
        } else {
            return item->sub_items_check_state();
        }
    }
    return QVariant();
}

bool TaskModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (role == Qt::CheckStateRole) {
        const bool checked = value.value<Qt::CheckState>() == Qt::Checked;
        auto       item    = static_cast<TaskItem*>(index.internalPointer());
        if (item->is_leaf()) {
            item->set_checked(checked);
        } else {
            item->set_sub_items_checked(checked);
        }
        emit dataChanged(QModelIndex(), QModelIndex(), {role});
        return true;
    }
    return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags TaskModel::flags(const QModelIndex& index) const {
    Qt::ItemFlags flags  = QAbstractItemModel::flags(index);
    flags               |= Qt::ItemIsUserCheckable;
    return flags;
}

} // namespace Experimental

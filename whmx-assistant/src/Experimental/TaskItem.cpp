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

#include "TaskItem.h"

#include <QtCore/QUuid>

namespace Experimental {

int TaskItem::total_sub_items() const {
    return is_leaf() ? 0 : const_cast<TaskItem*>(this)->sub_items().size();
}

void TaskItem::set_sub_items_checked(bool checked) {
    for (auto item : sub_items()) {
        if (item->is_leaf()) {
            item->set_checked(checked);
        } else {
            item->set_sub_items_checked(checked);
        }
    }
}

Qt::CheckState TaskItem::sub_items_check_state() const {
    if (is_leaf()) { return Qt::Unchecked; }
    int total_checked   = 0;
    int total_unchecked = 0;
    for (const auto& item : sub_items()) {
        Qt::CheckState state = Qt::Unchecked;
        if (item->is_leaf()) {
            state = item->checked() ? Qt::Checked : Qt::Unchecked;
        } else {
            state = item->sub_items_check_state();
        }
        if (state == Qt::PartiallyChecked) {
            return Qt::PartiallyChecked;
        } else if (state == Qt::Checked) {
            ++total_checked;
            if (total_unchecked > 0) { return Qt::PartiallyChecked; }
        } else if (state == Qt::Unchecked) {
            ++total_unchecked;
            if (total_checked > 0) { return Qt::PartiallyChecked; }
        }
    }
    return total_checked == 0 ? Qt::Unchecked : Qt::Checked;
}

void TaskItem::append(TaskItem* item) {
    if (is_leaf()) { return; }
    sub_items().append(item);
}

TaskItem* TaskItem::append_non_leaf(const QString& title, bool expanded) {
    auto item = new TaskItem(title, false, this);
    item->set_expanded(expanded);
    append(item);
    return item;
}

TaskItem* TaskItem::append_leaf(const QString& title, bool checked) {
    auto item = new TaskItem(title, true, this);
    item->set_checked(checked);
    append(item);
    return item;
}

TaskItem* TaskItem::row(int index) const {
    if (is_leaf()) { return nullptr; }
    return const_cast<TaskItem*>(this)->sub_items().at(index);
}

int TaskItem::row_index() const {
    if (is_leaf()) { return 0; }
    return parent()->sub_items().indexOf(this);
}

TaskItem::TaskItem(const QString& title, bool leaf, TaskItem* parent)
    : QObject(parent)
    , key_(QUuid::createUuid().toString(QUuid::Id128))
    , leaf_(leaf) {
    set_checked(false);
    set_title(title);
    set_expanded(false);
    set_parent(parent);
}

} // namespace Experimental

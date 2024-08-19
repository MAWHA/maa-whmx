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

#include "NavModel.h"

namespace UI {

NavNode *NavModel::node(const QString &key) const {
    for (auto node : nodes_) {
        if (node->key() == key) { return node; }
    }
    return nullptr;
}

QModelIndex NavModel::node_index(const QString &key) const {
    for (int i = 0; i < nodes_.size(); ++i) {
        if (nodes_[i]->key() == key) { return index(i); }
    }
    return QModelIndex();
}

QModelIndex NavModel::node_index(NavNode *node) const {
    for (int i = 0; i < nodes_.size(); ++i) {
        if (nodes_[i] == node) { return index(i); }
    }
    return QModelIndex();
}

bool NavModel::contains(const QString &key) const {
    return node_index(key).isValid();
}

bool NavModel::contains(NavNode *node) const {
    return nodes_.contains(node);
}

void NavModel::add_node(NavNode *node) {
    if (!node) { return; }
    beginResetModel();
    nodes_.append(node);
    endResetModel();
}

void NavModel::activate_node(NavNode *node) {
    if (!contains(node)) { return; }
    if (false) {
    } else if (auto widget = node->widget()) {
        set_selected_node(node);
    } else if (auto action = node->action()) {
        emit action->triggered();
    }
}

void NavModel::clear_selection() {
    set_selected_node(nullptr);
}

NavModel::NavModel(QObject *parent)
    : QAbstractListModel(parent) {
    set_selected_node(nullptr);
}

int NavModel::rowCount(const QModelIndex &parent) const {
    return nodes_.size();
}

QVariant NavModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::UserRole && index.row() < nodes_.size()) { return QVariant::fromValue(nodes_[index.row()]); }
    return QVariant();
}

} // namespace UI

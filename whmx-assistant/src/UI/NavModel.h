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

#include "PropertyHelper.h"
#include "NavNode.h"

#include <QtCore/QAbstractListModel>

namespace UI {

class NavModel : public QAbstractListModel {
    Q_OBJECT

    Q_DECLARE_PROPERTY(NavNode *, selected_node);

public:
    NavNode    *node(const QString &key) const;
    QModelIndex node_index(const QString &key) const;
    QModelIndex node_index(NavNode *node) const;
    bool        contains(const QString &key) const;
    bool        contains(NavNode *node) const;
    void        add_node(NavNode *node);
    void        activate_node(NavNode *node);
    void        clear_selection();

public:
    NavModel(QObject *parent = nullptr);

    int      rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QList<NavNode *> nodes_;
};

} // namespace UI

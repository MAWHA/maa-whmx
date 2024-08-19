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

#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QListView>

namespace UI {

class NavItemDelegate : public QStyledItemDelegate {
public:
    NavItemDelegate(QListView* view, QObject* parent = nullptr)
        : QStyledItemDelegate(parent)
        , view_(view) {}

protected:
    void  paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QListView* view_ = nullptr;
};

} // namespace UI

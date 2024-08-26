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

#include "TaskView.h"
#include "TaskViewStyle.h"
#include "TaskItem.h"

#include <QtGui/QContextMenuEvent>
#include <ElaScrollBar.h>
#include <ElaMenu.h>

namespace Experimental {

TaskView::TaskView(QWidget *parent)
    : QTreeView(parent) {
    setObjectName("TaskView");
    setStyleSheet("#TaskView{background-color:transparent;border:0px;}");
    setMouseTracking(true);
    setHeaderHidden(true);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto scrollbar = new ElaScrollBar(this);
    scrollbar->setisAnimation(true);
    connect(scrollbar, &ElaScrollBar::rangeAnimationFinished, this, [this] {
        viewport()->update();
    });

    setVerticalScrollBar(scrollbar);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    setStyle(new TaskViewStyle);
}

void TaskView::contextMenuEvent(QContextMenuEvent *event) {
    const auto index = indexAt(event->pos());
    if (!index.isValid()) { return; }

    const auto item = static_cast<const TaskItem *>(index.constInternalPointer());

    auto menu = new ElaMenu(this);
    menu->setMenuItemHeight(27);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (item->is_leaf()) {
        const auto action_config = menu->addElaIconAction(ElaIconType::GearCode, "配置");
        connect(action_config, &QAction::triggered, this, [=, this] {
            emit on_open_task_config(index);
        });
    }

    const auto action_property = menu->addElaIconAction(ElaIconType::ListUl, "属性");
    connect(action_property, &QAction::triggered, this, [=, this] {
        emit on_open_task_property(index);
    });

    menu->popup(event->globalPos());
}

} // namespace Experimental

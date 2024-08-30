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

#include "UmaWorkbench.h"
#include "TaskView.h"
#include "TaskModel.h"
#include "StyledPanel.h"

#include <QtWidgets/QHBoxLayout>

namespace Experimental {

UmaWorkbench::UmaWorkbench(gsl::not_null<std::shared_ptr<UmaInstance>> instance)
    : instance_(instance) {
    setup();
}

void UmaWorkbench::setup() {
    auto task_view = new TaskView;
    task_view->setFixedWidth(240);
    {
        auto model = new TaskModel(task_view);
        task_view->setModel(model);
        QMap<QString, QString> keys;
        for (const auto &[task_id, meta] : instance_->uma_prop->interface->tasks.asKeyValueRange()) {
            if (meta.opt_category.has_value()) {
                const auto category = meta.opt_category.value();
                if (!keys.contains(category)) {
                    const auto key = model->append_non_leaf(category, true);
                    keys.insert(category, key);
                }
                model->append_leaf(keys.find(category).value(), meta.name, false);
            } else {
                model->append_leaf(meta.name, false);
            }
        }
    }

    auto task_view_panel = new StyledPanel(task_view);
    task_view_panel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    task_view_panel->set_shadow_width(3);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(task_view_panel);
    layout->addStretch();
}

} // namespace Experimental

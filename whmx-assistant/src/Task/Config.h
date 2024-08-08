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

#include "MajorTask.h"

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <memory>

namespace Task {

class Router;

struct Config {
    QMap<MajorTask, QString>  task_entries;
    QMap<MajorTask, QVariant> task_params;
};

void                    reset_shared_task_config(const std::shared_ptr<Config> config);
std::shared_ptr<Config> shared_task_config();

bool     load_task_config(Config& config, const QString& file_path, std::shared_ptr<Router> router = nullptr);
QVariant get_default_task_param(MajorTask task);

} // namespace Task

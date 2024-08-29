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

#include "../DeviceManager.h"
#include "../TaskInterface.h"
#include "Package.h"
#include "TaskRouter.h"
#include "TaskGraph.h"
#include "MessageProducer.h"

#include <MaaPP/MaaPP.hpp>
#include <QtCore/QObject>
#include <QtCore/QUuid>
#include <QtCore/QDir>
#include <QtGui/QColor>
#include <memory>

namespace Experimental {

struct MaaProperty {
    std::shared_ptr<::maa::Instance>   handle;
    std::shared_ptr<::maa::Resource>   res;
    std::shared_ptr<::maa::Controller> ctrl;
};

struct UmaProperty {
    std::shared_ptr<Package>       package;
    std::shared_ptr<TaskInterface> interface;
    std::shared_ptr<TaskGraph>     task_graph;
    std::shared_ptr<TaskRouter>    task_router;

    static std::shared_ptr<UmaProperty> create(std::shared_ptr<Package> package) {
        QDir package_dir(package->location());
        auto prop       = std::make_shared<UmaProperty>();
        prop->package   = package;
        prop->interface = package_dir.exists("task.json") ? TaskInterface::load(package_dir.absoluteFilePath("task.json"))
                                                          : std::make_shared<TaskInterface>();
        return prop;
    }
};

struct UmaInstance {
    QString                          instance_id;
    QString                          name;
    QString                          comment;
    QColor                           marker_color;
    QString                          device;
    QString                          cache_key;
    std::shared_ptr<UmaProperty>     uma_prop;
    std::shared_ptr<MaaProperty>     maa_prop;
    std::shared_ptr<MessageProducer> message_producer;

    static std::shared_ptr<UmaInstance> create(std::shared_ptr<Package> package) {
        auto instance         = std::make_shared<UmaInstance>();
        instance->instance_id = QUuid::createUuid().toString();
        instance->uma_prop    = UmaProperty::create(package);
        return instance;
    }

    static std::shared_ptr<UmaInstance> from_local_storage(const QString &id) {
        return nullptr;
    }
};

} // namespace Experimental

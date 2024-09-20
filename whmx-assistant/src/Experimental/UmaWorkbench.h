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

#include "ActuatorInstance.h"

#include <QtWidgets/QWidget>
#include <gsl/gsl>

namespace Experimental {

class UmaWorkbench : public QWidget {
protected slots:
    void open_task_config(const QModelIndex &index);
    void open_task_property(const QModelIndex &index);

public:
    UmaWorkbench(gsl::not_null<std::shared_ptr<UmaInstance>> instance);

protected:
    void setup();

private:
    gsl::not_null<std::shared_ptr<UmaInstance>> instance_;
};

} // namespace Experimental

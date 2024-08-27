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

#include "AppEvent.h"

#include <QtCore/QSharedMemory>
#include <QtWidgets/QApplication>
#include <MaaPP/MaaPP.hpp>
#include <gsl/gsl>
#include <tl/expected.hpp>

#define gApp static_cast<UniversalMaaActuator *>(qApp)

class UniversalMaaActuator final : public QApplication {
public:
    static auto create(int &argc, char **argv) -> tl::expected<gsl::not_null<std::shared_ptr<UniversalMaaActuator>>, QString>;

    auto window() const -> gsl::not_null<std::shared_ptr<QWidget>>;
    auto window_cref() const -> gsl::strict_not_null<const QWidget *>;
    void wakeup_client();

    gsl::strict_not_null<AppEvent *> app_event() const {
        return app_event_;
    }

protected:
    UniversalMaaActuator(int &argc, char **argv, gsl::not_null<gsl::owner<QSharedMemory *>> singleton_ipc_memory);

    void setup();

private:
    gsl::not_null<std::shared_ptr<maa::coro::EventLoop>> default_maa_event_loop_;
    gsl::strict_not_null<gsl::owner<QThread *>>          maa_worker_;
    gsl::strict_not_null<gsl::owner<AppEvent *>>         app_event_;
    gsl::not_null<std::unique_ptr<QSharedMemory>>        singleton_ipc_memory_;
};

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

#include "Consts.h"
#include "Logger.h"
#include "UI/Client.h"

#include <MaaPP/MaaPP.hpp>
#include <QtWidgets/QApplication>

using namespace maa;

coro::Promise<int> async_main() {
    //! ATTENTION: ctor of client contains async calls, thus the maa::coro::EventLoop must be created before it
    UI::Client client;
    client.show();

    //! FIXME: coro::EventLoop must wait for all async calls to complete before returning, and the app dtor may destroy the
    //! dependent objects held by the async calls

    co_return QApplication::exec();
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QApplication::setOrganizationDomain("github.com/MAWHA");
    QApplication::setOrganizationName("MAWHA");
    QApplication::setApplicationName("whmx-assistant");
    QApplication::setApplicationDisplayName("物华弥新小助手");
    QApplication::setApplicationVersion(QString::fromUtf8(Consts::VERSION));

    GlobalLoggerProxy::setup();

    //! TODO: optimize threads assignment

    coro::EventLoop ev;
    ev.stop_after(async_main());
    return ev.exec();
}

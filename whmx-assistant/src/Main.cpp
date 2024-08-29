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

#include "App.h"
#include "Logger.h"

int main(int argc, char *argv[]) {
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");
#ifndef NDEBUG
    qputenv("QT_DEBUG_PLUGINS", "1");
#endif

    auto app_resp = UniversalMaaActuator::create(argc, argv);
    if (!app_resp) {
        LOG_ERROR().noquote() << "failed to start uma:" << app_resp.error();
        return -1;
    }

    const auto app = app_resp.value();

    app->wakeup_client();
    return app->exec();
}

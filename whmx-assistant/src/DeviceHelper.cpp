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

#include "DeviceHelper.h"

#include <QtCore/QDebug>
#include <regex>
#include <algorithm>

using namespace maa;

coro::Promise<QList<AdbDevice>> list_adb_devices() {
    const auto devices = co_await AdbDeviceFinder::find();
    if (!devices || devices->empty()) { co_return {}; }
    co_return QList<AdbDevice>(devices->begin(), devices->end());
}

coro::Promise<std::optional<AdbDevice>> find_adb_device(const std::string &adb_hint) {
    const auto devices = co_await list_adb_devices();
    if (devices.empty()) {
        qDebug() << "no adb devices found";
        co_return std::nullopt;
    }
    const auto device_resp = std::find_if(devices.begin(), devices.end(), [pattern = std::regex(adb_hint)](const auto &device) {
        return std::regex_match(device.name, pattern);
    });
    if (device_resp == devices.end()) {
        qDebug() << "no device matched regex hint" << adb_hint;
        co_return std::nullopt;
    }
    co_return std::make_optional(*device_resp);
}

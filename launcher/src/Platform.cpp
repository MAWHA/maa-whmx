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

#include "Platform.h"

#include <QtCore/QString>
#include <QtCore/QRegularExpression>

#if !defined(WIN32)
#error "non-windows platform is not supported yet"
#endif

#define NO_MIN_MAX
#define MEAN_AND_LEAN
#include <windows.h>
#include <FileLocksmithLibInterop.h>

namespace Platform {

bool kill_occupied_process(std::string_view target_file, std::string_view pattern) {
    const auto result = FileLocksmith::find_processes_recursive(QString::fromUtf8(target_file).toStdWString());

    const auto         pat = QString::fromUtf8(pattern);
    QRegularExpression regex(pat);

    bool all_done = true;
    for (const auto& item : result) {
        const auto name = QString::fromStdWString(item.name);
        if (!(name == pat || regex.isValid() && regex.match(name).hasMatch())) { continue; }
        const auto proc = OpenProcess(PROCESS_ALL_ACCESS, false, item.pid);
        const bool done = TerminateProcess(proc, 0);
        if (!done) { all_done = false; }
    }

    return all_done;
}

} // namespace Platform

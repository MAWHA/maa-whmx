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

#include <filesystem>
#include <assert.h>

#define NO_MIN_MAX
#define MEAN_AND_LEAN
#include <windows.h>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
    TCHAR pathbuf[MAX_PATH];
    GetModuleFileName(NULL, pathbuf, MAX_PATH);

    const auto app_dir = fs::path(pathbuf).parent_path();

    const auto app_path = app_dir / TEXT("whmx-assistant.exe");
    const auto log_path = app_dir / TEXT("debug/maa.log");

    assert(fs::exists(app_path));

    STARTUPINFO         si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(si);

    const bool started = CreateProcess(app_path.c_str(), nullptr, nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi);
    assert(started);

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    const bool all_killed = Platform::kill_occupied_process(log_path.generic_string(), R"(^adb\.exe$)");
    assert(all_killed);

    return 0;
}

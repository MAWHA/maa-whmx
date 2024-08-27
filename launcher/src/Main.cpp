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
#include "Consts.h"
#include "VirtualDesktop.h"

#include <QtCore/QSharedMemory>
#include <QtCore/QDebug>
#include <gsl/gsl>
#include <filesystem>
#include <assert.h>

#define NO_MIN_MAX
#define MEAN_AND_LEAN
#include <windows.h>

namespace fs = std::filesystem;

struct CoInstanceGuard {
    CoInstanceGuard() {
        const int hr = CoInitialize(nullptr);
        Ensures(SUCCEEDED(hr));
    }

    ~CoInstanceGuard() {
        CoUninitialize();
    }
};

std::optional<HWND> active_singleton_window_handle() {
    const auto SINGLETON_IPC_MEMORY_ID = QString("%1::UniversalMaaActuator.Singleton").arg(QString::fromUtf8(Consts::MUA_UUID));
    QSharedMemory shared_memory(SINGLETON_IPC_MEMORY_ID);
    if (const bool created = shared_memory.create(sizeof(HWND))) {
        shared_memory.detach();
        return std::nullopt;
    }
    const bool attached = shared_memory.attach(QSharedMemory::ReadOnly);
    Ensures(attached);
    const gsl::not_null singleton_window_handle = *reinterpret_cast<HWND *>(shared_memory.data());
    Ensures(IsWindow(singleton_window_handle));
    shared_memory.detach();
    return singleton_window_handle;
}

void locate_and_wakeup_window(HWND hwnd) {
    //! be careful to wakeup a originally non-visible window, here simply ignore it
    if (!IsWindowVisible(hwnd)) { return; }

    //! restore minimized window, it will also switch the current virtual desktop to the window's desktop
    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
        return;
    }

    //! ok, now we shall check whether the window is on the current virtual desktop, or we need to switch to it

    IServiceProvider *service_provider = nullptr;
    {
        const int hr = CoCreateInstance(
            CLSID_ImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER, __uuidof(IServiceProvider), (void **)&service_provider);
        Ensures(SUCCEEDED(hr));
        Ensures(service_provider != nullptr);
    }

    IVirtualDesktopManager *desktop_manager = nullptr;
    {
        const int hr = service_provider->QueryService(__uuidof(IVirtualDesktopManager), &desktop_manager);
        Ensures(SUCCEEDED(hr));
        Ensures(desktop_manager != nullptr);
    }

    std::optional<GUID> opt_desktop_uuid;
    {
        int       on_current_desktop = 0;
        const int hr                 = desktop_manager->IsWindowOnCurrentVirtualDesktop(hwnd, &on_current_desktop);
        Ensures(SUCCEEDED(hr));
        if (!on_current_desktop) {
            GUID      desktop_id;
            const int hr = desktop_manager->GetWindowDesktopId(hwnd, &desktop_id);
            Ensures(SUCCEEDED(hr));
            opt_desktop_uuid = desktop_id;
        }
    }

    if (opt_desktop_uuid.has_value()) {
        IVirtualDesktopManagerInternal *virtual_desktop_manager = nullptr;
        {
            const int hr = service_provider->QueryService(
                CLSID_VirtualDesktopAPI_Unknown, IID_IVirtualDesktopManagerInternal, (void **)&virtual_desktop_manager);
            Ensures(SUCCEEDED(hr));
            Ensures(virtual_desktop_manager != nullptr);
        }
        IVirtualDesktop *target_desktop = nullptr;
        {
            auto      desktop_uuid = opt_desktop_uuid.value();
            const int hr           = virtual_desktop_manager->FindDesktop(&desktop_uuid, &target_desktop);
            Ensures(SUCCEEDED(hr));
            Ensures(target_desktop != nullptr);
        }
        virtual_desktop_manager->SwitchDesktop(target_desktop);
        target_desktop->Release();
        virtual_desktop_manager->Release();
    }

    desktop_manager->Release();
    service_provider->Release();
}

void launch_and_wait_application(const std::wstring &app_path) {
    Expects(fs::exists(app_path));

    STARTUPINFO         si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(si);

    const bool started = CreateProcess(app_path.c_str(), nullptr, nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi);
    Ensures(started);

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main(int argc, char *argv[]) {
    CoInstanceGuard co_instance_guard;

    if (const auto opt_window_handle = active_singleton_window_handle()) {
        locate_and_wakeup_window(opt_window_handle.value());
        return 0;
    }

    TCHAR pathbuf[MAX_PATH];
    GetModuleFileName(nullptr, pathbuf, MAX_PATH);

    const auto app_dir = fs::path(pathbuf).parent_path();

    const auto app_path = app_dir / TEXT("whmx-assistant.exe");
    const auto log_path = app_dir / TEXT("debug/maa.log");

    launch_and_wait_application(app_path);

    const bool all_killed = Platform::kill_occupied_process(log_path.generic_string(), R"(^adb\.exe$)");
    Ensures(all_killed);

    return 0;
}

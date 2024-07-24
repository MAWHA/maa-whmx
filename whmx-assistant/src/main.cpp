#include "CustomRecognizer/Research.h"
#include "CustomAction/Research.h"

#include <MaaPP/MaaPP.hpp>
#include <iostream>
#include <regex>
#include <filesystem>
#include <optional>
#include <windows.h>

namespace fs = std::filesystem;

using namespace maa;

coro::Promise<std::optional<AdbDevice>> find_adb_device(const std::string &adb_hint) {
    const auto devices = co_await AdbDeviceFinder::find();
    if (!devices || devices->empty()) {
        std::clog << "no device found" << std::endl;
        co_return std::nullopt;
    }

    const auto device_resp =
        std::find_if(devices->begin(), devices->end(), [pattern = std::regex(adb_hint)](const auto &device) {
            return std::regex_match(device.name, pattern);
        });
    if (device_resp == devices->end()) {
        std::clog << "no device matched regex hint \"" << adb_hint << "\"" << std::endl;
        co_return std::nullopt;
    }

    co_return std::make_optional(*device_resp);
}

fs::path get_application_dir() {
    char path[MAX_PATH]{};
    GetModuleFileName(NULL, path, sizeof(path));
    return fs::path(path).parent_path();
}

coro::Promise<int> async_main() {
    const fs::path    app_dir   = get_application_dir();
    const fs::path    agent_dir = app_dir / "agent";
    const fs::path    res_dir   = app_dir / "assets" / "general";
    const std::string adb_hint  = R"(MuMuPlayer\d+)";
    const std::string package   = "com.cipaishe.wuhua.bilibili";
    const std::string activity  = "com.cipaishe.wuhua.bilibili/com.cipaishe.wuhua.bilibili.activity.ONESDKSplashActivity";

    init(app_dir.string());

    const auto device_resp = co_await find_adb_device(adb_hint);
    if (!device_resp.has_value()) {
        std::clog << "failed to create abd controller" << std::endl;
        co_return -1;
    }

    auto controller = Controller::make(device_resp.value(), agent_dir.string())
                          ->set_long_side(1280)
                          ->set_short_side(720)
                          ->set_start_entry(activity)
                          ->set_stop_entry(package);
    auto ctrl_req_conn = controller->post_connect();

    auto resource     = Resource::make();
    auto res_req_path = resource->post_path(res_dir.string());

    co_await ctrl_req_conn->wait();
    co_await res_req_path->wait();

    auto instance = Instance::make()->bind(resource)->bind(controller);
    if (!instance->inited()) {
        std::clog << "failed to init MAA" << std::endl;
        co_return -1;
    }

    //! TODO: check if the app is already started
    //! TODO: check if the app is in the foreground

    // co_await controller->post_start_app(activity)->wait();

    instance->bind<Rec::Research::ParseGradeOptionsOnModify>();
    instance->bind<Action::Research::SelectGradeOption>();

    const auto entry_task = "Test.ReservedTaskToAvoidErrorAlert";
    co_await instance->post_task(entry_task)->wait();

    co_return 0;
}

int main(int argc, char *argv[]) {
    coro::EventLoop ev;
    ev.stop_after(async_main());
    return ev.exec();
}

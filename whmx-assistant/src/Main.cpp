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

#include "CustomRecognizer/Research.h"
#include "CustomAction/Research.h"
#include "ReferenceDataSet.h"
#include "Consts.h"
#include "DeviceHelper.h"

#include <MaaPP/MaaPP.hpp>
#include <filesystem>
#include <QApplication>
#include <QThread>

namespace fs = std::filesystem;

using namespace maa;

fs::path get_application_dir() {
    return fs::path(QApplication::applicationDirPath().toStdString());
}

coro::Promise<int> async_main() {
    const fs::path    app_dir       = get_application_dir();
    const fs::path    agent_dir     = app_dir / "agents";
    const fs::path    res_dir       = app_dir / "assets" / "general";
    const fs::path    res_data_dir  = app_dir / "assets" / "data";
    const fs::path    anecdotes_dir = res_data_dir / "anecdotes.json";
    const std::string adb_hint      = R"(MuMuPlayer\d+)";

    init(app_dir.string());

    {
        const bool ok = Ref::ResearchAnecdoteSet::instance()->load(anecdotes_dir.string());
        Q_ASSERT(ok);
    }

    const auto device_resp = co_await find_adb_device(adb_hint);
    if (!device_resp.has_value()) {
        qDebug("failed to create adb controller");
        co_return -1;
    }

    auto controller = Controller::make(device_resp.value(), agent_dir.string())
                          ->set_long_side(1280)
                          ->set_short_side(720)
                          ->set_start_entry(Consts::ACTIVITY)
                          ->set_stop_entry(Consts::PACKAGE);
    auto ctrl_req_conn = controller->post_connect();

    auto resource     = Resource::make();
    auto res_req_path = resource->post_path(res_dir.string());

    co_await ctrl_req_conn->wait();
    co_await res_req_path->wait();

    auto instance = Instance::make()->bind(resource)->bind(controller);
    if (!instance->inited()) {
        qDebug("failed to init MAA");
        co_return -1;
    }

    //! TODO: check if the app is already started
    //! TODO: check if the app is in the foreground

    // co_await controller->post_start_app(activity)->wait();

    instance->bind<Rec::Research::ParseGradeOptionsOnModify>();
    instance->bind<Rec::Research::ParseAnecdote>();
    instance->bind<Action::Research::SelectGradeOption>();

    // const auto entry_task = "Test.ReservedTaskToAvoidErrorAlert";
    const auto entry_task = "Test.Research.GetAnecdoteData";
    co_await instance->post_task(entry_task)->wait();

    co_return 0;
}

class MaaWorker : public QThread {
public:
    void run() override {
        coro::EventLoop ev;
        ev.stop_after(async_main());
        exit(ev.exec());
    }

    int ret_code() const {
        return ret_code_;
    }

protected:
    void exit(int ret_code) {
        ret_code_ = ret_code;
        QThread::exit();
    }

private:
    int ret_code_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MaaWorker worker;
    QObject::connect(&worker, &QThread::finished, &app, [&worker]() {
        QApplication::exit(worker.ret_code());
    });
    worker.start();

    return app.exec();
}

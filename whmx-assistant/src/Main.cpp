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
#include <QtWidgets/QApplication>
#include <QtCore/QThread>
#include <QtCore/QElapsedTimer>

namespace fs = std::filesystem;

using namespace maa;

fs::path get_application_dir() {
    return fs::path(QApplication::applicationDirPath().toStdString());
}

auto get_project_dirs() {
    struct DirectorySet {
        const fs::path app_dir       = get_application_dir();
        const fs::path agent_dir     = app_dir / "agents";
        const fs::path res_dir       = app_dir / "assets" / "general";
        const fs::path res_data_dir  = app_dir / "assets" / "data";
        const fs::path anecdotes_dir = res_data_dir / "anecdotes.json";
    };

    return DirectorySet();
}

bool reload_anecdotes() {
    return Ref::ResearchAnecdoteSet::instance()->load(get_project_dirs().anecdotes_dir.string());
}

coro::Promise<int> routine(std::shared_ptr<Instance> instance) {
    const auto task_list = instance->resource()->task_list();

    QTextStream fin(stdin);
    QTextStream fout(stderr);

    bool should_quit = false;
    while (!should_quit) {
        qInfo() << "\n[$] Quit";
        qInfo() << "[r] Reload anecdotes";
        for (int i = 0; i < task_list->size(); ++i) {
            qInfo().noquote() << QString("[%1] %2").arg(i).arg(QString::fromUtf8(task_list->at(i)));
        }
        fout << ">>> select task: ";
        fout.flush();
        const auto input = fin.readLine().trimmed();
        if (input == "$") {
            should_quit = true;
            continue;
        }
        if (input.toLower() == "r") {
            if (const bool reloaded = reload_anecdotes()) {
                qInfo() << "[INF] anecdotes reloaded";
            } else {
                qWarning() << "[WRN] failed to reload anecdotes";
            }
            continue;
        }
        bool       ok      = false;
        const auto task_id = input.toInt(&ok);
        if (!ok) {
            qWarning() << "[WRN] invalid task id";
        } else if (task_id < 0 || task_id >= task_list->size()) {
            qWarning() << "[WRN] task id out of range";
        } else {
            const auto entry_task = task_list->at(task_id);
            const auto task_fut   = instance->post_task(entry_task);
            qInfo().nospace().noquote() << "[INF] running task: " << entry_task;
            QElapsedTimer timer;
            timer.start();
            co_await task_fut->wait();
            qInfo().nospace() << "[INF] cost time: " << timer.elapsed() << "ms";
        }
    }

    qDebug() << "[quited]";

    co_return 0;
}

coro::Promise<int> async_main() {
    const auto project = get_project_dirs();

    const std::string adb_hint = R"(MuMuPlayer\d+)";

    init(project.app_dir.string());

    if (const bool loaded = reload_anecdotes()) {
        const auto     anecdote_set = Ref::ResearchAnecdoteSet::instance();
        QList<QString> categories;
        for (const auto &category : anecdote_set->categories()) { categories << QString::fromUtf8(category); }
        qDebug().noquote() << "loaded anecdote categories: [" << categories.join(", ") << "]";
        for (const auto &category : anecdote_set->categories()) {
            QList<QString> entries;
            for (const auto &entry : anecdote_set->entry(category)->get().entry_names()) {
                entries << QString::fromUtf8(entry);
            }
            qDebug().noquote().nospace()
                << QString("loaded anecdote entries under %1: [ %2 ]").arg(QString::fromUtf8(category)).arg(entries.join(", "));
        }
    } else {
        qDebug("failed to load anecdotes set");
        co_return -1;
    }

    const auto device_resp = co_await find_adb_device(adb_hint);
    if (!device_resp.has_value()) {
        qDebug("failed to create adb controller");
        co_return -1;
    }

    auto controller = Controller::make(device_resp.value(), project.agent_dir.string())
                          ->set_long_side(1280)
                          ->set_short_side(720)
                          ->set_start_entry(Consts::ACTIVITY)
                          ->set_stop_entry(Consts::PACKAGE);
    auto ctrl_req_conn = controller->post_connect();

    auto resource     = Resource::make();
    auto res_req_path = resource->post_path(project.res_dir.string());

    co_await ctrl_req_conn->wait();
    co_await res_req_path->wait();

    auto instance = Instance::make()->bind(resource)->bind(controller);
    if (!instance->inited()) {
        qDebug("failed to init MAA");
        co_return -1;
    }

    instance->bind<Rec::Research::ParseGradeOptionsOnModify>();
    instance->bind<Rec::Research::ParseAnecdote>();
    instance->bind<Rec::Research::AnalyzeItemPairs>();
    instance->bind<Action::Research::SelectGradeOption>();
    instance->bind<Action::Research::ResolveAnecdote>();
    instance->bind<Action::Research::PerformItemPairsMatch>();

    //! TODO: check if the app is already started
    //! TODO: check if the app is in the foreground

    // co_await controller->post_start_app(activity)->wait();

    co_return co_await routine(instance);
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

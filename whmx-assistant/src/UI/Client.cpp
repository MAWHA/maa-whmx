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

#include "Client.h"
#include "Notification.h"
#include "TaskConfigPanel.h"
#include "../App.h"
#include "../Logger.h"
#include "../Rec/Research.h"
#include "../Rec/Utils.h"
#include "../Action/Research.h"
#include "../Action/FourInRow.h"
#include "../Action/Combat.h"
#include "../Consts.h"
#include "../ReferenceDataSet.h"

#include <MaaPP/MaaPP.hpp>
#include <QtCore/QDir>
#include <QtWidgets/QHBoxLayout>
#include <QtGui/QDesktopServices>
#include <magic_enum.hpp>
#include <chrono>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

using namespace maa;

namespace UI {

void Client::reload_anecdotes() {
    const auto anecdotes_path = data_dir() + "/anecdotes.json";
    auto       anecdote_set   = Ref::ResearchAnecdoteSet::instance();
    const auto old_hash       = anecdote_set->hash();
    if (const bool reloaded = anecdote_set->load(anecdotes_path.toStdString()); !reloaded) {
        LOG_WARN() << "failed to load anecdotes set";
        LOG_WARN(Workstation) << "加载研学奇遇事件集失败 [assets/data/anecdotes.json]";
    } else if (old_hash != anecdote_set->hash()) {
        QList<QString> categories;
        for (const auto &category : anecdote_set->categories()) { categories << QString::fromUtf8(category); }
        LOG_INFO().noquote() << "loaded anecdote categories: [" << categories.join(", ") << "]";
        for (const auto &category : anecdote_set->categories()) {
            QList<QString> entries;
            for (const auto &entry : anecdote_set->entry(category)->get().entry_names()) {
                entries << QString::fromUtf8(entry);
            }
            LOG_INFO().noquote().nospace()
                << QString("loaded anecdote entries under %1: [ %2 ]").arg(QString::fromUtf8(category)).arg(entries.join(", "));
        }
        LOG_INFO(Workstation) << "加载研学奇遇事件集";
        for (const auto &category : anecdote_set->categories()) {
            LOG_INFO(Workstation).noquote() << QString("• %1 已加载，找到 %2 条事件")
                                                   .arg(QString::fromUtf8(category))
                                                   .arg(anecdote_set->entry(category).value().get().entry_names().size());
        }
    }
}

void Client::reload_task_config() {
    const auto task_config_path = data_dir() + "/task_bindings.json";
    if (const bool loaded = Task::load_task_config(*task_config_, task_config_path, task_router_)) {
        LOG_INFO() << "task bindings reloaded";
    } else {
        LOG_ERROR() << "failed to load task bindings";
        LOG_ERROR(Workstation) << "任务绑定加载失败 [assets/data/task_bindings.json]";
    }
}

void Client::build_task_graph() {
    LOG_INFO() << "build task graph";

    QList<QDir> pipeline_dirs;
    pipeline_dirs.append(QDir(assets_dir() + "/pipeline"));
    for (int i = 0; i < pipeline_dirs.size(); ++i) {
        auto dir = pipeline_dirs[i];
        dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
        for (const auto &entry : dir.entryList()) { pipeline_dirs.append(QDir(dir.filePath(entry))); }
    }

    QStringList pipeline_files;
    for (const auto &dir : pipeline_dirs) {
        for (const auto &entry : dir.entryList({"*.json"})) { pipeline_files.append(dir.filePath(entry)); }
    }

    task_graph_->nodes.clear();

    QStringList failed_pipelines;
    for (const auto &file : pipeline_files) {
        if (const bool ok = task_graph_->merge_pipeline(file); !ok) { failed_pipelines << file; }
    }

    LOG_INFO() << "build task graph done: merge" << pipeline_files.size() << "pipelines in total," << failed_pipelines.size()
               << "pipelines failed";
    if (!failed_pipelines.empty()) { LOG_INFO() << "failed to load pipelines:\n" << failed_pipelines.join("\n"); }
}

void Client::setup_runtime_log() {
    const auto datetime      = QDateTime::currentDateTime();
    const auto log_path      = QString("%1/runtime.%2.log").arg(log_dir()).arg(datetime.toString("yyyyMMddHH"));
    latest_runtime_log_file_ = log_path;

    auto runtime_logger = spdlog::basic_logger_mt("whmx-assistant.default", log_path.toLocal8Bit().toStdString());
    runtime_logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] %v");

    //! TODO: enable level config
    runtime_logger->set_level(spdlog::level::trace);

    //! FIXME: has global effect
    spdlog::set_default_logger(runtime_logger);
    spdlog::flush_every(std::chrono::minutes(1));
    spdlog::flush_on(spdlog::level::err);

    //! hello message
    LOG_INFO().noquote() << QString().fill('=', 16);
    LOG_INFO().noquote() << "应用名称：" << QApplication::applicationDisplayName();
    LOG_INFO().noquote() << "当前版本：" << QApplication::applicationVersion();
    LOG_INFO().noquote() << "启动时间：" << datetime.toString("yyyy-MM-dd HH:mm:ss");
    LOG_INFO().noquote() << QString().fill('=', 16);
    runtime_logger->flush();
}

void Client::config_maa() {
    maa::init(user_dir().toLocal8Bit().toStdString());
    maa_res_ = Resource::make();
    handle_on_reload_assets();
}

void Client::create_task_config_panel(Task::MajorTask task) {
    const auto task_info = Task::get_task_info(task);
    if (!task_info.has_config) {
        LOG_ERROR() << "try to open config panel for task" << magic_enum::enum_name(task) << "which has no config";
        return;
    }

    if (auto &params = task_config_->task_params; !params.contains(task)) {
        params.insert(task, Task::get_default_task_param(task));
    }

    auto       panel = TaskConfigPanel::build(task, task_config_->task_params.value(task));
    const auto title = QString("核心任务配置·%1").arg(task_info.name);

    connect(panel, &TaskConfigPanel::on_config_change, [this](Task::MajorTask task, QVariant config) {
        task_config_->task_params[task] = config;
    });

    emit gApp->app_event()->on_open_task_config_panel(title, panel);
}

void Client::handle_on_reload_assets() {
    reload_anecdotes();
    reload_task_config();
    build_task_graph();

    if (maa_res_ && (!fut_res_req_path_.state_->task_.has_value() || fut_res_req_path_.fulfilled())) {
        fut_res_req_path_ = coro::EventLoop::current()->eval([this, assets_dir = assets_dir().toStdString()] {
            LOG_INFO().noquote() << "sync res dir:" << assets_dir;
            emit on_sync_res_dir_done(maa_res_->post_path(assets_dir)->wait().sync_wait());
        });
    }
}

void Client::handle_on_open_log_file(LogFileType type) {
    QMap<LogFileType, QString> LOG_FILE_TABLE{
        {LogFileType::MaaFramework, "maa.log"               },
        {LogFileType::Application,  latest_runtime_log_file_},
    };

    const auto log_file = LOG_FILE_TABLE.value(type);
    Q_ASSERT(!log_file.isEmpty());

    const QDir log_dir(this->log_dir());
    if (!log_dir.exists(log_file)) {
        Notification::info(gApp->window(), "打开日志", "日志文件暂未建立，请稍后再试");
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(log_dir.filePath(log_file)));
}

void Client::handle_on_sync_res_dir_done(int maa_status) {
    if (maa_status != MaaStatus_Success) {
        LOG_ERROR() << "failed to sync res dir";
        LOG_ERROR(Workstation) << "MAA 资源加载失败，请检查本地资源完整性 [assets/general]";
    } else {
        LOG_INFO() << "sync res dir successfully";
        LOG_INFO().noquote() << "found" << maa_res_->task_list()->size() << "tasks under" << assets_dir();
        QStringList major_tasks_bindings(task_config_->task_entries.values());
        emit gApp->app_event()->workbench_on_reload_pipeline_tasks(task_graph_->find_left_root_tasks(major_tasks_bindings));
    }
}

void Client::execute_pipeline_task(QString task_id, QVariant task) {
    Q_ASSERT(maa_instance_ && maa_instance_->inited());

    emit gApp->app_event()->workbench_on_notify_queued_task_accepted(task_id);

    if (const QString type_name = task.typeName(); type_name == "Task::MajorTask") {
        execute_major_task(task_id, task.value<Task::MajorTask>());
    } else if (type_name == "QString") {
        execute_custom_task(task_id, task.value<QString>());
    } else {
        std::unreachable();
    }
}

void Client::execute_major_task(const QString &task_id, Task::MajorTask task) {
    const auto major_task_name = Task::get_task_info(task).name;
    if (task_config_->task_entries.contains(task)) {
        //! TODO: pass major task params
        const auto task_entry = task_config_->task_entries.value(task);
        coro::EventLoop::current()->eval([this, task, task_id, task_entry, major_task_name] {
            LOG_INFO(Workstation).noquote() << QString("启动核心任务 %1 | 目标任务 %2").arg(major_task_name).arg(task_entry);
            auto flag = std::make_shared<std::atomic_bool>(false);
            QTimer::singleShot(6e+4 * 5, [expected = flag->load(), flag] {
                if (expected == *flag) { LOG_WARN(Workstation).noquote() << "超时预警，请检查任务是否进入死循环"; }
            });
            const auto status = instance()->post_task(task_entry.toUtf8().toStdString())->wait().sync_wait();
            flag->store(true);
            emit gApp->app_event()->workbench_on_notify_queued_task_finished(task_id, status);
        });
    } else if (task_router_->contains_route_of(task)) {
        coro::EventLoop::current()->eval([this, task, task_id, route = task_router_->route(task), major_task_name] {
            int status = MaaStatus_Invalid;
            do {
                const bool started = route->start();
                if (started) {
                    LOG_INFO(Workstation).noquote()
                        << QString("启动核心任务 %1 | 共 %2 步").arg(major_task_name).arg(route->total_stages());
                } else {
                    LOG_INFO(Workstation).noquote() << QString("启动核心任务 %1").arg(major_task_name);
                    LOG_ERROR(Workstation) << "未找到命中路径";
                    break;
                }
                LOG_TRACE().noquote() << "start route of" << task_id;
                auto step_index = std::make_shared<int>(0);
                while (route->has_next()) {
                    // if (ui_workbench_->pipeline_state().is_idle()) {
                    //     status = MaaStatus_Success;
                    //     break;
                    // }
                    const auto opt_task = route->next();
                    if (!opt_task.has_value()) {
                        status = MaaStatus_Failed;
                        break;
                    }
                    //! NOTE: not exactly correspoding to the real pipeline stage
                    ++*step_index;
                    const auto task       = opt_task.value();
                    const auto task_entry = task.task_entry.toUtf8().toStdString();
                    LOG_TRACE().noquote() << "execute task" << task_entry << "with params"
                                          << QString::fromUtf8(task.params.to_string());
                    LOG_INFO(Workstation).noquote()
                        << QString("• 步骤 %1 - %2").arg(route->next_stage()).arg(QString::fromUtf8(task_entry));
                    QTimer::singleShot(6e+4 * 5, [expected = *step_index, step_index] {
                        if (expected == *step_index) {
                            LOG_WARN(Workstation).noquote() << "超时预警，请检查任务是否进入死循环";
                        }
                    });
                    const int task_status = instance()->post_task(task_entry, task.params)->wait().sync_wait();
                    if (task_status != MaaStatus_Success) {
                        status = task_status;
                        break;
                    }
                }
            } while (0);
            if (status == MaaStatus_Invalid) { status = MaaStatus_Success; }
            emit gApp->app_event()->workbench_on_notify_queued_task_finished(task_id, status);
        });
    } else {
        LOG_WARN().noquote() << "failed to execute task" << task_id << ": task entry not found for major task"
                             << magic_enum::enum_name(task);
        LOG_WARN(Workstation).noquote() << QString("未找到 %1 的任务绑定，已跳过").arg(major_task_name);
        emit gApp->app_event()->workbench_on_notify_queued_task_finished(task_id, MaaStatus_Invalid);
    }
}

void Client::execute_custom_task(const QString &task_id, const QString &task_name) {
    coro::EventLoop::current()->eval([this, task_id, task_entry = task_name] {
        LOG_INFO(Workstation).noquote() << QString("执行任务 %1").arg(task_entry);
        auto flag = std::make_shared<std::atomic_bool>(false);
        QTimer::singleShot(6e+4 * 5, [expected = flag->load(), flag] {
            if (expected == *flag) { LOG_WARN(Workstation).noquote() << "超时预警，请检查任务是否进入死循环"; }
        });
        const auto status = instance()->post_task(task_entry.toUtf8().toStdString())->wait().sync_wait();
        flag->store(true);
        emit gApp->app_event()->workbench_on_notify_queued_task_finished(task_id, status);
    });
}

void Client::handle_on_request_connect_device(MaaAdbDevice device) {
    if (maa_ctrl_ && maa_instance_ && maa_instance_->inited()) {
        Notification::warning(gApp->window(), "设备连接", "不支持多设备连接，已取消该次连接请求");
        emit on_request_connect_device_done(MaaStatus_Failed);
    } else if (maa_ctrl_) {
        LOG_INFO() << "device alreay connected, retry init maa instance";
        QMetaObject::invokeMethod(this, "handle_on_create_and_init_instance", Qt::AutoConnection);
        emit on_request_connect_device_done(MaaStatus_Failed);
    } else {
        AdbDevice adb_device{
            .name     = device.name.toStdString(),
            .adb_path = device.path.toStdString(),
            .address  = device.address.toStdString(),
            .type     = device.type,
            .config   = device.config.toStdString(),
        };
        maa_ctrl_ = Controller::make(adb_device, agents_dir().toStdString())
                        ->set_long_side(1280)
                        ->set_short_side(720)
                        ->set_start_entry(Consts::ACTIVITY)
                        ->set_stop_entry(Consts::PACKAGE);
        fut_ctrl_req_conn_ = coro::EventLoop::current()->eval([this] {
            const int maa_status = maa_ctrl_->post_connect()->wait().sync_wait();
            emit on_request_connect_device_done(maa_ctrl_->post_connect()->wait().sync_wait());
            if (maa_status != MaaStatus_Success) {
                maa_ctrl_.reset();
            } else {
                QMetaObject::invokeMethod(this, "handle_on_create_and_init_instance", Qt::AutoConnection);
            }
        });
    }
}

void Client::handle_on_create_and_init_instance() {
    if (maa_instance_ && maa_instance_->inited()) { return; }
    //! FIXME: ensure maa_res_ is initialized
    maa_instance_ = Instance::make()->bind(maa_res_)->bind(maa_ctrl_);
    if (!maa_instance_->inited()) {
        maa_instance_.reset();
        LOG_ERROR() << "failed to init maa, view log file for details or check your assets integrity";
        return;
    }
    maa_instance_->bind<Rec::Utils::TwoStageTest>();
    maa_instance_->bind<Rec::Research::ParseGradeOptionsOnModify>();
    maa_instance_->bind<Rec::Research::ParseAnecdote>();
    maa_instance_->bind<Rec::Research::AnalyzeItemPairs>();
    maa_instance_->bind<Rec::Research::GetCandidateBuffs>();
    maa_instance_->bind<Action::Research::SelectGradeOption>();
    maa_instance_->bind<Action::Research::ResolveAnecdote>();
    maa_instance_->bind<Action::Research::PerformItemPairsMatch>();
    maa_instance_->bind<Action::Research::ResolveBuffSelection>();
    maa_instance_->bind<Action::SolveFourInRow>();
    maa_instance_->bind<Action::Combat::FillSquad>();
    LOG_INFO() << "maa instance created and initialized";
    emit gApp->app_event()->workbench_on_accept_maa_instance(maa_instance_);
}

Client::Client(const QString &user_path, QObject *parent)
    : QObject(parent)
    , user_path_(QDir::isAbsolutePath(user_path) ? user_path : app_dir() + "/" + user_path)
    , task_config_(std::make_shared<Task::Config>())
    , task_graph_(std::make_shared<Task::TaskGraph>())
    , task_router_(std::make_shared<Task::Router>(task_config_, task_graph_)) {
    setup();
    config_maa();
    setup_runtime_log();
    Task::reset_shared_task_config(task_config_);
}

Client::~Client() {
    //! TODO: dump user config
}

void Client::setup() {
    const auto event = gApp->app_event();

    connect(this, &Client::on_sync_res_dir_done, this, &Client::handle_on_sync_res_dir_done);
    connect(this, &Client::on_request_connect_device_done, event, &AppEvent::device_conn_on_request_connect_device_done);
    connect(event, &AppEvent::client_on_request_connect_device, this, &Client::handle_on_request_connect_device);
    connect(event, &AppEvent::workbench_on_reload_assets, this, &Client::handle_on_reload_assets);
    connect(event, &AppEvent::workbench_on_open_maa_log_file, this, &Client::handle_on_open_maa_log_file);
    connect(event, &AppEvent::workbench_on_open_app_log_file, this, &Client::handle_on_open_app_log_file);
    connect(event, &AppEvent::workbench_on_post_queued_task, this, &Client::execute_pipeline_task);
    connect(event, &AppEvent::workbench_on_request_open_task_config_panel, this, &Client::create_task_config_panel);
}

} // namespace UI

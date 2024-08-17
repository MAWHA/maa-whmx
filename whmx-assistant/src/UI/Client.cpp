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
#include "TaskConfigPanel.h"
#include "Notification.h"
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

using namespace maa;

namespace UI {

void Client::reload_anecdotes() {
    const auto anecdotes_path = data_dir() + "/anecdotes.json";
    auto       anecdote_set   = Ref::ResearchAnecdoteSet::instance();
    if (const bool loaded = anecdote_set->load(anecdotes_path.toStdString())) {
        QList<QString> categories;
        for (const auto &category : anecdote_set->categories()) { categories << QString::fromUtf8(category); }
        qInfo().noquote() << "loaded anecdote categories: [" << categories.join(", ") << "]";
        for (const auto &category : anecdote_set->categories()) {
            QList<QString> entries;
            for (const auto &entry : anecdote_set->entry(category)->get().entry_names()) {
                entries << QString::fromUtf8(entry);
            }
            qInfo().noquote().nospace()
                << QString("loaded anecdote entries under %1: [ %2 ]").arg(QString::fromUtf8(category)).arg(entries.join(", "));
        }
    } else {
        qWarning() << "failed to load anecdotes set";
    }
}

void Client::reload_task_config() {
    const auto task_config_path = data_dir() + "/task_bindings.json";
    if (const bool loaded = Task::load_task_config(*task_config_, task_config_path, task_router_)) {
        qInfo() << "task bindings reloaded";
    } else {
        qInfo() << "failed to load task bindings";
    }
}

void Client::build_task_graph() {
    qInfo() << "build task graph";

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

    qInfo() << "build task graph done: merge" << pipeline_files.size() << "pipelines in total," << failed_pipelines.size()
            << "pipelines failed";
    if (!failed_pipelines.empty()) { qInfo() << "failed to load pipelines:\n" << failed_pipelines.join("\n"); }
}

void Client::config_maa(const QString &user_path) {
    user_path_ = QDir::isAbsolutePath(user_path) ? user_path : app_dir() + "/" + user_path;
    maa::init(user_dir().toLocal8Bit().toStdString());
    maa_res_ = Resource::make();
    handle_on_reload_assets();
}

void Client::open_task_config_panel(Task::MajorTask task) {
    const auto task_info = Task::get_task_info(task);
    if (!task_info.has_config) {
        qCritical() << "try to open config panel for task" << magic_enum::enum_name(task) << "which has no config";
        return;
    }

    if (auto &params = task_config_->task_params; !params.contains(task)) {
        params.insert(task, Task::get_default_task_param(task));
    }

    auto panel = TaskConfigPanel::build(task, task_config_->task_params.value(task));
    addTab(panel, QString("核心任务配置·%1").arg(task_info.name));
    setCurrentIndex(count() - 1);

    connect(panel, &TaskConfigPanel::on_config_change, [this](Task::MajorTask task, QVariant config) {
        task_config_->task_params[task] = config;
    });
}

void Client::handle_on_reload_assets() {
    reload_anecdotes();
    reload_task_config();
    build_task_graph();

    if (maa_res_ && (!fut_res_req_path_.state_->task_.has_value() || fut_res_req_path_.fulfilled())) {
        fut_res_req_path_ = coro::EventLoop::current()->eval([this, assets_dir = assets_dir().toStdString()] {
            qInfo().noquote() << "sync res dir:" << assets_dir;
            emit on_sync_res_dir_done(maa_res_->post_path(assets_dir)->wait().sync_wait());
        });
    }
}

void Client::handle_on_open_log_file(LogFileType type) {
    static QMap<LogFileType, QString> LOG_FILE_TABLE{
        {LogFileType::MaaFramework, "maa.log"    },
        {LogFileType::Application,  "runtime.log"},
    };

    const auto log_file = LOG_FILE_TABLE.value(type);
    Q_ASSERT(!log_file.isEmpty());

    const QDir log_dir(this->log_dir());
    if (!log_dir.exists(log_file)) {
        Notification::info(this, "打开日志", "日志文件暂未建立，请稍后再试");
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(log_dir.filePath(log_file)));
}

void Client::handle_on_sync_res_dir_done(int maa_status) {
    if (maa_status != MaaStatus_Success) {
        qWarning() << "failed to sync res dir";
    } else {
        qInfo() << "sync res dir successfully";
        qInfo().noquote() << "found" << maa_res_->task_list()->size() << "tasks under" << assets_dir();
        QStringList major_tasks_bindings(task_config_->task_entries.values());
        ui_workbench_->reload_pipeline_tasks(task_graph_->find_left_root_tasks(major_tasks_bindings));
    }
}

void Client::execute_pipeline_task(QString task_id, QVariant task) {
    Q_ASSERT(maa_instance_ && maa_instance_->inited());

    ui_workbench_->notify_queued_task_accepted(task_id);

    if (const QString type_name = task.typeName(); type_name == "Task::MajorTask") {
        execute_major_task(task_id, task.value<Task::MajorTask>());
    } else if (type_name == "QString") {
        execute_custom_task(task_id, task.value<QString>());
    } else {
        std::unreachable();
    }
}

void Client::execute_major_task(const QString &task_id, Task::MajorTask task) {
    if (task_config_->task_entries.contains(task)) {
        //! TODO: pass major task params
        const auto task_entry = task_config_->task_entries.value(task);
        coro::EventLoop::current()->eval([this, task_id, task_entry] {
            const auto status = instance()->post_task(task_entry.toUtf8().toStdString())->wait().sync_wait();
            QMetaObject::invokeMethod(
                ui_workbench_, "notify_queued_task_finished", Qt::AutoConnection, Q_ARG(QString, task_id), Q_ARG(int, status));
        });
    } else if (task_router_->contains_route_of(task)) {
        coro::EventLoop::current()->eval([this, task_id, route = task_router_->route(task)] {
            int status = MaaStatus_Invalid;
            do {
                const bool started = route->start();
                if (!started) { break; }
                qDebug().noquote() << "start route of" << task_id;
                while (route->has_next()) {
                    if (ui_workbench_->pipeline_state().is_idle()) {
                        status = MaaStatus_Success;
                        break;
                    }
                    const auto opt_task = route->next();
                    if (!opt_task.has_value()) {
                        status = MaaStatus_Failed;
                        break;
                    }
                    const auto task       = opt_task.value();
                    const auto task_entry = task.task_entry.toUtf8().toStdString();
                    qDebug().noquote() << "execute task" << task_entry << "with params"
                                       << QString::fromUtf8(task.params.to_string());
                    const int task_status = instance()->post_task(task_entry, task.params)->wait().sync_wait();
                    if (task_status != MaaStatus_Success) {
                        status = task_status;
                        break;
                    }
                }
            } while (0);
            if (status == MaaStatus_Invalid) { status = MaaStatus_Success; }
            QMetaObject::invokeMethod(
                ui_workbench_, "notify_queued_task_finished", Qt::AutoConnection, Q_ARG(QString, task_id), Q_ARG(int, status));
        });
    } else {
        qWarning().noquote() << "failed to execute task" << task_id << ": task entry not found for major task"
                             << magic_enum::enum_name(task);
        ui_workbench_->notify_queued_task_finished(task_id, MaaStatus_Invalid);
    }
}

void Client::execute_custom_task(const QString &task_id, const QString &task_name) {
    coro::EventLoop::current()->eval([this, task_id, task_entry = task_name] {
        const auto status = instance()->post_task(task_entry.toUtf8().toStdString())->wait().sync_wait();
        QMetaObject::invokeMethod(
            ui_workbench_, "notify_queued_task_finished", Qt::AutoConnection, Q_ARG(QString, task_id), Q_ARG(int, status));
    });
}

void Client::handle_on_request_connect_device(MaaAdbDevice device) {
    if (maa_ctrl_ && maa_instance_ && maa_instance_->inited()) {
        Notification::warning(this, "设备连接", "不支持多设备连接，已取消该次连接请求");
        emit on_request_connect_device_done(MaaStatus_Failed);
    } else if (maa_ctrl_) {
        qInfo() << "device alreay connected, retry init maa instance";
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
        qCritical() << "failed to init maa, view log file for details or check your assets integrity";
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
    qInfo() << "maa instance created and initialized";
    ui_workbench_->accept_maa_instance(maa_instance_);
}

void Client::handle_on_logging_flush(QString loggings) {
    const auto log_file_path = log_dir() + "/runtime.log";

    QFile file(log_file_path);
    file.open(QIODevice::WriteOnly | QIODevice::Append);

    if (first_time_to_flush_log_) {
        const auto head = QString("%1\n应用名称: %2\n当前版本: %3\n启动时间: %4\n%5\n")
                              .arg(QString().fill('=', 16))
                              .arg(QApplication::applicationDisplayName())
                              .arg(QApplication::applicationVersion())
                              .arg(startup_time_.toString("yyyy-MM-dd HH:mm:ss"))
                              .arg(QString().fill('=', 16));
        file.write(head.toUtf8());
        first_time_to_flush_log_ = false;
    }

    file.write(loggings.toUtf8());
}

void Client::handle_on_leave_task_config_panel() {
    const int confg_panel_index = count() - 1;
    Q_ASSERT(tabText(confg_panel_index).startsWith("核心任务配置"));
    auto panel = widget(confg_panel_index);
    removeTab(confg_panel_index);
    panel->deleteLater();
}

void Client::handle_on_switch_tab(int index) {
    const int last_tab = count() - 1;
    if (index != last_tab && tabText(last_tab).startsWith("核心任务配置")) { handle_on_leave_task_config_panel(); }
}

Client::Client(const QString &user_path, QWidget *parent)
    : QTabWidget(parent)
    , startup_time_(QDateTime::currentDateTime())
    , first_time_to_flush_log_(true)
    , task_config_(std::make_shared<Task::Config>())
    , task_graph_(std::make_shared<Task::TaskGraph>())
    , task_router_(std::make_shared<Task::Router>(task_config_, task_graph_)) {
    setup();
    config_maa(user_path);
    Task::reset_shared_task_config(task_config_);
}

Client::~Client() {
    //! TODO: dump user config

    //! ATTENTION: do not use logger->flush() here, the client is already destroyed here
    handle_on_logging_flush(ui_workbench_->logger()->take());
}

void Client::setup() {
    //! ATTENTION: ensure the logger panel ctor first to redirect all logging messages to it
    ui_workbench_   = new Workbench;
    ui_device_conn_ = new DeviceConn;
    ui_settings_    = new Settings;

    addTab(ui_device_conn_, "设备连接");
    addTab(ui_workbench_, "工作台");
    addTab(ui_settings_, "设置");

    setCurrentIndex(1);

    setMinimumSize(600, 400);
    setContentsMargins({});
    {
        auto pal = palette();
        pal.setColor(backgroundRole(), Qt::white);
        setPalette(pal);
    }

    connect(ui_device_conn_, &DeviceConn::on_request_connect_device, this, &Client::handle_on_request_connect_device);
    connect(this, &Client::on_request_connect_device_done, ui_device_conn_, &DeviceConn::handle_on_request_connect_device_done);
    connect(ui_workbench_, &Workbench::on_reload_assets, this, &Client::handle_on_reload_assets);
    connect(ui_workbench_, &Workbench::on_open_maa_log_file, this, &Client::handle_on_open_maa_log_file);
    connect(ui_workbench_, &Workbench::on_open_app_log_file, this, &Client::handle_on_open_app_log_file);
    connect(this, &Client::on_sync_res_dir_done, this, &Client::handle_on_sync_res_dir_done);
    connect(ui_workbench_->logger(), &LogPanel::on_flush, this, &Client::handle_on_logging_flush);
    connect(ui_workbench_, &Workbench::on_post_queued_task, this, &Client::execute_pipeline_task);
    connect(ui_workbench_, &Workbench::on_request_open_task_config_panel, this, &Client::open_task_config_panel);
    connect(this, &Client::currentChanged, this, &Client::handle_on_switch_tab);
}

} // namespace UI

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

#pragma once

#include "DeviceConn.h"
#include "Workbench.h"
#include "Settings.h"
#include "../Task/TaskGraph.h"
#include "../Task/Config.h"
#include "../Task/Router.h"

#include <MaaPP/MaaPP.hpp>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QApplication>
#include <QtCore/QDateTime>

namespace UI {

class Client : public QTabWidget {
    Q_OBJECT

public:
    enum class LogFileType {
        MaaFramework,
        Application,
    };

public:
    static QString app_dir() {
        return QApplication::applicationDirPath();
    }

    QString user_dir() const {
        return user_path_;
    }

    QString log_dir() const {
        return user_dir() + "/debug";
    }

    QString config_dir() const {
        return user_dir() + "/config";
    }

    QString assets_dir() const {
        return user_dir() + "/assets/general";
    }

    QString data_dir() const {
        return user_dir() + "/assets/data";
    }

    QString agents_dir() const {
        return user_dir() + "/agents";
    }

    std::shared_ptr<maa::Controller> controller() const {
        return maa_ctrl_;
    }

    std::shared_ptr<maa::Resource> resource() const {
        return maa_res_;
    }

    std::shared_ptr<maa::Instance> instance() const {
        return maa_instance_;
    }

    void reload_anecdotes();
    void reload_task_config();
    void build_task_graph();

protected:
    void setup_runtime_log();
    void config_maa();

public slots:
    void open_task_config_panel(Task::MajorTask task);

    void handle_on_reload_assets();
    void handle_on_open_log_file(LogFileType type);

    void handle_on_open_maa_log_file() {
        handle_on_open_log_file(LogFileType::MaaFramework);
    }

    void handle_on_open_app_log_file() {
        handle_on_open_log_file(LogFileType::Application);
    }

protected slots:
    void execute_pipeline_task(QString task_id, QVariant task);
    void execute_major_task(const QString &task_id, Task::MajorTask task);
    void execute_custom_task(const QString &task_id, const QString &task_name);

    void handle_on_sync_res_dir_done(int maa_status);
    void handle_on_request_connect_device(MaaAdbDevice device);
    void handle_on_create_and_init_instance();
    void handle_on_leave_task_config_panel();
    void handle_on_switch_tab(int index);

signals:
    void on_sync_res_dir_done(int maa_status);
    void on_request_connect_device_done(int maa_status);

public:
    Client(const QString &user_path = ".", QWidget *parent = nullptr);
    ~Client() override;

protected:
    void setup();

private:
    QString                          latest_runtime_log_file_;
    QString                          user_path_;
    std::shared_ptr<Task::Config>    task_config_;
    std::shared_ptr<Task::TaskGraph> task_graph_;
    std::shared_ptr<Task::Router>    task_router_;
    std::shared_ptr<maa::Controller> maa_ctrl_;
    std::shared_ptr<maa::Resource>   maa_res_;
    std::shared_ptr<maa::Instance>   maa_instance_;
    maa::coro::Promise<void>         fut_res_req_path_;
    maa::coro::Promise<void>         fut_ctrl_req_conn_;
    DeviceConn                      *ui_device_conn_ = nullptr;
    Workbench                       *ui_workbench_   = nullptr;
    Settings                        *ui_settings_    = nullptr;
};

} // namespace UI

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

#include "Task/MajorTask.h"
#include "UI/DeviceConn.h"

#include <MaaPP/MaaPP.hpp>
#include <QObject>

class AppEvent : public QObject {
    Q_OBJECT

signals:
    void on_set_window_top(bool on);
    void on_minimize();
    void on_maximize();
    void on_restore();
    void on_close();
    void on_exit();

    void on_open_task_config_panel(QString title, QWidget *panel);

    void device_conn_on_request_connect_device_done(int maa_status);
    void workbench_on_reload_assets();
    void workbench_on_open_maa_log_file();
    void workbench_on_open_app_log_file();
    void workbench_on_post_queued_task(QString task_id, QVariant task);
    void workbench_on_request_open_task_config_panel(Task::MajorTask task);
    void workbench_on_accept_maa_instance(std::shared_ptr<maa::Instance> instance);
    void workbench_on_notify_queued_task_accepted(QString task_id);
    void workbench_on_notify_queued_task_finished(QString task_id, int status);
    void workbench_on_reload_pipeline_tasks(const QStringList &custom_tasks);
    void client_on_request_connect_device(MaaAdbDevice device);

public:
    AppEvent(QObject *parent = nullptr)
        : QObject(parent) {}
};

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

#include "FlatButton.h"
#include "LogPanel.h"
#include "QueuedTaskItem.h"
#include "../Task/MajorTask.h"

#include <MaaPP/MaaPP.hpp>
#include <QtWidgets/QWidget>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QListWidget>
#include <qtmaterialtextfield.h>
#include <functional>
#include <optional>

namespace UI {

enum PipelineState {
    PipelineState_Idle,
    PipelineState_Running,
    PipelineState_PendingPause,
    PipelineState_Paused,
};

class Workbench : public QWidget {
    Q_OBJECT

protected:
    friend class Client;

public:
    LogPanel *logger() const {
        return ui_log_panel_;
    }

    auto pipeline_state() const {
        class Wrapper {
        public:
            Wrapper(PipelineState state)
                : state_(state) {}

            operator int() const {
                return state_;
            }

            int state() const {
                return state_;
            }

            bool is_idle() const {
                return state_ == PipelineState_Idle;
            }

            bool is_running() const {
                return state_ == PipelineState_Running;
            }

            bool is_paused() const {
                return state_ == PipelineState_Paused;
            }

            bool is_pending_pause() const {
                return state_ == PipelineState_PendingPause;
            }

        private:
            PipelineState state_;
        };

        return Wrapper{pipeline_state_};
    }

protected:
    void    pop_first_queued_task();
    void    clear_queued_tasks();
    void    accept_maa_instance(std::shared_ptr<maa::Instance> instance);
    void    set_list_item_widget_checked(QListWidgetItem *item, bool on);
    QString get_list_item_text(QListWidgetItem *item);
    void    connect_queued_task_item_signal_slot(QListWidgetItem *item);

public slots:
    void reload_pipeline_tasks(const QStringList &custom_tasks);
    void push_selected_tasks_to_queue();
    void notify_queued_task_accepted(const QString &task_id);
    void notify_queued_task_finished(const QString &task_id, MaaStatus status);
    void start_pipeline();
    void pause_pipeline();
    void stop_pipeline();

protected slots:
    void pause_pipeline(std::optional<std::function<void()>> opt_on_paused);
    void resume_pipeline();
    void post_next_queued_task();

    void handle_on_pending_pipeline_pause_done();
    void handle_on_cancel_posted_queued_task(const QString &task_id);
    void handle_on_task_list_filter_changed(QListWidget *list, const QString &filter);
    void handle_on_toggle_task_list_select_all(QListWidget *list, bool on);
    void handle_on_execute_queued_task_action(QListWidgetItem *item, QueuedTaskItem::ActionType action);

    void handle_on_major_task_filter_changed(const QString &filter) {
        handle_on_task_list_filter_changed(ui_major_task_list_, filter);
    }

    void handle_on_custom_task_filter_changed(const QString &filter) {
        handle_on_task_list_filter_changed(ui_custom_task_list_, filter);
    }

    void handle_on_toggle_major_tasks_select_all(bool on) {
        handle_on_toggle_task_list_select_all(ui_major_task_list_, on);
    }

    void handle_on_toggle_custom_tasks_select_all(bool on) {
        handle_on_toggle_task_list_select_all(ui_custom_task_list_, on);
    }

signals:
    void on_reload_assets();
    void on_push_tasks();
    void on_start_pipeline();
    void on_pause_pipeline();
    void on_stop_pipeline();
    void on_open_maa_log_file();
    void on_open_app_log_file();
    void on_post_queued_task(QString task_id, QVariant task);
    void on_request_open_task_config_panel(Task::MajorTask task);

public:
    Workbench(QWidget *parent = nullptr);
    ~Workbench() override;

protected:
    void setup();

private:
    QMap<QString, bool>            pending_task_accepted_;
    PipelineState                  pipeline_state_;
    maa::coro::Promise<void>       fut_pending_pause_;
    std::shared_ptr<maa::Instance> instance_;
    QCheckBox                     *ui_major_tasks_select_all_  = nullptr;
    QListWidget                   *ui_major_task_list_         = nullptr;
    QtMaterialTextField           *ui_major_task_filter_       = nullptr;
    QCheckBox                     *ui_custom_tasks_select_all_ = nullptr;
    QListWidget                   *ui_custom_task_list_        = nullptr;
    QtMaterialTextField           *ui_custom_task_filter_      = nullptr;
    QListWidget                   *ui_task_queue_              = nullptr;
    FlatButton                    *ui_reload_custom_tasks_     = nullptr;
    FlatButton                    *ui_join_tasks_              = nullptr;
    FlatButton                    *ui_start_pipeline_          = nullptr;
    FlatButton                    *ui_pause_pipeline_          = nullptr;
    LogPanel                      *ui_log_panel_               = nullptr;
    FlatButton                    *ui_open_maa_log_file_       = nullptr;
    FlatButton                    *ui_open_app_log_file_       = nullptr;
};

} // namespace UI

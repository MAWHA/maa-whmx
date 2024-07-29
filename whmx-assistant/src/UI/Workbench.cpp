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

#include "Workbench.h"
#include "TaskItem.h"
#include "CheckableItem.h"
#include "Helper.h"
#include "../Task/MajorTask.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMessageBox>
#include <QtCore/QUuid>
#include <QtCore/QTimer>
#include <magic_enum.hpp>
#include <qtmaterialscrollbar.h>

using namespace maa;

namespace UI {

void Workbench::pop_first_queued_task() {
    ui_task_queue_->takeItem(0);
}

void Workbench::clear_queued_tasks() {
    ui_task_queue_->clear();
}

void Workbench::accept_maa_instance(std::shared_ptr<maa::Instance> instance) {
    if (!instance || !instance->inited()) {
        qCritical() << "accepted invalid maa instance";
        return;
    }
    if (instance_) { qInfo() << "override current exisiting maa instance"; }
    instance_ = instance;
    qInfo() << "workbench: accepted maa instance";
}

void Workbench::set_list_item_widget_checked(QListWidgetItem *item, bool on) {
    const auto list_widget = item->listWidget();
    const auto item_widget = list_widget->itemWidget(item);
    if (list_widget == ui_major_task_list_) {
        qobject_cast<TaskItem *>(item_widget)->set_checked(on);
    } else if (list_widget == ui_custom_task_list_) {
        qobject_cast<CheckableItem *>(item_widget)->set_checked(on);
    } else {
        std::unreachable();
    }
}

QString Workbench::get_list_item_text(QListWidgetItem *item) {
    const auto list_widget = item->listWidget();
    const auto item_widget = list_widget->itemWidget(item);
    if (list_widget == ui_major_task_list_) {
        return Task::get_task_info(qobject_cast<TaskItem *>(item_widget)->task()).name;
    } else if (list_widget == ui_custom_task_list_) {
        return qobject_cast<CheckableItem *>(item_widget)->name();
    } else {
        std::unreachable();
    }
}

void Workbench::connect_queued_task_item_signal_slot(QListWidgetItem *item) {
    const auto list_widget = item->listWidget();
    const auto item_widget = qobject_cast<QueuedTaskItem *>(list_widget->itemWidget(item));
    connect(item_widget, &QueuedTaskItem::on_request_action, this, [this, item](QueuedTaskItem::ActionType action) {
        handle_on_execute_queued_task_action(item, action);
    });
}

void Workbench::reload_pipeline_tasks(const QStringList &custom_tasks) {
    qInfo() << "reload pipeline tasks";

    if (const auto state = pipeline_state(); state.is_running()) {
        qWarning() << "reject reload tasks due to pipeline is running";
        QMessageBox::warning(this, "重载任务", "流水线运行中，请先中止流水线运行。");
        return;
    } else if (state.is_paused()) {
        const auto option = QMessageBox::information(
            this,
            "重载任务",
            "流水线暂停中，是否中止流水线并清空任务队列以进行任务重载？",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (option == QMessageBox::No) { return; }
        stop_pipeline();
        clear_queued_tasks();
    }

    //! NOTE: keep the state of unchanged tasks

    QSet<QString> tasks(custom_tasks.begin(), custom_tasks.end());
    {
        int index = 0;
        while (index < ui_custom_task_list_->count()) {
            const auto item      = ui_custom_task_list_->item(index);
            const auto task_name = get_list_item_text(item);
            if (tasks.contains(task_name)) {
                tasks.remove(task_name);
                ++index;
            } else {
                const auto widget = ui_custom_task_list_->itemWidget(item);
                ui_custom_task_list_->setItemWidget(item, nullptr);
                ui_custom_task_list_->takeItem(index);
            }
        }
    }

    const int total_reloaded_task = tasks.size();
    for (const auto task : tasks) { append_list_widget_item(ui_custom_task_list_, new CheckableItem(task)); }

    qInfo() << total_reloaded_task << "tasks reloaded";
}

void Workbench::push_selected_tasks_to_queue() {
    qInfo() << "push selected tasks to queue";

    if (pipeline_state().is_running()) {
        qWarning() << "reject push selected tasks due to pipeline is running";
        QMessageBox::warning(this, "推送任务", "流水线运行中，请先暂停或中止流水线运行。");
        return;
    }

    int total_pushed_major_tasks  = 0;
    int total_pushed_custom_tasks = 0;

    for (int i = 0; i < ui_major_task_list_->count(); ++i) {
        const auto item   = ui_major_task_list_->item(i);
        const auto widget = qobject_cast<TaskItem *>(ui_major_task_list_->itemWidget(item));
        if (!widget->checked()) { continue; }
        auto queued_task = new QueuedTaskItem(widget->task());
        append_list_widget_item(ui_task_queue_, queued_task);
        connect_queued_task_item_signal_slot(ui_task_queue_->item(ui_task_queue_->count() - 1));
        ++total_pushed_major_tasks;
    }

    for (int i = 0; i < ui_custom_task_list_->count(); ++i) {
        const auto item   = ui_custom_task_list_->item(i);
        const auto widget = qobject_cast<CheckableItem *>(ui_custom_task_list_->itemWidget(item));
        if (!widget->checked()) { continue; }
        auto queued_task = new QueuedTaskItem(widget->name());
        append_list_widget_item(ui_task_queue_, queued_task);
        connect_queued_task_item_signal_slot(ui_task_queue_->item(ui_task_queue_->count() - 1));
        ++total_pushed_custom_tasks;
    }

    qInfo() << total_pushed_major_tasks << "major tasks," << total_pushed_custom_tasks << "custom tasks pushed to task queue";
}

void Workbench::notify_queued_task_accepted(const QString &task_id) {
    if (pending_task_accepted_.contains(task_id)) { pending_task_accepted_[task_id] = true; }
}

void Workbench::notify_queued_task_finished(const QString &task_id, MaaStatus status) {
    if (!pending_task_accepted_.contains(task_id)) {
        qCritical().noquote() << "accept finished notification to unknown queued task" << task_id;
        return;
    }
    pending_task_accepted_.remove(task_id);
    if (status == MaaStatus_Success) {
        qInfo().noquote() << "posted queued task" << task_id << "resolved successfully";
    } else {
        qWarning().noquote() << "failed to resolve posted queued task" << task_id;
        qInfo().noquote() << "skip failed queued task" << task_id;
    }
    pop_first_queued_task();
    post_next_queued_task();
}

void Workbench::start_pipeline() {
    if (!instance_ || !instance_->inited()) {
        qWarning() << "workbench: failed to start pipeline: no valid maa instance accepted";
        QMessageBox::critical(this, "启动流水线", "缺少有效 MAA 实例，请检查本地资源完整性并确保已连接至设备");
        return;
    }
    if (pipeline_state().is_running()) {
        qWarning() << "pipeline is already running";
        return;
    }
    if (pipeline_state().is_pending_pause()) {
        qWarning() << "pipeline is pending pause, reject to start pipeline";
        return;
    }
    if (pipeline_state().is_paused()) {
        resume_pipeline();
        return;
    }
    qInfo() << "start pipeline with" << ui_task_queue_->count() << "queued tasks";
    pipeline_state_ = PipelineState_Running;
    post_next_queued_task();
}

void Workbench::pause_pipeline() {
    pause_pipeline(std::nullopt);
}

void Workbench::stop_pipeline() {
    if (pipeline_state().is_idle()) {
        qWarning() << "pipeline is already stopped";
        return;
    }
    pause_pipeline([this] {
        pending_task_accepted_.clear();
        pipeline_state_ = PipelineState_Idle;
        qInfo() << "pipeline stopped";
    });
}

void Workbench::pause_pipeline(std::optional<std::function<void()>> opt_on_paused) {
    if (pipeline_state().is_pending_pause()) {
        qInfo() << "pipeline is already pending pause";
        return;
    }
    if (!pipeline_state().is_running()) {
        qWarning() << "reject pause pipeline: pipeline is not running";
        return;
    }
    if (fut_pending_pause_.state_->task_.has_value() && !fut_pending_pause_.fulfilled()) {
        qCritical() << "failed to pause pipeline: unknown error";
        return;
    }
    pipeline_state_    = PipelineState_PendingPause;
    fut_pending_pause_ = coro::EventLoop::current()->eval([this, opt_on_paused] {
        handle_on_pending_pipeline_pause_done();
        if (opt_on_paused.has_value()) { std::invoke(*opt_on_paused); }
    });
    qInfo() << "wait the pipeline to be paused";
}

void Workbench::resume_pipeline() {
    if (pipeline_state().is_pending_pause()) {
        qWarning() << "failed to resume pipeline: pipeline is pending pause";
        return;
    }
    if (!pipeline_state().is_paused()) {
        qWarning() << "failed to resume pipeline: pipeline is not paused";
        return;
    }
    //! FIXME: resume under the guidance of pending_task_accepted_
    post_next_queued_task();
}

void Workbench::post_next_queued_task() {
    if (ui_task_queue_->count() == 0) {
        qInfo() << "post queued task: no more tasks found in task queue";
        stop_pipeline();
        return;
    }
    const auto task_item = qobject_cast<QueuedTaskItem *>(ui_task_queue_->itemWidget(ui_task_queue_->item(0)));
    const auto task_id   = QUuid::createUuid().toString();
    const auto task = task_item->task_type() == QueuedTaskItem::MajorTask ? QVariant::fromValue(task_item->major_task().value())
                                                                          : QVariant::fromValue(task_item->task_name());
    pending_task_accepted_[task_id] = false;

    qInfo().noquote().nospace() << "post queued task " << task << " [task_id=" << task_id << "]";

    //! NOTE: if no handle to resolve the posted queued task, then simply cancel it
    QTimer::singleShot(3000, [this, task_id] {
        if (!pending_task_accepted_.value(task_id, true)) { handle_on_cancel_posted_queued_task(task_id); }
    });
    emit on_post_queued_task(task_id, std::move(task));
}

void Workbench::handle_on_pending_pipeline_pause_done() {
    pipeline_state_ = PipelineState_Paused;
    qInfo() << "pipeline paused";
}

void Workbench::handle_on_cancel_posted_queued_task(const QString &task_id) {
    if (!pending_task_accepted_.contains(task_id)) { return; }
    qWarning().noquote() << "no resolver found, cancel posted queued task" << task_id;
    pending_task_accepted_.remove(task_id);
    stop_pipeline();
}

void Workbench::handle_on_task_list_filter_changed(QListWidget *list, const QString &filter) {
    QRegularExpression regex(filter);
    if (!regex.isValid()) { return; }
    for (int i = 0; i < list->count(); ++i) {
        const auto item       = list->item(i);
        const bool filter_out = !filter.isEmpty() && !regex.match(get_list_item_text(item)).hasMatch();
        item->setHidden(filter_out);
    }
}

void Workbench::handle_on_toggle_task_list_select_all(QListWidget *list, bool on) {
    for (int i = 0; i < list->count(); ++i) {
        //! TODO: enable user to config whether to ignore hidden items or not
        if (list->item(i)->isHidden()) { continue; }
        set_list_item_widget_checked(list->item(i), on);
    }
}

void Workbench::handle_on_execute_queued_task_action(QListWidgetItem *item, QueuedTaskItem::ActionType action) {
    if (const auto state = pipeline_state(); !(state.is_paused() || state.is_idle())) {
        qWarning() << "reject execute queued task action: pipeline is running";
        QMessageBox::warning(this, "任务调整", "请先暂停或中止流水线运行。");
        return;
    }

    const int  index       = ui_task_queue_->row(item);
    const auto item_widget = qobject_cast<QueuedTaskItem *>(ui_task_queue_->itemWidget(item));
    Q_ASSERT(index >= 0);

    if (action == QueuedTaskItem::Drop) {
        const auto widget = ui_task_queue_->itemWidget(item);
        ui_task_queue_->setItemWidget(item, nullptr);
        ui_task_queue_->takeItem(index);
        return;
    }

    bool ignore = false;
    switch (action) {
        case QueuedTaskItem::MoveUp:
            [[fallthrough]];
        case QueuedTaskItem::MoveToTop: {
            ignore = index == 0;
        } break;
        case QueuedTaskItem::MoveDown:
            [[fallthrough]];
        case QueuedTaskItem::MoveToBottom: {
            ignore = index + 1 == ui_custom_task_list_->count();
        } break;
        default: {
            std::unreachable();
        } break;
    }

    int next_index = -1;
    switch (action) {
        case QueuedTaskItem::MoveUp: {
            next_index = index - 1;
        } break;
        case QueuedTaskItem::MoveDown: {
            next_index = index + 1;
        } break;
        case QueuedTaskItem::MoveToTop: {
            next_index = 0;
        } break;
        case QueuedTaskItem::MoveToBottom: {
            next_index = ui_task_queue_->count() - 1;
        } break;
        default: {
            std::unreachable();
        } break;
    }

    const auto cloned_widget = item_widget->task_type() == QueuedTaskItem::MajorTask
                                 ? new QueuedTaskItem(item_widget->major_task().value())
                                 : new QueuedTaskItem(item_widget->task_name());
    ui_task_queue_->takeItem(index);
    ui_task_queue_->insertItem(next_index, item);
    ui_task_queue_->setItemWidget(item, cloned_widget);
    connect_queued_task_item_signal_slot(item);
}

Workbench::Workbench(QWidget *parent)
    : QWidget(parent)
    , pipeline_state_(PipelineState_Idle) {
    setup();
    ui_log_panel_->attach_to_global_logger();
}

Workbench::~Workbench() {
    if (pipeline_state().is_running()) { stop_pipeline(); }
}

void Workbench::setup() {
    ui_major_tasks_select_all_  = new QCheckBox;
    ui_major_task_list_         = new QListWidget;
    ui_major_task_filter_       = new QtMaterialTextField;
    ui_custom_tasks_select_all_ = new QCheckBox;
    ui_custom_task_list_        = new QListWidget;
    ui_custom_task_filter_      = new QtMaterialTextField;
    ui_task_queue_              = new QListWidget;
    ui_reload_custom_tasks_     = new FlatButton;
    ui_join_tasks_              = new FlatButton;
    ui_start_pipeline_          = new FlatButton;
    ui_pause_pipeline_          = new FlatButton;
    ui_log_panel_               = new LogPanel;
    ui_open_maa_log_file_       = new FlatButton;
    ui_open_app_log_file_       = new FlatButton;

    config_list_widget(ui_major_task_list_);
    config_list_widget(ui_custom_task_list_);
    config_list_widget(ui_task_queue_);

    config_flat_button(ui_reload_custom_tasks_, "重载自定义任务");
    config_flat_button(ui_join_tasks_, "推送任务");
    config_flat_button(ui_start_pipeline_, "启动流水线");
    config_flat_button(ui_pause_pipeline_, "暂停流水线");
    config_flat_button(ui_open_maa_log_file_, "打开 MAA 日志");
    config_flat_button(ui_open_app_log_file_, "打开应用日志");

    ui_major_tasks_select_all_->setText("全选");
    ui_custom_tasks_select_all_->setText("全选");
    ui_major_task_filter_->setFont(font());
    ui_major_task_filter_->setPlaceholderText("任务过滤器（正则表达式）");
    ui_major_task_filter_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    ui_custom_task_filter_->setFont(font());
    ui_custom_task_filter_->setPlaceholderText("任务过滤器（正则表达式）");
    ui_custom_task_filter_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    auto task_list_container   = new QWidget;
    auto active_task_container = new QWidget;
    auto log_panel_container   = new QWidget;

    auto task_list_area = new QWidget;
    {
        auto layout = new QVBoxLayout(task_list_area);
        layout->setContentsMargins({});
        auto major_task_area = new QWidget;
        {
            auto layout = new QVBoxLayout(major_task_area);
            layout->setContentsMargins({});
            layout->addWidget(ui_major_task_list_);
            auto filter_area = new QWidget;
            {
                auto layout = new QHBoxLayout(filter_area);
                layout->setContentsMargins({});
                auto label = new QLabel;
                label->setText("筛选：");
                label->setFont(font());
                layout->addWidget(label);
                layout->addWidget(ui_major_task_filter_);
            }
            layout->addWidget(filter_area);
        }
        layout->addWidget(make_titled_widget("核心任务", major_task_area, ui_major_tasks_select_all_));
        auto custom_task_area = new QWidget;
        {
            auto layout = new QVBoxLayout(custom_task_area);
            layout->setContentsMargins({});
            layout->addWidget(ui_custom_task_list_);
            auto filter_area = new QWidget;
            {
                auto layout = new QHBoxLayout(filter_area);
                layout->setContentsMargins({});
                auto label = new QLabel;
                label->setText("筛选：");
                label->setFont(font());
                layout->addWidget(label);
                layout->addWidget(ui_custom_task_filter_);
            }
            layout->addWidget(filter_area);
        }
        layout->addWidget(make_titled_widget("自定义任务", custom_task_area, ui_custom_tasks_select_all_));
    }

    auto button_area = new QWidget;
    {
        auto layout = new QGridLayout(button_area);
        layout->setContentsMargins({});
        layout->addWidget(ui_reload_custom_tasks_, 0, 0);
        layout->addWidget(ui_join_tasks_, 0, 1);
        layout->addWidget(ui_start_pipeline_, 1, 0);
        layout->addWidget(ui_pause_pipeline_, 1, 1);
    }

    auto task_queue_area = make_titled_widget("任务队列", ui_task_queue_);

    auto logger_area = new QWidget;
    {
        auto layout = new QVBoxLayout(logger_area);
        layout->setContentsMargins({});
        layout->setSpacing(8);
        layout->addWidget(make_titled_widget("运行日志", ui_log_panel_));
        auto open_log_file_area = new QWidget;
        {
            auto layout = new QHBoxLayout(open_log_file_area);
            layout->setContentsMargins({});
            layout->addWidget(ui_open_maa_log_file_);
            layout->addWidget(ui_open_app_log_file_);
        }
        layout->addWidget(open_log_file_area);
    }

    for (auto task : magic_enum::enum_values<Task::MajorTask>()) {
        const auto task_item = new TaskItem(task);
        append_list_widget_item(ui_major_task_list_, task_item);
        if (task_item->has_config()) {
            connect(task_item, &TaskItem::on_request_config, this, &Workbench::on_request_open_task_config_panel);
        }
    }

    auto layout = new QGridLayout(this);
    layout->setContentsMargins({});

    layout->addWidget(task_list_container, 0, 0);
    layout->addWidget(active_task_container, 0, 1);
    layout->addWidget(log_panel_container, 0, 2, 1, 2);

    {
        auto layout = new QVBoxLayout(task_list_container);
        layout->setSpacing(8);
        layout->addWidget(task_list_area);
        layout->addWidget(button_area);
        task_list_container->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    }
    {
        auto layout = new QVBoxLayout(active_task_container);
        layout->addWidget(task_queue_area);
        active_task_container->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    }
    {
        auto layout = new QVBoxLayout(log_panel_container);
        layout->addWidget(logger_area);
        log_panel_container->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    }

    //! TODO: add stop pipeline button and reconnnect pause-pipeline to pause-pipeline
    //! NOTE: currently pause-pipeline means stop-pipeline
    connect(ui_reload_custom_tasks_, &FlatButton::pressed, this, &Workbench::on_reload_assets);
    connect(ui_join_tasks_, &FlatButton::pressed, this, &Workbench::on_push_tasks);
    connect(ui_start_pipeline_, &FlatButton::pressed, this, &Workbench::start_pipeline);
    connect(ui_pause_pipeline_, &FlatButton::pressed, this, &Workbench::stop_pipeline);
    connect(ui_open_maa_log_file_, &FlatButton::pressed, this, &Workbench::on_open_maa_log_file);
    connect(ui_open_app_log_file_, &FlatButton::pressed, this, &Workbench::on_open_app_log_file);
    connect(ui_major_task_filter_, &QtMaterialTextField::textChanged, this, &Workbench::handle_on_major_task_filter_changed);
    connect(ui_custom_task_filter_, &QtMaterialTextField::textChanged, this, &Workbench::handle_on_custom_task_filter_changed);
    connect(ui_major_tasks_select_all_, &QAbstractButton::toggled, this, &Workbench::handle_on_toggle_major_tasks_select_all);
    connect(ui_custom_tasks_select_all_, &QAbstractButton::toggled, this, &Workbench::handle_on_toggle_custom_tasks_select_all);
    connect(this, &Workbench::on_push_tasks, this, &Workbench::push_selected_tasks_to_queue);
}

} // namespace UI

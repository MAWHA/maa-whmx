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

#include "App.h"
#include "Consts.h"
#include "Logger.h"
#include "AppEvent.h"
#include "Experimental/UmaClient.h"
#include "Task/Router.h"

#include <QtCore/QThread>
#include <ElaApplication.h>

static const auto SINGLETON_IPC_MEMORY_ID =
    QString("%1::UniversalMaaActuator.Singleton").arg(QString::fromUtf8(Consts::MUA_UUID));

class MaaWorker : public QThread {
public:
    MaaWorker(std::shared_ptr<maa::coro::EventLoop> ev, QObject *parent)
        : QThread(parent)
        , ev_(ev) {}

    void run() override {
        ev_->exec();
    }

private:
    std::shared_ptr<maa::coro::EventLoop> ev_;
};

tl::expected<gsl::not_null<std::shared_ptr<UniversalMaaActuator>>, QString>
    UniversalMaaActuator::create(int &argc, char **argv) {
    if (gApp != nullptr) { return tl::unexpected("uma is already initialized in the current process"); }

    auto singleton_ipc_memory = std::make_unique<QSharedMemory>(SINGLETON_IPC_MEMORY_ID);
    Ensures(singleton_ipc_memory != nullptr);
    if (const bool created = singleton_ipc_memory->create(sizeof(WId)); !created) {
        if (const auto state = singleton_ipc_memory->error(); state == QSharedMemory::AlreadyExists) {
            return tl::unexpected("uma is already running");
        } else {
            return tl::unexpected(QString("unknown error [error=%1]").arg(state));
        }
    }

    singleton_ipc_memory->attach();
    const auto app_ptr = new UniversalMaaActuator(argc, argv, singleton_ipc_memory.release());
    const auto app     = gsl::make_not_null(std::shared_ptr<UniversalMaaActuator>(app_ptr));

    QApplication::setOrganizationDomain("github.com/MAWHA");
    QApplication::setOrganizationName("MAWHA");
    QApplication::setApplicationName("whmx-assistant");
    QApplication::setApplicationDisplayName("物华弥新小助手");
    QApplication::setApplicationVersion(QString::fromUtf8(Consts::VERSION));
    QApplication::setWindowIcon(QIcon(":/logo.png"));

    return app;
}

gsl::not_null<std::shared_ptr<QWidget>> UniversalMaaActuator::window() const {
    return Experimental::UmaClient::create_or_get();
}

gsl::strict_not_null<const QWidget *> UniversalMaaActuator::window_cref() const {
    return gsl::make_strict_not_null(window().get().get());
}

void UniversalMaaActuator::wakeup_client() {
    auto client = window();
    {
        singleton_ipc_memory_->lock();
        auto &uma_window_id = *static_cast<WId *>(singleton_ipc_memory_->data());
        uma_window_id       = client->winId();
        singleton_ipc_memory_->unlock();
    }
    client->show();
}

UniversalMaaActuator::UniversalMaaActuator(
    int &argc, char **argv, gsl::not_null<gsl::owner<QSharedMemory *>> singleton_ipc_memory)
    : QApplication(argc, argv)
    , default_maa_event_loop_(std::make_shared<maa::coro::EventLoop>())
    , maa_worker_(new MaaWorker(default_maa_event_loop_, this))
    , app_event_(new AppEvent(this))
    , singleton_ipc_memory_(std::unique_ptr<QSharedMemory>(singleton_ipc_memory)) {
    setup();
    maa_worker_->start();
}

void UniversalMaaActuator::setup() {
    Ensures(gApp == this);

    ElaApplication::getInstance()->init();
    GlobalLoggerProxy::setup();
    Task::Router::setup();

    setQuitOnLastWindowClosed(false);

    {
        auto pal = palette();
        pal.setColor(QPalette::Window, Qt::white);
        pal.setColor(QPalette::Base, Qt::white);
        setPalette(pal);
    }

    const auto event = gApp->app_event();

    connect(this, &QApplication::aboutToQuit, this, [this] {
        default_maa_event_loop_->stop();
    });

    connect(gApp, &QApplication::lastWindowClosed, event, &AppEvent::on_exit);
    connect(event, &AppEvent::on_exit, gApp, &QApplication::quit);
}

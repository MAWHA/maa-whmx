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

#include "Consts.h"
#include "Logger.h"
#include "Task/Router.h"
#include "UI/Client.h"

#include <MaaPP/MaaPP.hpp>
#include <QtWidgets/QApplication>
#include <QtCore/QThread>
#include <ElaApplication.h>

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

class WhmxAssistant : public QApplication {
public:
    WhmxAssistant(int &argc, char **argv)
        : QApplication(argc, argv)
        , default_maa_event_loop_(std::make_shared<maa::coro::EventLoop>())
        , maa_worker_(new MaaWorker(default_maa_event_loop_, this)) {
        setPalette(style()->standardPalette());

        ElaApplication::getInstance()->init();
        GlobalLoggerProxy::setup();
        Task::Router::setup();

        connect(this, &QApplication::aboutToQuit, this, [this] {
            default_maa_event_loop_->stop();
        });

        maa_worker_->start();
    }

private:
    std::shared_ptr<maa::coro::EventLoop> default_maa_event_loop_;
    QThread                              *maa_worker_;
};

int main(int argc, char *argv[]) {
    WhmxAssistant app(argc, argv);

    QApplication::setOrganizationDomain("github.com/MAWHA");
    QApplication::setOrganizationName("MAWHA");
    QApplication::setApplicationName("whmx-assistant");
    QApplication::setApplicationDisplayName("物华弥新小助手");
    QApplication::setApplicationVersion(QString::fromUtf8(Consts::VERSION));

    auto client = std::make_shared<UI::Client>();
    client->show();

    return app.exec();
}

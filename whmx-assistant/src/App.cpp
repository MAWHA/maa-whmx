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
#include "UI/Client.h"
#include "UI/TitleBar.h"
#include "Platform.h"
#include "Logger.h"
#include "Task/Router.h"

#include <QtCore/QThread>
#include <QtWidgets/QStyle>
#include <ElaApplication.h>

using namespace UI;

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

WhmxAssistant::WhmxAssistant(int &argc, char **argv)
    : QApplication(argc, argv)
    , default_maa_event_loop_(std::make_shared<maa::coro::EventLoop>())
    , maa_worker_(new MaaWorker(default_maa_event_loop_, this))
    , app_event_(new AppEvent(this)) {
    setPalette(style()->standardPalette());
    setQuitOnLastWindowClosed(false);

    ElaApplication::getInstance()->init();
    GlobalLoggerProxy::setup();
    Task::Router::setup();

    connect(this, &QApplication::aboutToQuit, this, [this] {
        default_maa_event_loop_->stop();
    });

    maa_worker_->start();
}

void WhmxAssistant::create_or_wakeup_client() {
    auto client = new QWidget();

    client->setAutoFillBackground(true);
    client->setBackgroundRole(QPalette::Base);
    {
        auto pal = client->palette();
        pal.setColor(QPalette::Base, Qt::white);
        setPalette(pal);
    }

    auto app_bar = new UI::TitleBar;
    auto pushpin = new ElaIconButton(ElaIconType::PushPin, 15, 30, 30);
    {
        pushpin->setLightHoverIconColor("#9d9be1");
        pushpin->setLightHoverColor("#ffffff");
        app_bar->tool_button_layout()->addWidget(pushpin);
        app_bar->set_tracking_widget(client);
    }

    auto layout = new QVBoxLayout(client);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(app_bar);
    layout->addWidget(new Client);

    auto frameless_agent = new QWK::WidgetWindowAgent(client);
    frameless_agent->setup(client);
    frameless_agent->setTitleBar(app_bar);
    for (int i = 0; i < app_bar->tool_button_layout()->count(); ++i) {
        if (auto w = app_bar->tool_button_layout()->itemAt(i)->widget()) { frameless_agent->setHitTestVisible(w); }
    }
    frameless_agent->setSystemButton(QWK::WidgetWindowAgent::Minimize, app_bar->minimize_button());
    frameless_agent->setSystemButton(QWK::WidgetWindowAgent::Maximize, app_bar->maximize_button());
    frameless_agent->setSystemButton(QWK::WidgetWindowAgent::Close, app_bar->close_button());

    connect(app_bar, &TitleBar::on_request_minimize, gApp->app_event(), &AppEvent::on_minimize);
    connect(app_bar, &TitleBar::on_request_maximize, gApp->app_event(), &AppEvent::on_maximize);
    connect(app_bar, &TitleBar::on_request_restore, gApp->app_event(), &AppEvent::on_restore);
    connect(app_bar, &TitleBar::on_request_close, gApp->app_event(), &AppEvent::on_close);
    connect(pushpin, &ElaIconButton::clicked, this, [=] {
        const bool on = !pushpin->getIsSelected();
        pushpin->setIsSelected(on);
        emit gApp->app_event()->on_set_window_top(on);
    });

    connect(gApp->app_event(), &AppEvent::on_minimize, client, &QWidget::showMinimized);
    connect(gApp->app_event(), &AppEvent::on_maximize, client, &QWidget::showMaximized);
    connect(gApp->app_event(), &AppEvent::on_restore, client, &QWidget::showNormal);
    connect(gApp->app_event(), &AppEvent::on_close, client, &QWidget::close);
    connect(gApp->app_event(), &AppEvent::on_set_window_top, this, [=](bool on) {
        Platform::set_window_top_most(reinterpret_cast<void *>(client->window()->winId()), on);
    });
    connect(gApp, &QApplication::lastWindowClosed, gApp->app_event(), &AppEvent::on_exit);
    connect(gApp->app_event(), &AppEvent::on_exit, gApp, &QApplication::quit);

    client->show();
}

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
#include "UI/TitleBar.h"
#include "UI/Client.h"
#include "UI/Workbench.h"
#include "UI/DeviceConn.h"
#include "UI/Settings.h"
#include "UI/NavBar.h"
#include "Platform.h"
#include "Logger.h"
#include "Task/Router.h"

#include <QtCore/QThread>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStackedWidget>
#include <ElaApplication.h>
#include <ElaFlowLayout.h>
#include <ElaText.h>
#include <ElaScrollArea.h>

#include <ElaNavigationBar.h>
#include "UI/Card.h"

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

    client_ = new QWidget;
}

void WhmxAssistant::create_or_wakeup_client() {
    client_->setAutoFillBackground(true);
    client_->setBackgroundRole(QPalette::Base);
    {
        auto pal = client_->palette();
        pal.setColor(QPalette::Base, Qt::white);
        setPalette(pal);
    }

    auto device_conn = new DeviceConn;
    auto workbench   = new Workbench;
    auto settings    = new Settings;
    new Client(".", client_);

    auto app_bar = new UI::TitleBar;
    auto pushpin = new ElaIconButton(ElaIconType::PushPin, 15, 30, 30);
    {
        pushpin->setLightHoverIconColor("#9d9be1");
        pushpin->setLightHoverColor("#ffffff");
        app_bar->tool_button_layout()->addWidget(pushpin);
        app_bar->set_tracking_widget(client_);
    }

    auto workstaion_page = new QWidget;
    {
        auto card = new Card;
        card->set_title("物华弥新");
        card->set_brief("你好，欢迎使用物华弥新小助手！");
        card->set_card_pixmap(QPixmap(":/logo.png"));

        auto page_title = new ElaText("工作站");
        page_title->setTextStyle(ElaTextType::TextStyle::Title);

        auto card_container = new QWidget;
        auto card_layout    = new ElaFlowLayout(card_container);
        card_layout->addWidget(card);

        auto card_scroll = new ElaScrollArea;
        card_scroll->setWidgetResizable(true);
        card_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        card_scroll->setIsGrabGesture(Qt::Vertical, true);
        card_scroll->setWidget(card_container);

        auto page_layout = new QVBoxLayout(workstaion_page);
        page_layout->setContentsMargins(8, 8, 8, 4);
        page_layout->addWidget(page_title);
        page_layout->addSpacing(8);
        page_layout->addWidget(card_scroll);
    }

    auto task_config_page = new QWidget;
    {
        auto layout       = new QVBoxLayout(task_config_page);
        auto config_title = new ElaText;
        config_title->setTextStyle(ElaTextType::TextStyle::Title);
        layout->setContentsMargins(8, 8, 8, 4);
        layout->setSpacing(8);
        layout->addWidget(config_title);
    }

    auto nav_widget = new QStackedWidget;
    auto nav        = new UI::NavBar;
    auto router     = std::make_shared<QMap<QString, QString>>();
    connect(nav, &NavBar::on_add_nav_page, gApp, [=](NavNode *node, QWidget *page) {
        nav_widget->addWidget(page);
        router->insert(node->text(), node->key());
    });
    connect(nav, &NavBar::on_navigation, nav_widget, &QStackedWidget::setCurrentWidget);
    nav->add_page_node(ElaIconType::GridRound2Plus, "终端", workstaion_page);
    nav->add_page_node(ElaIconType::RightLeftLarge, "设备", device_conn);
    nav->add_page_node(ElaIconType::RocketLaunch, "作业", workbench);
    nav->add_page_node(ElaIconType::GearCode, "任务", task_config_page);
    nav->add_footer_node(ElaIconType::CirclePlus, new QAction);
    nav->add_footer_node(ElaIconType::Gear, settings);
    nav->navigate(router->value("终端"));

    auto ws_layout = new QHBoxLayout;
    ws_layout->addWidget(nav);
    ws_layout->addWidget(nav_widget);

    auto layout = new QVBoxLayout(client_);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(app_bar);
    layout->addLayout(ws_layout);

    auto frameless_agent = new QWK::WidgetWindowAgent(client_);
    frameless_agent->setup(client_);
    frameless_agent->setTitleBar(app_bar);
    for (int i = 0; i < app_bar->tool_button_layout()->count(); ++i) {
        if (auto w = app_bar->tool_button_layout()->itemAt(i)->widget()) { frameless_agent->setHitTestVisible(w); }
    }
    frameless_agent->setSystemButton(QWK::WidgetWindowAgent::Minimize, app_bar->minimize_button());
    frameless_agent->setSystemButton(QWK::WidgetWindowAgent::Maximize, app_bar->maximize_button());
    frameless_agent->setSystemButton(QWK::WidgetWindowAgent::Close, app_bar->close_button());

    const auto event = gApp->app_event();
    connect(app_bar, &TitleBar::on_request_minimize, event, &AppEvent::on_minimize);
    connect(app_bar, &TitleBar::on_request_maximize, event, &AppEvent::on_maximize);
    connect(app_bar, &TitleBar::on_request_restore, event, &AppEvent::on_restore);
    connect(app_bar, &TitleBar::on_request_close, event, &AppEvent::on_close);
    connect(pushpin, &ElaIconButton::clicked, this, [=] {
        const bool on = !pushpin->getIsSelected();
        pushpin->setIsSelected(on);
        emit event->on_set_window_top(on);
    });
    connect(event, &AppEvent::on_open_task_config_panel, gApp, [=](QString title, QWidget *panel) {
        panel->moveToThread(gApp->thread());
        auto layout = task_config_page->layout();
        while (layout->count() != 1) { layout->takeAt(1)->widget()->deleteLater(); }
        auto config_title = static_cast<ElaText *>(layout->itemAt(0)->widget());
        config_title->setText(title);
        layout->addWidget(panel);
        nav->navigate(router->value("任务"));
    });
    connect(event, &AppEvent::on_minimize, client_, &QWidget::showMinimized);
    connect(event, &AppEvent::on_maximize, client_, &QWidget::showMaximized);
    connect(event, &AppEvent::on_restore, client_, &QWidget::showNormal);
    connect(event, &AppEvent::on_close, client_, &QWidget::close);
    connect(event, &AppEvent::on_set_window_top, this, [=](bool on) {
        Platform::set_window_top_most(reinterpret_cast<void *>(client_->window()->winId()), on);
    });
    connect(gApp, &QApplication::lastWindowClosed, event, &AppEvent::on_exit);
    connect(event, &AppEvent::on_exit, gApp, &QApplication::quit);

    client_->show();
}

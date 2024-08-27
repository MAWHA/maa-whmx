#include "UmaClient.h"
#include "../App.h"
#include "../Platform.h"
#include "../UI/TitleBar.h"
#include "../UI/Client.h"
#include "../UI/Workbench.h"
#include "../UI/DeviceConn.h"
#include "../UI/Settings.h"
#include "../UI/NavBar.h"
#include "../UI/Card.h"

#include <QtWidgets/QStyle>
#include <QtWidgets/QStackedWidget>
#include <ElaFlowLayout.h>
#include <ElaText.h>
#include <ElaScrollArea.h>
#include <ElaNavigationBar.h>
#include <QWKWidgets/widgetwindowagent.h>

using namespace UI;

namespace Experimental {

static std::shared_ptr<UmaClient> UMA_CLIENT_INSTANCE;

struct QtObjectTreeDeleter {
    void operator()(gsl::not_null<QObject *> ptr) const {
        ptr->deleteLater();
    }
};

bool UmaClient::initialized() {
    return UMA_CLIENT_INSTANCE != nullptr;
}

gsl::not_null<std::shared_ptr<UmaClient>> UmaClient::create_or_get() {
    if (!UMA_CLIENT_INSTANCE) { UMA_CLIENT_INSTANCE = std::shared_ptr<UmaClient>(new UmaClient, QtObjectTreeDeleter{}); }
    return UMA_CLIENT_INSTANCE;
}

UmaClient::UmaClient() {
    setup();
}

void UmaClient::setup() {
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    {
        auto pal = palette();
        pal.setColor(QPalette::Base, Qt::white);
        setPalette(pal);
    }

    auto device_conn = new DeviceConn;
    auto workbench   = new Workbench;
    auto settings    = new Settings;
    std::ignore      = new Client(".", this);

    auto app_bar = new TitleBar;
    auto pushpin = new ElaIconButton(ElaIconType::PushPin, 15, 30, 30);
    {
        pushpin->setLightHoverIconColor("#9d9be1");
        pushpin->setLightHoverColor("#ffffff");
        app_bar->tool_button_layout()->addWidget(pushpin);
        app_bar->set_tracking_widget(this);
    }

    auto workstaion_page = new QWidget;
    {
        auto card1 = new Card;
        card1->set_title("物华弥新");
        card1->set_brief("欢迎使用物华弥新小助手！");
        card1->set_card_pixmap(QPixmap(":/logo.png"));

        auto card2 = new Card;
        card2->set_title("这是版本前瞻，不是公告（划掉）");
        card2->set_brief(
            "0.6.x "
            "版本将实装“工作站”功能，支持多设备（甚至多游戏）操作，你在当前页面见到的每一张卡片都将成为持久化的本地实例配置");

        auto card3 = new Card;
        card3->set_title("## 预告 ##");
        card3->set_brief("从 0.6.x 开始将逐步拆除代码中对物华弥新功能的实现，转向由插件+配置实现的通用作业 GUI "
                         "应用");

        auto card4 = new Card;
        card4->set_title("## 预告（续） ##");
        card4->set_brief("**我的意思是**，现存作业将不再一并维护，而是转换为“物华弥新”作业包（插件+配置）以使用");

        auto card5 = new Card;
        card5->set_title("声明");
        card5->set_brief("是转型不是跑路，再说了不是还有隔壁的 MWA 嘛！总之会保证现存功能总是可用于更新/转型后的本应用的。");

        auto page_title = new ElaText("工作站");
        page_title->setTextStyle(ElaTextType::TextStyle::Title);

        auto card_container = new QWidget;
        auto card_layout    = new ElaFlowLayout(card_container);
        card_layout->addWidget(card1);
        card_layout->addWidget(card2);
        card_layout->addWidget(card3);
        card_layout->addWidget(card4);
        card_layout->addWidget(card5);

        auto card_scroll = new ElaScrollArea;
        card_scroll->setWidgetResizable(true);
        card_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        card_scroll->setIsGrabGesture(true);
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
    auto nav        = new NavBar;
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

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(app_bar);
    layout->addLayout(ws_layout);

    auto frameless_agent = new QWK::WidgetWindowAgent(this);
    frameless_agent->setup(this);
    frameless_agent->setTitleBar(app_bar);
    for (int i = 0; i < app_bar->tool_button_layout()->count(); ++i) {
        if (auto w = app_bar->tool_button_layout()->itemAt(i)->widget()) { frameless_agent->setHitTestVisible(w); }
    }
    frameless_agent->setSystemButton(QWK::WidgetWindowAgent::Minimize, app_bar->minimize_button());
    frameless_agent->setSystemButton(QWK::WidgetWindowAgent::Maximize, app_bar->maximize_button());
    frameless_agent->setSystemButton(QWK::WidgetWindowAgent::Close, app_bar->close_button());

    const auto event = gApp->app_event();

    connect(pushpin, &ElaIconButton::clicked, this, [=] {
        const bool on = !pushpin->getIsSelected();
        pushpin->setIsSelected(on);
        emit event->on_set_window_top(on);
    });
    connect(event, &AppEvent::on_open_task_config_panel, this, [=, this](QString title, QWidget *panel) {
        panel->moveToThread(thread());
        auto layout = task_config_page->layout();
        while (layout->count() != 1) { layout->takeAt(1)->widget()->deleteLater(); }
        auto config_title = static_cast<ElaText *>(layout->itemAt(0)->widget());
        config_title->setText(title);
        layout->addWidget(panel);
        nav->navigate(router->value("任务"));
    });

    connect(app_bar, &TitleBar::on_request_minimize, event, &AppEvent::on_minimize);
    connect(app_bar, &TitleBar::on_request_maximize, event, &AppEvent::on_maximize);
    connect(app_bar, &TitleBar::on_request_restore, event, &AppEvent::on_restore);
    connect(app_bar, &TitleBar::on_request_close, event, &AppEvent::on_close);

    connect(event, &AppEvent::on_minimize, this, &QWidget::showMinimized);
    connect(event, &AppEvent::on_maximize, this, &QWidget::showMaximized);
    connect(event, &AppEvent::on_restore, this, &QWidget::showNormal);
    connect(event, &AppEvent::on_close, this, &QWidget::close);
    connect(event, &AppEvent::on_set_window_top, this, [this](bool on) {
        Platform::set_window_top_most(reinterpret_cast<void *>(window()->winId()), on);
    });
}

} // namespace Experimental

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

#include "UmaClient.h"
#include "../App.h"
#include "../Platform.h"
#include "../UI/Settings.h"

#include <QtWidgets/QStyle>
#include <QtWidgets/QStackedWidget>
#include <ElaFlowLayout.h>
#include <ElaText.h>
#include <ElaMenu.h>
#include <ElaScrollArea.h>
#include <ElaNavigationBar.h>
#include <ElaTheme.h>
#include <QWKWidgets/widgetwindowagent.h>
#include <QUuid>
#include <magic_enum.hpp>

using namespace UI;

namespace Experimental {

static std::shared_ptr<UmaClient> UMA_CLIENT_INSTANCE;

struct QtObjectTreeDeleter {
    void operator()(gsl::not_null<QObject *> ptr) const {
        ptr->deleteLater();
    }
};

void UmaClient::show_uma_card_context_menu(const QString &id, const QPoint &pos) {
    Expects(card_for_uma_instance_.contains(id));

    const auto menu = new ElaMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    //! TODO: hide or disable part of items based on the properties of the target uma instance

    const auto action_startup = menu->addElaIconAction(ElaIconType::CircleCaretRight, "启动");
    const auto action_pause   = menu->addElaIconAction(ElaIconType::CirclePause, "暂停");
    const auto action_config  = menu->addElaIconAction(ElaIconType::Gear, "配置");
    const auto action_remove  = menu->addElaIconAction(ElaIconType::TrashXmark, "删除");

    const auto more           = menu->addMenu(ElaIconType::CircleEllipsis, "更多");
    const auto action_enqueue = more->addElaIconAction(ElaIconType::CodePullRequest, "加入启动队列");
    const auto action_mark    = more->addElaIconAction(ElaIconType::LightbulbOn, "颜色标记");
    const auto action_clone   = more->addElaIconAction(ElaIconType::DiagramProject, "克隆实例");
    const auto action_opendir = more->addElaIconAction(ElaIconType::FolderOpen, "打开数据目录");
    const auto action_reset   = more->addElaIconAction(ElaIconType::ArrowsRotate, "恢复默认设置");

    //! NOTE: code for test
    //! TODO: impl all the actions
    connect(action_remove, &QAction::triggered, this, [this, id = id] {
        emit on_remove_uma_instance(id);
    });

    menu->popup(pos);
}

bool UmaClient::initialized() {
    return UMA_CLIENT_INSTANCE != nullptr;
}

gsl::not_null<std::shared_ptr<UmaClient>> UmaClient::create_or_get() {
    if (!UMA_CLIENT_INSTANCE) { UMA_CLIENT_INSTANCE = std::shared_ptr<UmaClient>(new UmaClient, QtObjectTreeDeleter{}); }
    return UMA_CLIENT_INSTANCE;
}

void UmaClient::navigate(PrimaryPage page) {
    Expects(primary_page_nav_keys_.contains(page));
    ui_nav_widget_->nav_bar()->navigate(primary_page_nav_keys_.find(page).value());
}

gsl::strict_not_null<Card *> UmaClient::make_card_for_uma(const QString &id) {
    const auto card = new Card;
    //! TODO: config the card by the properties of the target uma instance
    card->set_title(QString("测试用例·%1").arg(card_for_uma_instance_.size() + 1));
    card->set_brief(QString("来自：%1").arg(id));
    connect(card, &QWidget::customContextMenuRequested, this, [this, id = id, card](const QPoint &pos) {
        show_uma_card_context_menu(id, card->mapToGlobal(pos));
    });
    return gsl::make_not_null(card);
}

UmaClient::UmaClient()
    : ui_title_bar_(new TitleBar)
    , ui_nav_widget_(new NavWidget) {
    setup();
    navigate(Workstation);
}

void UmaClient::setup() {
    const auto event = gApp->app_event();

    //! setup title bar
    {
        ui_title_bar_->set_tracking_widget(this);
        const auto ui_pushpin = new ElaIconButton(ElaIconType::PushPin, 15, 30, 30);
        {
            ui_pushpin->setLightHoverIconColor("#9d9be1");
            ui_pushpin->setLightHoverColor("#ffffff");
            ui_title_bar_->tool_button_layout()->addWidget(ui_pushpin);
        }
        connect(ui_pushpin, &ElaIconButton::clicked, this, [=] {
            const bool on = !ui_pushpin->getIsSelected();
            ui_pushpin->setIsSelected(on);
            emit event->on_set_window_top(on);
        });
    }

    //! setup workstation page
    const auto ui_workstation_page = new QWidget;
    {
        auto ui_uma_card_container = new QWidget;
        {
            auto ui_uma_card_layout = new ElaFlowLayout(ui_uma_card_container);
            connect(this, &UmaClient::on_add_uma_instance, this, [this, layout = ui_uma_card_layout](const QString &id) {
                Expects(!card_for_uma_instance_.contains(id));
                const auto card = make_card_for_uma(id);
                card_for_uma_instance_.insert(id, card);
                layout->addWidget(card);
            });
            connect(this, &UmaClient::on_remove_uma_instance, this, [this, layout = ui_uma_card_layout](const QString &id) {
                Expects(card_for_uma_instance_.contains(id));
                const auto card = card_for_uma_instance_.take(id);
                layout->removeWidget(card);
                card->deleteLater();
            });
        }
        auto ui_ws_layout = new QVBoxLayout(ui_workstation_page);
        {
            ui_ws_layout->setContentsMargins(8, 8, 8, 4);
            {
                auto ui_page_title = new ElaText("工作站");
                ui_page_title->setTextStyle(ElaTextType::TextStyle::Title);
                ui_ws_layout->addWidget(ui_page_title);
            }
            ui_ws_layout->addSpacing(8);
            {
                auto uma_card_scroll = new ElaScrollArea;
                uma_card_scroll->setWidgetResizable(true);
                uma_card_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                uma_card_scroll->setIsGrabGesture(true);
                uma_card_scroll->setWidget(ui_uma_card_container);
                ui_ws_layout->addWidget(uma_card_scroll);
            }
        }
    }

    //! setup package page
    const auto ui_package_page = new QWidget;

    //! setup device page
    const auto ui_device_page = new QWidget;

    //! setup nav widget
    {
        const auto nav = ui_nav_widget_->nav_bar();
        {
            const auto key = nav->add_page_node(ElaIconType::GridRound2Plus, "终端", ui_workstation_page);
            primary_page_nav_keys_.insert(Workstation, key);
        }
        {
            const auto key = nav->add_page_node(ElaIconType::BoxOpenFull, "资源", ui_package_page);
            primary_page_nav_keys_.insert(Package, key);
        }
        {
            const auto key = nav->add_page_node(ElaIconType::LaptopBinary, "设备", ui_device_page);
            primary_page_nav_keys_.insert(Device, key);
        }
        {
            const auto req_new_uma = new QAction(this);
            connect(req_new_uma, &QAction::triggered, this, &UmaClient::on_request_new_uma_instance);
            nav->add_footer_node(ElaIconType::CirclePlus, req_new_uma);
        }
        {
            const auto ui_settings_page = new ::UI::Settings;
            const auto key              = nav->add_footer_node(ElaIconType::Gear, ui_settings_page);
            primary_page_nav_keys_.insert(Settings, key);
        }
    }

    //! setup frameless
    {
        auto frameless_agent = new QWK::WidgetWindowAgent(this);
        frameless_agent->setup(this);
        frameless_agent->setTitleBar(ui_title_bar_);
        for (int i = 0; i < ui_title_bar_->tool_button_layout()->count(); ++i) {
            if (auto w = ui_title_bar_->tool_button_layout()->itemAt(i)->widget()) { frameless_agent->setHitTestVisible(w); }
        }
        frameless_agent->setSystemButton(QWK::WidgetWindowAgent::Minimize, ui_title_bar_->minimize_button());
        frameless_agent->setSystemButton(QWK::WidgetWindowAgent::Maximize, ui_title_bar_->maximize_button());
        frameless_agent->setSystemButton(QWK::WidgetWindowAgent::Close, ui_title_bar_->close_button());
    }

    //! setup layout
    auto ui_main_layout = new QVBoxLayout(this);
    {
        ui_main_layout->setContentsMargins(0, 0, 0, 0);
        ui_main_layout->setSpacing(0);
        ui_main_layout->addWidget(ui_title_bar_);
        ui_main_layout->addWidget(ui_nav_widget_);
    }

    setMinimumSize(800, 450);
    setAutoFillBackground(true);

    //! setup connection
    connect(ui_title_bar_, &TitleBar::on_request_minimize, event, &AppEvent::on_minimize);
    connect(ui_title_bar_, &TitleBar::on_request_maximize, event, &AppEvent::on_maximize);
    connect(ui_title_bar_, &TitleBar::on_request_restore, event, &AppEvent::on_restore);
    connect(ui_title_bar_, &TitleBar::on_request_close, event, &AppEvent::on_close);

    connect(event, &AppEvent::on_minimize, this, &QWidget::showMinimized);
    connect(event, &AppEvent::on_maximize, this, &QWidget::showMaximized);
    connect(event, &AppEvent::on_restore, this, &QWidget::showNormal);
    connect(event, &AppEvent::on_close, this, &QWidget::close);
    connect(event, &AppEvent::on_set_window_top, this, [this](bool on) {
        Platform::set_window_top_most(reinterpret_cast<void *>(window()->winId()), on);
    });

    //! NOTE: code for test
    //! TODO: impl request new uma instance
    connect(this, &UmaClient::on_request_new_uma_instance, this, [this] {
        emit on_add_uma_instance(QUuid::createUuid().toString());
    });
}

} // namespace Experimental

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
#include "ProjectDirs.h"
#include "../App.h"
#include "../Platform.h"
#include "../UI/Settings.h"
#include "../UI/FlatButton.h"
#include "../UI/Helper.h"
#include "../UI/Notification.h"
#include "OverlayDialog.h"
#include "StyledPanel.h"

#include <ElaContentDialog.h>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <ElaFlowLayout.h>
#include <ElaText.h>
#include <ElaLineEdit.h>
#include <ElaMenu.h>
#include <ElaScrollArea.h>
#include <ElaTheme.h>
#include <QWKWidgets/widgetwindowagent.h>

using namespace UI;

namespace Experimental {

static std::shared_ptr<UmaClient> UMA_CLIENT_INSTANCE;

struct QtObjectTreeDeleter {
    void operator()(gsl::not_null<QObject *> ptr) const {
        ptr->deleteLater();
    }
};

void UmaClient::handle_on_request_new_uma_instance() {
    if (available_packages_.empty()) {
        Notification::info(gApp->window_cref(), "UMA", "似乎还没有安装任何资源包呢~");
        return;
    }

    //! TODO: enable customize the color mark

    auto panel          = new QWidget;
    auto submit_button  = new FlatButton;
    auto cancel_button  = new FlatButton;
    auto package_select = new ElaComboBox;
    auto name_input     = new ElaLineEdit;
    auto comment_input  = new ElaLineEdit;
    {
        auto layout        = new QVBoxLayout(panel);
        auto form_layout   = new QGridLayout;
        auto button_layout = new QHBoxLayout;
        form_layout->addWidget(new ElaText("资源", 16), 0, 0);
        form_layout->addWidget(package_select, 0, 1);
        form_layout->addWidget(new ElaText("名称", 16), 1, 0);
        form_layout->addWidget(name_input, 1, 1);
        form_layout->addWidget(new ElaText("备注", 16), 2, 0);
        form_layout->addWidget(comment_input, 2, 1);
        form_layout->setSpacing(8);
        button_layout->setContentsMargins(0, 0, 6, 0);
        button_layout->addStretch();
        button_layout->addWidget(cancel_button);
        button_layout->addWidget(submit_button);
        layout->addSpacing(8);
        layout->addLayout(form_layout);
        layout->addSpacing(4);
        layout->addLayout(button_layout);
    }
    {
        package_select->setFixedHeight(35);
        package_select->setPlaceholderText("<请选择需要使用的资源包>");
        name_input->setFixedHeight(35);
        name_input->setClearButtonEnabled(false);
        name_input->setPlaceholderText("来取个一目了然的好名字吧！");
        comment_input->setFixedHeight(35);
        comment_input->setClearButtonEnabled(false);
        comment_input->setPlaceholderText("总之备注一下肯定是有用的！");
        config_flat_button(cancel_button, "取消");
        config_flat_button(submit_button, "创建");
        cancel_button->setFixedSize(80, 32);
        submit_button->setFixedSize(80, 32);
    }
    {
        for (const auto &package : available_packages_) {
            if (!package->valid()) { continue; }
            const auto &info         = package->info().get();
            auto        display_name = QString("%1 %2").arg(info.name).arg(info.generic_version());
            package_select->addItem(display_name, QVariant::fromValue(package));
        }
        if (package_select->count() == 1) {
            package_select->setCurrentIndex(0);
        } else {
            package_select->setCurrentIndex(-1);
        }
    }
    panel->setFixedWidth(400);

    OverlayDialog d;

    connect(submit_button, &FlatButton::pressed, &d, [=, this, &d] {
        if (package_select->currentIndex() == -1) {
            Notification::info(gApp->window_cref(), "UMA", "不选择资源包是不行的哟~");
            return;
        }
        if (name_input->text().isEmpty()) {
            Notification::info(gApp->window_cref(), "UMA", "达咩！还没取名字呢！");
            return;
        }
        d.accept();
    });
    connect(cancel_button, &FlatButton::pressed, &d, &OverlayDialog::quit);

    d.set_widget(new StyledPanel(panel));
    d.exec(ui_nav_widget_);

    if (!d.is_accepted()) { return; }

    auto package           = package_select->currentData().value<std::shared_ptr<class Package>>();
    auto instance          = UmaInstance::create(package);
    instance->name         = name_input->text();
    instance->comment      = comment_input->text();
    instance->marker_color = QColor(42, 109, 197);
    uma_instances_.insert(instance->instance_id, instance);
    emit on_add_uma_instance(instance->instance_id);
}

void UmaClient::show_uma_card_context_menu(const QString &id, const QPoint &pos) {
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
    Expects(uma_instances_.contains(id));
    const auto instance = uma_instances_.find(id).value();

    const auto card = new Card;
    card->set_title(instance->name);
    card->set_brief(instance->comment.isEmpty() ? "哎呀呀，我好像……并没有被备注呢。" : instance->comment);
    card->set_pin_color(instance->marker_color);

    if (const auto icon_url = instance->uma_prop->package->info().get().icon_url; !icon_url.isEmpty()) {
        if (QPixmap pix(icon_url); !pix.isNull()) {
            card->set_card_pixmap(std::move(pix));
        } else {
            auto network = new QNetworkAccessManager;
            auto reply   = network->get(QNetworkRequest(icon_url));
            connect(reply, &QNetworkReply::finished, this, [=] {
                if (reply->error() == QNetworkReply::NoError) {
                    if (auto image = QImage::fromData(reply->readAll()); !image.isNull()) {
                        card->set_card_pixmap(QPixmap::fromImage(image));
                    }
                }
                network->deleteLater();
            });
        }
    }

    connect(card, &QWidget::customContextMenuRequested, this, [this, id = id, card](const QPoint &pos) {
        show_uma_card_context_menu(id, card->mapToGlobal(pos));
    });
    return gsl::make_not_null(card);
}

void UmaClient::scan_and_reload_packages() {
    const auto root = ProjectDirs::create_or_get("packages");
    for (const auto dir : root.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks)) {
        auto package = Package::create(root.absoluteFilePath(dir));
        std::ignore  = package->load_info();
        available_packages_.append(package);
    }
}

UmaClient::UmaClient()
    : ui_title_bar_(new TitleBar)
    , ui_nav_widget_(new NavWidget) {
    setup();
    navigate(Workstation);
    scan_and_reload_packages();
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
                const auto card = make_card_for_uma(id);
                layout->addWidget(card);
                connect(this, &UmaClient::on_add_uma_instance, card, [=, this, this_id = id](const QString &id) {
                    if (this_id != id) { return; }
                    layout->removeWidget(card);
                    card->deleteLater();
                });
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
    connect(this, &UmaClient::on_request_new_uma_instance, this, &UmaClient::handle_on_request_new_uma_instance);

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
}

} // namespace Experimental

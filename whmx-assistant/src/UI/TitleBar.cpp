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

#include "TitleBar.h"
#include "ElidedLabel.h"

#include <QtWidgets/QApplication>

namespace UI {

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent) {
    setup();
    set_title(QApplication::applicationDisplayName());
    set_icon(QApplication::windowIcon());
    set_maximized(false);
}

void TitleBar::setup() {
    auto ui_title_                 = new ElidedLabel;
    auto ui_icon_                  = new QLabel;
    ui_minimize_                   = new ElaIconButton(ElaIconType::Dash, 12, 50, 30);
    ui_maximize_                   = new ElaIconButton(ElaIconType::Square, 13, 50, 30);
    ui_close_                      = new ElaIconButton(ElaIconType::Xmark, 17, 50, 30);
    auto ui_tool_button_container_ = new QWidget;
    ui_tool_button_layout_         = new QHBoxLayout(ui_tool_button_container_);

    ui_title_->set_font_pixel_size(13);
    ui_minimize_->setLightHoverColor("#e9e9f0");
    ui_maximize_->setLightHoverColor("#e9e9f0");
    ui_close_->setLightHoverColor("#c42b1c");
    ui_close_->setLightHoverIconColor("#ffffff");

    auto icon_container = new QWidget;
    {
        auto layout = new QHBoxLayout(icon_container);
        layout->setContentsMargins({});
        layout->setSpacing(0);
        layout->addWidget(ui_icon_);
        layout->addSpacing(10);
    }

    ui_tool_button_layout_->setContentsMargins({});
    ui_tool_button_layout_->setSpacing(2);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addSpacing(5);
    layout->addWidget(icon_container);
    layout->addWidget(ui_title_);
    layout->addStretch();
    layout->addWidget(ui_tool_button_container_);
    layout->addWidget(ui_minimize_);
    layout->addWidget(ui_maximize_);
    layout->addWidget(ui_close_);

    setFixedHeight(30);

    connect(this, &TitleBar::on_title_change, ui_title_, [=, this] {
        ui_title_->setText(title());
    });
    connect(this, &TitleBar::on_icon_change, ui_icon_, [=, this] {
        if (icon().isNull()) {
            icon_container->setVisible(false);
        } else {
            ui_icon_->setPixmap(icon().pixmap(18, 18));
            icon_container->setVisible(true);
        }
    });
    connect(this, &TitleBar::on_maximized_change, ui_maximize_, [=, this] {
        if (maximized()) {
            ui_maximize_->setAwesome(ElaIconType::WindowRestore);
        } else {
            ui_maximize_->setAwesome(ElaIconType::Square);
        }
    });
    connect(ui_minimize_, &ElaIconButton::clicked, this, &TitleBar::on_request_minimize);
    connect(ui_maximize_, &ElaIconButton::clicked, this, [this] {
        if (maximized()) {
            emit on_request_restore();
        } else {
            emit on_request_maximize();
        }
    });
    connect(ui_close_, &ElaIconButton::clicked, this, &TitleBar::on_request_close);
}

void TitleBar::set_tracking_widget(QWidget *widget) {
    if (tracking_widget_) { tracking_widget_->removeEventFilter(this); }
    tracking_widget_ = widget;
    if (tracking_widget_) { tracking_widget_->installEventFilter(this); }
}

bool TitleBar::eventFilter(QObject *watched, QEvent *event) {
    if (watched == tracking_widget_) {
        switch (event->type()) {
            case QEvent::Resize:
                [[fallthrough]];
            case QEvent::Show: {
                set_maximized(static_cast<QWidget *>(watched)->isMaximized());
            } break;
            default: {
            } break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

} // namespace UI

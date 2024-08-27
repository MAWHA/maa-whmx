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

#include "NavWidget.h"

#include <QHBoxLayout>

namespace UI {

void NavWidget::add_nav_page_internal(NavNode *node, QWidget *page) {
    Expects(ui_page_container_->indexOf(page) == -1);
    ui_page_container_->addWidget(page);
}

NavWidget::NavWidget(QWidget *parent)
    : QWidget(parent)
    , ui_page_container_(new QStackedWidget)
    , ui_nav_bar_(new NavBar) {
    setup();
}

void NavWidget::setup() {
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(ui_nav_bar_);
    layout->addWidget(ui_page_container_);

    connect(ui_nav_bar_, &NavBar::on_add_nav_page, this, &NavWidget::add_nav_page_internal);
    connect(ui_nav_bar_, &NavBar::on_navigation, ui_page_container_, &QStackedWidget::setCurrentWidget);
}

} // namespace UI

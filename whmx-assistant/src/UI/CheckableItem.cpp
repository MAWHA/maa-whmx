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

#include "CheckableItem.h"

#include <QtWidgets/QHBoxLayout>

namespace UI {

CheckableItem::CheckableItem(const QString &name, QWidget *parent)
    : QWidget(parent)
    , name_(name) {
    setup();
}

QSize CheckableItem::minimumSizeHint() const {
    return QSize(0, 32);
}

QSize CheckableItem::sizeHint() const {
    return minimumSizeHint();
}

void CheckableItem::setup() {
    ui_select_ = new QCheckBox;
    ui_name_   = new QLabel;

    ui_name_->setText(name_);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(ui_select_);
    layout->addWidget(ui_name_);
    layout->addStretch();

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
}

} // namespace UI

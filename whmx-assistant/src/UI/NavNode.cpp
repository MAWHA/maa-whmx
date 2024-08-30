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

#include "NavNode.h"

#include <QtCore/QUuid>
#include <ElaTheme.h>

namespace UI {

void NavNode::set_target(QWidget* widget) {
    if (widget == this->widget()) { return; }
    target_ = QVariant::fromValue(static_cast<QObject*>(widget));
    emit on_target_change(this);
}

void NavNode::set_target(QAction* action) {
    if (action == this->action()) { return; }
    target_ = QVariant::fromValue(static_cast<QObject*>(action));
    emit on_target_change(this);
}

QVariant NavNode::target() const {
    return target_;
}

QWidget* NavNode::widget() const {
    if (target_.isNull()) { return nullptr; }
    Q_ASSERT(target_.typeId() == QMetaType::QObjectStar);
    auto target = target_.value<QObject*>();
    return qobject_cast<QWidget*>(target);
}

QAction* NavNode::action() const {
    if (target_.isNull()) { return nullptr; }
    Q_ASSERT(target_.typeId() == QMetaType::QObjectStar);
    auto target = target_.value<QObject*>();
    return qobject_cast<QAction*>(target);
}

bool NavNode::can_select() const {
    return widget() != nullptr;
}

NavNode::NavNode(ElaIconType::IconName font_icon, const QString& text, QObject* parent)
    : QObject(parent)
    , key_(QUuid::createUuid().toString(QUuid::Id128)) {
    set_font_icon(font_icon);
    set_text(text);
    set_foreground_color(ElaThemeColor(ElaThemeType::Light, WindowText));
}

} // namespace UI

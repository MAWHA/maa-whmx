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

#include "Notification.h"

#include <ElaMessageBar.h>

namespace UI {

void Notification::info(QWidget *parent, const QString &title, const QString &msg) {
    ElaMessageBar::information(ElaMessageBarType::Top, title, msg, 1000, parent->topLevelWidget());
}

void Notification::warning(QWidget *parent, const QString &title, const QString &msg) {
    ElaMessageBar::warning(ElaMessageBarType::Top, title, msg, 1000, parent->topLevelWidget());
}

void Notification::success(QWidget *parent, const QString &title, const QString &msg) {
    ElaMessageBar::success(ElaMessageBarType::Top, title, msg, 1000, parent->topLevelWidget());
}

void Notification::error(QWidget *parent, const QString &title, const QString &msg) {
    ElaMessageBar::error(ElaMessageBarType::Top, title, msg, 1000, parent->topLevelWidget());
}

} // namespace UI

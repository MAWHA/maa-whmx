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

#pragma once

#include "FlatButton.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpinBox>

namespace UI {

QWidget *make_titled_widget(const QString &title, QWidget *widget, QWidget *right_side_widget = nullptr);
void     config_list_widget(QListWidget *widget);
void     config_flat_button(FlatButton *button, const QString &text);
void     config_combo_box(QComboBox *combo, const QString &placeholder_text = QString());
void     config_spin(QSpinBox *spin, int value);
void     append_list_widget_item(QListWidget *list, QWidget *item_widget, const QString &text = QString());

template <typename T>
inline void combo_set_current_item(QComboBox *combo, const T &value) {
    const bool blocked = combo->signalsBlocked();
    bool       found   = false;
    combo->blockSignals(true);
    for (int i = 0; i < combo->count(); ++i) {
        if (combo->itemData(i).value<T>() != value) { continue; }
        combo->setCurrentIndex(i);
        found = true;
        break;
    }
    if (!found) { combo->setCurrentIndex(-1); }
    combo->blockSignals(blocked);
}

} // namespace UI

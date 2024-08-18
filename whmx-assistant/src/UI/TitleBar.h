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

#include "PropertyHelper.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <ElaText.h>
#include <ElaIconButton.h>

namespace UI {

class TitleBar : public QWidget {
    Q_OBJECT

    Q_DECLARE_PROPERTY(QString, title)
    Q_DECLARE_PROPERTY(QIcon, icon)
    Q_DECLARE_PROPERTY(bool, maximized)

signals:
    void on_request_minimize();
    void on_request_maximize();
    void on_request_restore();
    void on_request_close();

public:
    QHBoxLayout *tool_button_layout() {
        return ui_tool_button_layout_;
    }

    QPushButton *minimize_button() {
        return ui_minimize_;
    }

    QPushButton *maximize_button() {
        return ui_maximize_;
    }

    QPushButton *close_button() {
        return ui_close_;
    }

    void set_tracking_widget(QWidget *widget);

public:
    TitleBar(QWidget *parent = nullptr);

protected:
    void setup();

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QWidget       *tracking_widget_       = nullptr;
    ElaIconButton *ui_minimize_           = nullptr;
    ElaIconButton *ui_maximize_           = nullptr;
    ElaIconButton *ui_close_              = nullptr;
    QHBoxLayout   *ui_tool_button_layout_ = nullptr;
};

} // namespace UI

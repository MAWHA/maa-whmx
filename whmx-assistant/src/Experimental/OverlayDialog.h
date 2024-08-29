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

#include "../UI/PropertyHelper.h"

#include <QtWidgets/QWidget>
#include <QtCore/QEventLoop>
#include <gsl/gsl>

namespace Experimental {

class OverlayDialog : public QWidget {
    Q_OBJECT

    Q_DECLARE_PROPERTY(double, opacity)
    Q_DECLARE_PROPERTY(int, result)

signals:
    void accepted();
    void rejected();
    void closed(int result);

public slots:
    void accept();
    void reject();
    void quit();
    void exit(int result);

public:
    int  exec(gsl::not_null<QWidget *> parent);
    void set_widget(gsl::not_null<QWidget *> widget);

    bool is_accepted() const {
        return accepted_.value_or(false);
    }

    bool is_rejected() const {
        return !accepted_.value_or(true);
    }

public:
    OverlayDialog();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    gsl::owner<QEventLoop *> ev_     = nullptr;
    gsl::owner<QWidget *>    widget_ = nullptr;
    std::optional<bool>      accepted_;
    gsl::not_null<QLayout *> ui_layout_;
};

} // namespace Experimental

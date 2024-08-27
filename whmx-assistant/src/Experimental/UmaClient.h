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

#include "../UI/TitleBar.h"
#include "../UI/NavWidget.h"
#include "../UI/Card.h"

#include <QtWidgets/QWidget>
#include <gsl/gsl>

namespace Experimental {

class UmaClient : public QWidget {
    Q_OBJECT

public:
    enum PrimaryPage {
        Workstation,
        Package,
        Device,
        Settings,
    };

signals:
    void on_request_new_uma_instance();
    void on_add_uma_instance(const QString &id);
    void on_remove_uma_instance(const QString &id);

protected slots:
    void show_uma_card_context_menu(const QString &id, const QPoint &pos);

public:
    static bool initialized();
    static auto create_or_get() -> gsl::not_null<std::shared_ptr<UmaClient>>;

    void navigate(PrimaryPage page);

protected:
    auto make_card_for_uma(const QString &id) -> gsl::strict_not_null<UI::Card *>;

protected:
    UmaClient();

    void setup();

private:
    QMap<QString, QWidget *>                          card_for_uma_instance_;
    QMap<PrimaryPage, QString>                        primary_page_nav_keys_;
    gsl::strict_not_null<gsl::owner<UI::TitleBar *>>  ui_title_bar_;
    gsl::strict_not_null<gsl::owner<UI::NavWidget *>> ui_nav_widget_;
};

} // namespace Experimental

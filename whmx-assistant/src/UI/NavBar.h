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

#include "NavModel.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QListView>

namespace UI {

class NavBar : public QWidget {
    Q_OBJECT

signals:
    void on_navigation(QWidget *widget);
    void on_add_nav_page(NavNode *node, QWidget *widget);

public:
    void add_page_node(ElaIconType::IconName icon, const QString &text, QWidget *widget);
    void add_page_node(ElaIconType::IconName icon, QWidget *widget);
    void add_page_node(ElaIconType::IconName icon, const QString &text, QAction *action);
    void add_page_node(ElaIconType::IconName icon, QAction *action);
    void add_page_node(NavNode *node);

    void add_footer_node(ElaIconType::IconName icon, const QString &text, QWidget *widget);
    void add_footer_node(ElaIconType::IconName icon, QWidget *widget);
    void add_footer_node(ElaIconType::IconName icon, const QString &text, QAction *action);
    void add_footer_node(ElaIconType::IconName icon, QAction *action);
    void add_footer_node(NavNode *node);

public slots:
    bool navigate(const QString &key);

public:
    NavBar(QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void setup();

    void paintEvent(QPaintEvent *event) override;

private:
    NavModel  *default_model_       = nullptr;
    NavModel  *footer_model_        = nullptr;
    QListView *ui_default_view_     = nullptr;
    QListView *ui_footer_view_      = nullptr;
    QWidget   *ui_footer_container_ = nullptr;
};

} // namespace UI

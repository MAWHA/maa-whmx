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

#include "NavBar.h"
#include "NavItemDelegate.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QScroller>
#include <QtWidgets/QScrollbar>
#include <QtWidgets/QScrollerProperties>
#include <QtGui/QPainter>
#include <ElaTheme.h>

namespace UI {

void NavBar::add_page_node(ElaIconType::IconName icon, const QString &text, QWidget *widget) {
    auto node = new NavNode(icon, text, this);
    node->set_target(widget);
    add_page_node(node);
}

void NavBar::add_page_node(ElaIconType::IconName icon, QWidget *widget) {
    auto node = new NavNode(icon, "", this);
    node->set_target(widget);
    add_page_node(node);
}

void NavBar::add_page_node(ElaIconType::IconName icon, const QString &text, QAction *action) {
    auto node = new NavNode(icon, text, this);
    node->set_target(action);
    add_page_node(node);
}

void NavBar::add_page_node(ElaIconType::IconName icon, QAction *action) {
    auto node = new NavNode(icon, "", this);
    node->set_target(action);
    add_page_node(node);
}

void NavBar::add_page_node(NavNode *node) {
    default_model_->add_node(node);
    if (auto w = node->widget()) { emit on_add_nav_page(node, w); }
}

void NavBar::add_footer_node(ElaIconType::IconName icon, const QString &text, QWidget *widget) {
    auto node = new NavNode(icon, text, this);
    node->set_target(widget);
    add_footer_node(node);
}

void NavBar::add_footer_node(ElaIconType::IconName icon, QWidget *widget) {
    auto node = new NavNode(icon, "", this);
    node->set_target(widget);
    add_footer_node(node);
}

void NavBar::add_footer_node(ElaIconType::IconName icon, const QString &text, QAction *action) {
    auto node = new NavNode(icon, text, this);
    node->set_target(action);
    add_footer_node(node);
}

void NavBar::add_footer_node(ElaIconType::IconName icon, QAction *action) {
    auto node = new NavNode(icon, "", this);
    node->set_target(action);
    add_footer_node(node);
}

void NavBar::add_footer_node(NavNode *node) {
    const bool show_footer = footer_model_->rowCount() == 0;
    footer_model_->add_node(node);
    if (show_footer) { ui_footer_container_->setVisible(true); }
    if (auto w = node->widget()) { emit on_add_nav_page(node, w); }
}

bool NavBar::navigate(const QString &key) {
    NavNode  *node  = nullptr;
    NavModel *model = nullptr;
    do {
        model = default_model_;
        node  = model->node(key);
        if (node) { break; }
        model = footer_model_;
        node  = model->node(key);
    } while (0);
    if (!node) { return false; }
    if (!node->can_select()) { return false; }
    if (node == model->selected_node()) { return true; }
    model->activate_node(node);
    return true;
}

NavBar::NavBar(QWidget *parent)
    : QWidget(parent) {
    setup();
}

QSize NavBar::minimumSizeHint() const {
    const int  item_width = 48;
    const auto margins    = contentsMargins();
    const int  minw       = item_width + margins.left() + margins.right();
    return QSize(minw, 80);
}

QSize NavBar::sizeHint() const {
    return minimumSizeHint();
}

void NavBar::setup() {
    default_model_       = new NavModel;
    footer_model_        = new NavModel;
    ui_default_view_     = new QListView;
    ui_footer_view_      = new QListView;
    ui_footer_container_ = new QWidget;

    {
        ui_default_view_->setFixedWidth(40);
        ui_default_view_->setStyleSheet("QListView{background-color:transparent;border:0px;}"
                                        "QListView::item{border:none;}");
        ui_default_view_->setSelectionMode(QAbstractItemView::NoSelection);
        ui_default_view_->setModel(default_model_);
        ui_default_view_->setItemDelegate(new NavItemDelegate(ui_default_view_, ui_default_view_));

        ui_default_view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui_default_view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui_default_view_->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        ui_default_view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

        QScroller::grabGesture(ui_default_view_->viewport(), QScroller::LeftMouseButtonGesture);
        auto properties = QScroller::scroller(ui_default_view_->viewport())->scrollerProperties();
        properties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
        properties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOn);
        properties.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.35);
        properties.setScrollMetric(QScrollerProperties::OvershootScrollTime, 0.5);
        properties.setScrollMetric(QScrollerProperties::FrameRate, QScrollerProperties::Fps60);
        QScroller::scroller(ui_default_view_->viewport())->setScrollerProperties(properties);

        ui_default_view_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    }

    {
        ui_footer_view_->setFixedWidth(40);
        ui_footer_view_->setStyleSheet("QListView{background-color:transparent;border:0px;}"
                                       "QListView::item{border:none;}");
        ui_footer_view_->setSelectionMode(QAbstractItemView::NoSelection);
        ui_footer_view_->setModel(footer_model_);
        ui_footer_view_->setItemDelegate(new NavItemDelegate(ui_footer_view_, ui_footer_view_));

        ui_footer_view_->horizontalScrollBar()->setEnabled(false);
        ui_footer_view_->verticalScrollBar()->setEnabled(false);
        ui_footer_view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui_footer_view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui_footer_view_->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    }

    {
        auto layout = new QVBoxLayout(ui_footer_container_);
        ui_footer_container_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addSpacing(11);
        layout->addWidget(ui_footer_view_);
    }

    auto layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignHCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(ui_default_view_);
    layout->addWidget(ui_footer_container_);

    ui_footer_container_->setVisible(false);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

    connect(ui_default_view_, &QListView::clicked, [this](const QModelIndex &index) {
        default_model_->activate_node(index.data(Qt::UserRole).value<UI::NavNode *>());
    });
    connect(ui_footer_view_, &QListView::clicked, [this](const QModelIndex &index) {
        footer_model_->activate_node(index.data(Qt::UserRole).value<UI::NavNode *>());
    });
    connect(default_model_, &NavModel::on_selected_node_change, footer_model_, [this] {
        ui_default_view_->update();
        const auto node = default_model_->selected_node();
        if (!node) { return; }
        footer_model_->clear_selection();
        emit on_navigation(node->widget());
    });
    connect(footer_model_, &NavModel::on_selected_node_change, default_model_, [this] {
        ui_footer_view_->update();
        const auto node = footer_model_->selected_node();
        if (!node) { return; }
        default_model_->clear_selection();
        emit on_navigation(node->widget());
    });
}

void NavBar::paintEvent(QPaintEvent *event) {
    QPainter p(this);

    if (ui_footer_container_->isVisible()) {
        const int sep_space  = 11;
        const int sep_height = 3;
        auto      space_bb   = ui_footer_view_->rect().translated(ui_footer_view_->mapTo(this, QPoint()));
        space_bb.adjust(8, 0, -8, 0);
        space_bb.translate(0, -(sep_space + sep_height) / 2);
        space_bb.setHeight(sep_height);
        p.save();
        p.setRenderHints(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#e3e6e8"));
        p.drawRoundedRect(space_bb, sep_height, sep_height);
        p.restore();
    }

    QWidget::paintEvent(event);
}

} // namespace UI

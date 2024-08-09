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

#include "Helper.h"
#include "Scrollbar.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QProxyStyle>

namespace UI {

class QListWidgetProxyStyle : public QProxyStyle {
public:
    QListWidgetProxyStyle(QStyle *style = nullptr)
        : QProxyStyle(style) {}

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr)
        const override {
        if (element == PE_PanelItemViewItem) { return; }
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
};

QWidget *make_titled_widget(const QString &title, QWidget *widget, QWidget *right_side_widget) {
    auto label = new QLabel(title);
    {
        auto font = label->font();
        font.setBold(true);
        label->setFont(font);
    }
    QWidget *head_widget = label;
    if (right_side_widget) {
        head_widget = new QWidget;
        {
            auto layout = new QHBoxLayout(head_widget);
            layout->setContentsMargins({});
            layout->addWidget(label);
            layout->addStretch();
            layout->addWidget(right_side_widget);
        }
    }
    auto container = new QWidget;
    auto layout    = new QVBoxLayout(container);
    layout->setContentsMargins({});
    layout->addWidget(head_widget);
    layout->addWidget(widget);
    return container;
}

void config_list_widget(QListWidget *widget) {
    widget->setVerticalScrollBar(new Scrollbar);
    widget->setHorizontalScrollBar(new Scrollbar);
    widget->setFrameShadow(QFrame::Raised);
    widget->setFrameShape(QFrame::Box);
    widget->setResizeMode(QListView::Adjust);
    widget->setStyle(new QListWidgetProxyStyle(widget->style()));
    widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
}

void config_flat_button(FlatButton *button, const QString &text) {
    button->set_text(text);
    button->set_border_visible(true);
    button->set_radius(4);
    {
        auto pal = button->palette();
        pal.setColor(QPalette::Window, "#e0e0e0"); //<! border
        pal.setColor(QPalette::Base, "#ffffff");   //<! background
        pal.setColor(QPalette::Button, "#f4f4f4"); //<! mouse-on background
        button->setPalette(pal);
    }
}

void config_combo_box(QComboBox *combo, const QString &placeholder_text) {
    for (int i = 0; i < combo->count(); ++i) { combo->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole); }
    if (!placeholder_text.isEmpty()) { combo->setPlaceholderText(placeholder_text); }
    combo->view()->setVerticalScrollBar(new Scrollbar);
    combo->setStyleSheet(R"(
    QComboBox {
        color: #666666;
        padding: 4px 10px 4px 10px;
        border: 1px solid #e4e4e4;
        border-radius: 5px 5px 0px 0px;
    }
    QComboBox::drop-down {
        subcontrol-origin: padding;
        subcontrol-position: top right;
        width: 0px;
        border: none;
    }
    QComboBox QAbstractItemView {
        background: #ffffff;
        border: 1px solid #e4e4e4;
        border-radius: 0px 0px 5px 5px;
        outline: 0px;
    }
    QComboBox QAbstractItemView::item {
        height: 32px;
        color: #666666;
        background-color: #ffffff;
        padding: 0px 8px 0px 8px;
    }
    QComboBox QAbstractItemView::item:hover {
        background-color: #409ce1;
        color: #ffffff;
    }
    QComboBox QAbstractItemView::item:selected {
        background-color: #409ce1;
        color: #ffffff;
    }
    )");
}

void config_spin(QSpinBox *spin, int value) {
    spin->setValue(qBound(spin->minimum(), value, spin->maximum()));
    spin->setStyleSheet(R"(
    QSpinBox {
        font-size: 12pt;
        padding: 4px 10px 4px 10px;
        color: #000000;
        background-color: #ffffff;
        border: 1px solid rgba(228,228,228,1);
        border-radius: 5px 5px 0px 0px;
    }
    )");
    spin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    spin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spin->setAlignment(Qt::AlignCenter);
}

void append_list_widget_item(QListWidget *list, QWidget *item_widget, const QString &text) {
    auto item = new QListWidgetItem(nullptr, QListWidgetItem::UserType);
    item->setSizeHint(item_widget->sizeHint());
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsUserCheckable);
    if (!text.isEmpty()) { item->setText(text); }
    list->addItem(item);
    list->setItemWidget(item, item_widget);
}

} // namespace UI

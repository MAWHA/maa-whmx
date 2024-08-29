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

#include "OverlayDialog.h"

#include <QtCore/QPropertyAnimation>
#include <QtCore/QEventLoop>
#include <QtGui/QtEvents>
#include <QtGui/QPainter>
#include <QtWidgets/QVBoxLayout>

namespace Experimental {

void OverlayDialog::accept() {
    Expects(ev_);
    accepted_ = true;
    quit();
    emit accepted();
}

void OverlayDialog::reject() {
    Expects(ev_);
    accepted_ = true;
    quit();
    emit rejected();
}

void OverlayDialog::quit() {
    exit(0);
}

void OverlayDialog::exit(int result) {
    Expects(ev_);
    set_result(result);

    auto anim_exit = new QPropertyAnimation(this, "opacity");
    connect(anim_exit, &QPropertyAnimation::valueChanged, this, QOverload<>::of(&QWidget::update));
    connect(anim_exit, &QPropertyAnimation::finished, this, [=, this] {
        close();
        ev_->exit();
        emit closed(result);
    });
    anim_exit->setDuration(100);
    anim_exit->setStartValue(opacity());
    anim_exit->setEndValue(0.0);
    anim_exit->start(QAbstractAnimation::DeleteWhenStopped);
}

int OverlayDialog::exec(gsl::not_null<QWidget *> parent) {
    Expects(widget_);

    //! NOTE: force to trigger the event filter
    setParent(parent);
    show();

    QEventLoop ev;
    ev_ = std::addressof(ev);
    ev_->exec();
    ev_ = nullptr;

    return result();
}

void OverlayDialog::set_widget(gsl::not_null<QWidget *> widget) {
    Expects(widget);
    if (widget_) {
        ui_layout_->removeWidget(widget_);
        widget_->deleteLater();
    }
    widget_ = widget;
    ui_layout_->addWidget(widget_);
}

OverlayDialog::OverlayDialog()
    : ui_layout_(new QHBoxLayout(this)) {
    ui_layout_->setContentsMargins(0, 0, 0, 0);
    ui_layout_->setSpacing(0);
    ui_layout_->setAlignment(Qt::AlignCenter);
    set_opacity(0.4);
}

bool OverlayDialog::eventFilter(QObject *obj, QEvent *event) {
    Expects(obj && obj == parentWidget());
    switch (event->type()) {
        case QEvent::Resize: {
            setGeometry(parentWidget()->rect());
        } break;
        default: {
        } break;
    }
    return false;
}

bool OverlayDialog::event(QEvent *event) {
    switch (event->type()) {
        case QEvent::ParentAboutToChange: {
            if (const auto w = parentWidget()) { w->removeEventFilter(this); }
        } break;
        case QEvent::ParentChange: {
            if (const auto w = parentWidget()) {
                w->installEventFilter(this);
                setGeometry(w->rect());
            }
        } break;
        case QEvent::Show: {
            auto anim_popup = new QPropertyAnimation(this, "opacity");
            connect(anim_popup, &QPropertyAnimation::valueChanged, this, QOverload<>::of(&QWidget::update));
            anim_popup->setDuration(100);
            anim_popup->setStartValue(0.0);
            anim_popup->setEndValue(opacity());
            anim_popup->start(QAbstractAnimation::DeleteWhenStopped);
        } break;
        default: {
        } break;
    }
    return QWidget::event(event);
}

void OverlayDialog::paintEvent(QPaintEvent *event) {
    QPainter p(this);

    QColor overlay_color = Qt::black;
    overlay_color.setAlpha(opacity() * 255);

    p.fillRect(rect(), overlay_color);
}

} // namespace Experimental

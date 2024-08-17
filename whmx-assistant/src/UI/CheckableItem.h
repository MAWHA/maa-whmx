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

#include "ElidedLabel.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>

namespace UI {

class CheckableItem : public QWidget {
    Q_OBJECT

public:
    QString name() const {
        return name_;
    }

    bool checked() const {
        return ui_select_->isChecked();
    }

    void set_checked(bool on) {
        ui_select_->setChecked(on);
    }

public:
    CheckableItem(const QString &name, QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void setup();

    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    const QString name_;
    QCheckBox    *ui_select_ = nullptr;
    ElidedLabel  *ui_name_   = nullptr;
};

} // namespace UI

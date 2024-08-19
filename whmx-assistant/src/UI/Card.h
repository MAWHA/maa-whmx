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

namespace UI {

class Card : public QWidget {
    Q_OBJECT

    Q_DECLARE_PROPERTY(double, hover_offset)
    Q_DECLARE_PROPERTY(double, hover_opacity)
    Q_DECLARE_PROPERTY(int, border_radius)
    Q_DECLARE_PROPERTY(QColor, pin_color)
    Q_DECLARE_PROPERTY(QPixmap, card_pixmap)
    Q_DECLARE_PROPERTY(QString, title)
    Q_DECLARE_PROPERTY(QString, brief)

signals:
    void clicked();

public:
    Card(QWidget* parent = nullptr);

protected:
    bool event(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
};

} // namespace UI

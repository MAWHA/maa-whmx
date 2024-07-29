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

#include <QtWidgets/QWidget>

namespace UI {

class FlatButton : public QWidget {
    Q_OBJECT

public:
    void set_border_visible(bool visble);

    QString text() const {
        return text_;
    }

    void set_text(const QString &text);

    int radius() const {
        return ui_radius_;
    }

    void set_radius(int radius);

    Qt::Alignment text_alignment() {
        return ui_alignment_;
    }

    void set_text_alignment(Qt::Alignment alignment);

signals:
    void pressed();

public:
    FlatButton(QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void setupUi();

    void paintEvent(QPaintEvent *e) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;

private:
    QString text_;

    bool          border_visible_;
    int           ui_radius_;
    Qt::Alignment ui_alignment_;
    bool          ui_mouse_entered_;
};

} // namespace UI

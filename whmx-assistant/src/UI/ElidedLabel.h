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

#include <QtWidgets/QLabel>

namespace UI {

class ElidedLabel : public QLabel {
    Q_OBJECT

public:
    void set_elide_mode(Qt::TextElideMode elide_mode);

protected:
    void refresh_elided_text();

public:
    ElidedLabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ElidedLabel(const QString &text, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Qt::TextElideMode elide_mode_;
    QString           cached_elided_text_;
    QString           cached_text_;
};

} // namespace UI

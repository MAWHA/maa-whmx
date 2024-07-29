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

#include "ClickableLabel.h"

namespace UI {

class IconButton : public ClickableLabel {
    Q_OBJECT

public:
    int  get_icon_size() const;
    void set_icon_size(int size);
    void set_icon(const QString& default_icon, const QString& hover_icon = QString());
    void reload_icon();
    void set_action(std::unique_ptr<QAction> action);
    void reset_action();
    void set_tool_tip(const QString& tip);

public slots:
    void trigger();

public:
    IconButton(const QString& default_icon, const QString& hover_icon = QString(), QWidget* parent = nullptr);
    ~IconButton() override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QAction* action_;
    QString  default_icon_name_;
    QString  hover_icon_name_;
    int      icon_size_;
    QPixmap  ui_icon_;
    bool     ui_hover_;
};

} // namespace UI

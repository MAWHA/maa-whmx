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

#include <QtCore/QObject>
#include <QtCore/QModelIndex>
#include <QtCore/QVariant>
#include <QtWidgets/QWidget>
#include <QtGui/QAction>
#include <ElaIcon.h>

namespace UI {

class NavNode : public QObject {
    Q_OBJECT

    Q_DECLARE_PROPERTY(QString, text)
    Q_DECLARE_PROPERTY(QColor, foreground_color)
    Q_DECLARE_PROPERTY(ElaIconType::IconName, font_icon)

signals:
    void on_target_change(NavNode* node);

public:
    QString key() const {
        return key_;
    }

    void set_target(QWidget* widget);
    void set_target(QAction* action);

    QVariant target() const;

    QWidget* widget() const;
    QAction* action() const;

    bool can_select() const;

public:
    NavNode(QObject* parent = nullptr)
        : NavNode(ElaIconType::None, "", parent) {}

    NavNode(const QString& text, QObject* parent = nullptr)
        : NavNode(ElaIconType::None, text, parent) {}

    NavNode(ElaIconType::IconName font_icon, QObject* parent = nullptr)
        : NavNode(font_icon, "", parent) {}

    NavNode(ElaIconType::IconName font_icon, const QString& text, QObject* parent = nullptr);

private:
    const QString key_;
    QVariant      target_;
};

} // namespace UI

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
#include <gsl/gsl>

namespace UI {

class Notification {
public:
    static void info(gsl::not_null<const QWidget *> parent, const QString &title, const QString &msg);
    static void warning(gsl::not_null<const QWidget *> parent, const QString &title, const QString &msg);
    static void success(gsl::not_null<const QWidget *> parent, const QString &title, const QString &msg);
    static void error(gsl::not_null<const QWidget *> parent, const QString &title, const QString &msg);
};

} // namespace UI

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

#include <QObject>

class AppEvent : public QObject {
    Q_OBJECT

signals:
    void on_set_window_top(bool on);
    void on_minimize();
    void on_maximize();
    void on_restore();
    void on_close();
    void on_exit();

public:
    AppEvent(QObject *parent = nullptr)
        : QObject(parent) {}
};

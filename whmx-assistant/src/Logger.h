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

#include <QtCore/QtLogging>
#include <QtCore/QTemporaryFile>
#include <QtCore/QFile>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QTimer>
#include <stdio.h>
#include <memory>
#include <array>

class GlobalLoggerProxy final : public QObject {
    Q_OBJECT

public:
    static std::shared_ptr<GlobalLoggerProxy> instance();

    static void setup();

private:
    GlobalLoggerProxy();

    void inject_logger_proxy();
    void cleanup();

private slots:
    void post_hooked_logger_message(QString message);

private:
    struct Redirection {
        FILE                           *file;
        std::unique_ptr<QFile>          file_track;
        int                             saved_fd;
        std::unique_ptr<QTemporaryFile> redirection;
    };

    bool                       injected_;
    QFileSystemWatcher         watcher_;
    std::array<Redirection, 2> redirections_;
};

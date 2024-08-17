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

#include "MacroHelper.h"

#include <QtCore/QtLogging>
#include <QtCore/QTemporaryFile>
#include <QtCore/QFile>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QTimer>
#include <stdio.h>
#include <memory>
#include <array>
#include <QtCore/QLoggingCategory>

#define LOG_ON_LEVEL_IMPL0(level, ...)           MH_CONCAT(qC, level)(::LogCategory::AppRuntime)
#define LOG_ON_LEVEL_IMPL1(level, category, ...) MH_CONCAT(qC, level)(::LogCategory::category)
#define LOG_ON_LEVEL_IMPL(N, level, ...)         MH_EXPAND(MH_CONCAT(LOG_ON_LEVEL_IMPL, N)(level, __VA_ARGS__))
#define LOG_ON_LEVEL(level, ...)                 MH_EXPAND(LOG_ON_LEVEL_IMPL(MH_EXPAND(MH_NARG(__VA_ARGS__)), level, __VA_ARGS__))

#define LOG_TRACE(...) LOG_ON_LEVEL(Debug, __VA_ARGS__)
#define LOG_INFO(...)  LOG_ON_LEVEL(Info, __VA_ARGS__)
#define LOG_WARN(...)  LOG_ON_LEVEL(Warning, __VA_ARGS__)
#define LOG_ERROR(...) LOG_ON_LEVEL(Critical, __VA_ARGS__)

namespace LogCategory {
Q_DECLARE_LOGGING_CATEGORY(AppRuntime)
Q_DECLARE_LOGGING_CATEGORY(Workstation)
} // namespace LogCategory

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

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

#define PATCHED_MESSAGE_LOGGER_COMMON(category, level, patched_info)                                             \
    for (QLoggingCategoryMacroHolder<level> qt_category((category)()); qt_category; qt_category.control = false) \
    ::LoggerImpl::PatchedMessageLogger(                                                                          \
        patched_info, QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC, qt_category.name())

#define qCDebugPatched(category, patched_info, ...) \
    PATCHED_MESSAGE_LOGGER_COMMON(category, QtDebugMsg, patched_info).debug(__VA_ARGS__)
#define qCInfoPatched(category, patched_info, ...) \
    PATCHED_MESSAGE_LOGGER_COMMON(category, QtInfoMsg, patched_info).info(__VA_ARGS__)
#define qCWarningPatched(category, patched_info, ...) \
    PATCHED_MESSAGE_LOGGER_COMMON(category, QtWarningMsg, patched_info).warning(__VA_ARGS__)
#define qCCriticalPatched(category, patched_info, ...) \
    PATCHED_MESSAGE_LOGGER_COMMON(category, QtCriticalMsg).critical(__VA_ARGS__)

#define LOG_ON_LEVEL_IMPL0(level, ...)           MH_CONCAT(qC, level)(::LoggerImpl::AppRuntime)
#define LOG_ON_LEVEL_IMPL1(level, category, ...) MH_CONCAT(qC, level)(::LoggerImpl::category)
#define LOG_ON_LEVEL_IMPL2(level, category, patched_info, ...) \
    MH_CONCAT(qC, MH_CONCAT(level, Patched))(::LoggerImpl::category, patched_info)
#define LOG_ON_LEVEL_IMPL(N, level, ...) MH_EXPAND(MH_CONCAT(LOG_ON_LEVEL_IMPL, N)(level, __VA_ARGS__))
#define LOG_ON_LEVEL(level, ...)         MH_EXPAND(LOG_ON_LEVEL_IMPL(MH_EXPAND(MH_NARG(__VA_ARGS__)), level, __VA_ARGS__))

#define LOG_TRACE(...) LOG_ON_LEVEL(Debug, __VA_ARGS__)
#define LOG_INFO(...)  LOG_ON_LEVEL(Info, __VA_ARGS__)
#define LOG_WARN(...)  LOG_ON_LEVEL(Warning, __VA_ARGS__)
#define LOG_ERROR(...) LOG_ON_LEVEL(Critical, __VA_ARGS__)

namespace LoggerImpl {

Q_DECLARE_LOGGING_CATEGORY(AppRuntime)
Q_DECLARE_LOGGING_CATEGORY(Workstation)

struct StealDebugStreamTag {};

auto steal_debug_stream(QDebug &debug);

template <typename Tag, typename Fn, Fn member>
class StealDebugStreamImpl {
    friend auto steal_debug_stream(QDebug &debug) {
        return debug.*member;
    }
};

template class StealDebugStreamImpl<StealDebugStreamTag, decltype(&QDebug::stream), &QDebug::stream>;

class PatchedDebug : public QDebug {
public:
    PatchedDebug(const QString &patched_info, QDebug &&debug)
        : QDebug(debug)
        , patched_info_(std::move(patched_info)) {}

    PatchedDebug &space() {
        QDebug::space();
        return *this;
    }

    PatchedDebug &nospace() {
        QDebug::nospace();
        return *this;
    }

    PatchedDebug &maybeSpace() {
        QDebug::maybeSpace();
        return *this;
    }

    PatchedDebug &verbosity(int verbosityLevel) {
        QDebug::verbosity(verbosityLevel);
        return *this;
    }

    PatchedDebug &quote() {
        QDebug::quote();
        return *this;
    }

    PatchedDebug &noquote() {
        QDebug::noquote();
        return *this;
    }

    PatchedDebug &maybeQuote(char c = '"') {
        QDebug::maybeQuote(c);
        return *this;
    }

    template <typename T>
    PatchedDebug &operator<<(const T &value) {
        QDebug::operator<<(value);
        return *this;
    }

    ~PatchedDebug() {
        steal_debug_stream(*this)->buffer.prepend(QString("\x10%1\x02").arg(patched_info_));
        QDebug::~QDebug();
    }

private:
    QString patched_info_;
};

class PatchedMessageLogger : public QMessageLogger {
public:
    PatchedMessageLogger(const QString &patched_info, const char *file, int line, const char *function, const char *category)
        : QMessageLogger(file, line, function, category)
        , patched_info_(patched_info) {}

    PatchedDebug debug() const {
        return PatchedDebug(patched_info_, QMessageLogger::debug());
    }

    PatchedDebug info() const {
        return PatchedDebug(patched_info_, QMessageLogger::info());
    }

    PatchedDebug warning() const {
        return PatchedDebug(patched_info_, QMessageLogger::warning());
    }

    PatchedDebug critical() const {
        return PatchedDebug(patched_info_, QMessageLogger::critical());
    }

private:
    QString patched_info_;
};

} // namespace LoggerImpl

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

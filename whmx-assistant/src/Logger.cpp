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

#include "Logger.h"
#include "UI/LogPanel.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <spdlog/spdlog.h>
#include <mutex>
#include <array>
#include <io.h>

namespace fs = std::filesystem;

namespace LoggerImpl {
Q_LOGGING_CATEGORY(AppRuntime, "AppRuntime")
Q_LOGGING_CATEGORY(Workstation, "Workstation")
} // namespace LoggerImpl

void global_logger_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    static std::mutex LOGGER_LOCK;
    std::lock_guard   lock(LOGGER_LOCK);

    const auto log_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    std::optional<QString> opt_patched_info;
    QString                msg_text;
    if (const char data_link_escape_ctrl = '\x10'; msg.startsWith(data_link_escape_ctrl)) {
        const char start_of_text_ctrl = '\x02';
        Q_ASSERT(msg.count(start_of_text_ctrl) == 1);
        const int text_start = msg.indexOf(start_of_text_ctrl) + 1;
        msg_text             = msg.mid(text_start);
        opt_patched_info     = msg.mid(1, text_start - 2);
    } else {
        msg_text = msg;
    }

    if (false) {
    } else if (strcmp(context.category, "AppRuntime") == 0) {
        switch (type) {
            case QtDebugMsg: {
                spdlog::trace(msg_text.toUtf8().toStdString());
            } break;
            case QtInfoMsg: {
                spdlog::info(msg_text.toUtf8().toStdString());
            } break;
            case QtWarningMsg: {
                spdlog::warn(msg_text.toUtf8().toStdString());
            } break;
            case QtCriticalMsg:
                [[fallthrough]];
            case QtFatalMsg: {
                spdlog::error(msg_text.toUtf8().toStdString());
            } break;
        }
    } else if (strcmp(context.category, "Workstation") == 0) {
        const auto info = QString("%1 %2\n").arg(log_time).arg(msg_text);
        for (auto logger : UI::LogPanel::global_logger_panels()) { UI::LogPanel::log(logger, info); }
    }
}

namespace {
struct TranslationUnitTag {};
} // namespace

static void destruct_logger_proxy(GlobalLoggerProxy *proxy);

template <typename Tag, typename Fn, Fn method>
class PrivateMethodInvokeImpl {
    friend void destruct_logger_proxy(GlobalLoggerProxy *proxy) {
        (proxy->*method)();
    }
};

template class PrivateMethodInvokeImpl<TranslationUnitTag, decltype(&GlobalLoggerProxy::cleanup), &GlobalLoggerProxy::cleanup>;

std::shared_ptr<GlobalLoggerProxy> GlobalLoggerProxy::instance() {
    static std::shared_ptr<GlobalLoggerProxy> instance;
    if (!instance) { instance.reset(new GlobalLoggerProxy, destruct_logger_proxy); }
    return instance;
}

void GlobalLoggerProxy::setup() {
    instance()->inject_logger_proxy();
}

GlobalLoggerProxy::GlobalLoggerProxy()
    : injected_(false) {
    redirections_[0].file = stdout;
    redirections_[1].file = stderr;
}

void GlobalLoggerProxy::inject_logger_proxy() {
    if (injected_) { return; }

    //! FIXME: the current implementation do not guarantee the order of the log messages, which may cause some confusion.

    //! FIXME: on the windows platform, the redirection is not working correctly to the output from the dll. BUT, ehh, sometimes
    //! it may work well, but I don't know why it works well or why it doesn't.

    for (auto &[file, file_track, saved_fd, redirection] : redirections_) {
        redirection = std::make_unique<QTemporaryFile>();
        redirection->setAutoRemove(true);
        redirection->open();

        saved_fd = dup(fileno(file));
        freopen(redirection->fileName().toLocal8Bit().data(), "a", file);

        file_track = std::make_unique<QFile>(redirection->fileName());
        file_track->open(QIODevice::ReadOnly | QIODevice::Text);

        //! move the cursor to the end of the file to skip the old messages
        file_track->seek(file_track->size());

        watcher_.addPath(redirection->fileName());
        connect(&watcher_, &QFileSystemWatcher::fileChanged, this, [this, file_track = file_track.get()](const QString &path) {
            if (path != file_track->fileName()) { return; }
            if (const auto text = file_track->readAll(); !text.isEmpty()) { post_hooked_logger_message(file_track->readAll()); }
        });
    }

    qInstallMessageHandler(global_logger_handler);

    injected_ = true;
}

void GlobalLoggerProxy::cleanup() {
    if (!injected_) { return; }

    disconnect(&watcher_, &QFileSystemWatcher::fileChanged, this, nullptr);

    for (auto &[file, file_track, saved_fd, redirection] : redirections_) {
        watcher_.removePath(redirection->fileName());
        file_track.reset();
        //! FIXME: restore stdout by dup2 would cause the assertion failure for the gui application in debug mode
#if 0
        dup2(saved_fd, fileno(file));
#endif
        redirection.reset();
    }

    qInstallMessageHandler(nullptr);

    injected_ = false;
}

void GlobalLoggerProxy::post_hooked_logger_message(QString message) {
    LOG_INFO().noquote() << message;
}

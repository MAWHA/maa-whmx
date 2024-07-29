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
#include <mutex>
#include <string_view>
#include <array>
#include <io.h>

namespace fs = std::filesystem;

std::mutex LOGGER_LOCK;

void global_logger_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    static QMap<QtMsgType, QString> LEVEL_TABLE{
        {QtDebugMsg,    "DBG"},
        {QtInfoMsg,     "INF"},
        {QtWarningMsg,  "WRN"},
        {QtCriticalMsg, "ERR"},
        {QtFatalMsg,    "FTL"},
    };

    std::lock_guard lock(LOGGER_LOCK);

    if (msg.isEmpty()) { return; }

    QString log_text;

    //! NOTE: QMessageLogContext is not available in release config

#ifdef NDEBUG
    {
        log_text = QString("%1 [%2] %3\n")
                       .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
                       .arg(LEVEL_TABLE.value(type, "UNK"))
                       .arg(msg);
    }
#else
    {
        std::array<std::string_view, 2> repo_src_dirs{"/maa-whmx/whmx-assistant/src/", "\\maa-whmx\\whmx-assistant\\src\\"};
        std::string_view                path(context.file);

        auto file = QString::fromLocal8Bit(path);
        for (const auto &dir : repo_src_dirs) {
            const auto it = path.find(dir);
            if (it == path.npos) { continue; }
            file = QString::fromLocal8Bit(path.substr(it + dir.size()));
            break;
        }

        log_text = QString("%1 [%2][%3][L%4] %5\n")
                       .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
                       .arg(LEVEL_TABLE.value(type, "UNK"))
                       .arg(file)
                       .arg(context.line)
                       .arg(msg);
    }
#endif

    if (const auto global_loggers = UI::LogPanel::global_logger_panels(); global_loggers.isEmpty()) {
        //! ATTENTION: do not write to stdout or stderr, it will be redirected to the handler again
    } else {
        for (auto logger : global_loggers) {
            QMetaObject::invokeMethod(logger, "log", Qt::AutoConnection, Q_ARG(QString, log_text));
        }
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
    qInfo().noquote() << message;
}

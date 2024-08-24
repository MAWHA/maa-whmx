#include "Process.h"

#include <QtCore/QProcess>

ExecResult exec_and_read(const QString& program, QStringList args, int timeout_ms) {
    ExecResult resp;

    QProcess proc;
    proc.setProcessChannelMode(QProcess::SeparateChannels);
    proc.start(program, args);
    if (const bool started = proc.waitForStarted(); !started) {
        resp.state = ExecState::Fail;
        return resp;
    }
    if (const bool finished = proc.waitForFinished(timeout_ms); !finished) {
        resp.state = ExecState::Timeout;
        return resp;
    }

    resp.output    = proc.readAllStandardOutput();
    resp.exit_code = proc.exitCode();
    resp.state     = resp.exit_code == 0 ? ExecState::Success : ExecState::Error;

    return resp;
}

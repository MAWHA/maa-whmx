#pragma once

#include <QtCore/QString>
#include <QtCore/QList>

enum class ExecState {
    Fail,
    Timeout,
    Error,
    Success,
};

struct ExecResult {
    ExecState state;
    QString   output;    //<! standard output only
    int       exit_code; //<! valid when state is Success or Error
};

ExecResult exec_and_read(const QString& program, QStringList args = QStringList(), int timeout_ms = 5000);

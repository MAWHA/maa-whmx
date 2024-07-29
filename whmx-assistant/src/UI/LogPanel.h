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

#include <QtWidgets/QPlainTextEdit>
#include <QtCore/QList>
#include <mutex>

namespace UI {

class LogPanel : public QWidget {
    Q_OBJECT

public:
    static QList<LogPanel *> global_logger_panels();

    void attach_to_global_logger();
    void detach_from_global_logger() const;

    void    clear();
    QString take();
    void    flush();
    void    set_max_len(int max_len);

public slots:
    void log(QString message);

signals:
    void on_flush(QString loggings);

public:
    LogPanel(QWidget *parent = nullptr);
    ~LogPanel() override;

protected:
    void setup();

private:
    QPlainTextEdit *log_text_container_ = nullptr;
    std::mutex      log_mutex_;
    int             max_log_len_;
};

} // namespace UI

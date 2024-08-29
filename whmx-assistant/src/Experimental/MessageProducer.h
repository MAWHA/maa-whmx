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

#include <QtCore/QObject>
#include <optional>

namespace Experimental {

class MessageProducer : public QObject {
    Q_OBJECT

signals:
    void on_broadcast(const QString &message, std::optional<QString> source);

public slots:

    void post(const QString &message) {
        emit on_broadcast(message, std::nullopt);
    }

    void post(const QString &message, const QString &source) {
        emit on_broadcast(message, std::make_optional(source));
    }

public:
    template <typename Slot>
    void subscribe(QObject *subscriber, Slot &&slot) {
        if (!subscriber) { return; }
        connect(this, &MessageProducer::on_broadcast, subscriber, std::forward<Slot>(slot));
    }

    void unsubscribe(QObject *subscriber) {
        if (!subscriber) { return; }
        disconnect(subscriber);
    }

public:
    MessageProducer(QObject *parent = nullptr)
        : QObject(parent) {}
};

} // namespace Experimental

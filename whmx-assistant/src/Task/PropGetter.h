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

#include "../MacroHelper.h"

#include <QtCore/QMap>
#include <QtCore/QVariant>

namespace Task {

#define DECLARE_PROP_GETTER_IMPL(prop)                                       \
    [](PropGetter* self) {                                                   \
        return QVariant::fromValue(static_cast<decltype(this)>(self)->prop); \
    }
#define DECLARE_PROP(prop) add(MH_STRINGIFY(prop), DECLARE_PROP_GETTER_IMPL(prop));
#define DECLARE_PROPS(type, ...)                                                 \
    void init_props() override{MH_EXPAND(MH_FOREACH(DECLARE_PROP, __VA_ARGS__))} \
                                                                                 \
    type() {                                                                     \
        init_props();                                                            \
    }

class Prop {
public:
    Prop() = default;

    Prop(QVariant value)
        : value_(value) {}

    template <typename T>
    Prop& operator=(const T& value) {
        value_ = value;
        return *this;
    }

    operator bool() const {
        return !is_null();
    }

    QVariant get() const {
        return value_;
    }

    bool is_null() const {
        return value_.isNull();
    }

    QString type() const {
        return value_.typeName();
    }

    int type_id() const {
        return value_.typeId();
    }

    template <typename T>
    const T& as() const {
        return *reinterpret_cast<const T*>(value_.constData());
    }

    int to_int() const {
        return value_.toInt();
    }

    double to_double() const {
        return value_.toDouble();
    }

    bool to_bool() const {
        return value_.toBool();
    }

    QString to_string() const {
        return value_.toString();
    }

private:
    QVariant value_;
};

class PropGetter {
protected:
    using GetterMethod = std::function<QVariant(PropGetter* self)>;

public:
    virtual void init_props() = 0;

    QStringList keys() const {
        return getters_.keys();
    }

    bool contains(const QString& key) const {
        return getters_.contains(key);
    }

    Prop value(const QString& key) {
        const auto it = getters_.find(key);
        if (it == getters_.end()) { return Prop(); }
        return Prop(std::invoke(it.value(), this));
    }

protected:
    void add(const QString& key, GetterMethod getter) {
        getters_[key] = getter;
    }

private:
    QMap<QString, GetterMethod> getters_;
};

} // namespace Task

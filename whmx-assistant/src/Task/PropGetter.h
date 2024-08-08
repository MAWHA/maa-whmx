#pragma once

#include "../MacroHelper.h"

#include <QtCore/QMap>
#include <QtCore/QVariant>

namespace Task {

#define DECLARE_PROP(prop) add(MH_STRINGIFY(prop), prop);
#define DECLARE_PROPS(type, ...)                                      \
    void init_props() override{MH_FOREACH(DECLARE_PROP, __VA_ARGS__)} \
                                                                      \
    type() {                                                          \
        init_props();                                                 \
    }

class PropGetter {
protected:
    class Prop {
    public:
        Prop(QVariant value)
            : value_(value) {}

        operator QVariant() const {
            return value_;
        }

        operator bool() const {
            return !is_null();
        }

        bool is_null() const {
            return value_.isNull();
        }

        QString type() const {
            return value_.typeName();
        }

        template <typename T>
        const T& as() const {
            return *reinterpret_cast<const T*>(value_.constData());
        }

    private:
        QVariant value_;
    };

public:
    virtual void init_props() = 0;

    QStringList keys() const {
        return values_.keys();
    }

    bool contains(const QString& key) const {
        return values_.contains(key);
    }

    Prop value(const QString& key) {
        return Prop(values_.value(key));
    }

protected:
    template <typename T>
    void add(const QString& key, const T& value) {
        add(key, QVariant::fromValue(value));
    }

    void add(const QString& key, const QVariant& value) {
        values_[key] = value;
    }

private:
    QMap<QString, QVariant> values_;
};

} // namespace Task

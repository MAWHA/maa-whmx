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

#include "Property.h"

#include <meojson/json.hpp>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <memory>
#include <any>
#include <optional>

class PropertyContext : public std::enable_shared_from_this<PropertyContext> {
public:
    static std::shared_ptr<PropertyContext> create() {
        return std::shared_ptr<PropertyContext>(new PropertyContext);
    }

    static bool is_type_valid(PropertyType* type);

    template <PropertyMetaType Meta>
    Property create_primary_meta_property(const property_meta_type_t<Meta>& value) {
        static_assert(
            Meta == PropertyMetaType::Bool || Meta == PropertyMetaType::Int || Meta == PropertyMetaType::Real
                || Meta == PropertyMetaType::Str,
            "expect primary meta type: bool, int, real, str");

        if constexpr (false) {
        } else if constexpr (Meta == PropertyMetaType::Bool) {
            return Property(this, "bool", value);
        } else if constexpr (Meta == PropertyMetaType::Int) {
            return Property(this, "int", value);
        } else if constexpr (Meta == PropertyMetaType::Real) {
            return Property(this, "real", value);
        } else if constexpr (Meta == PropertyMetaType::Str) {
            return Property(this, "str", value);
        }
    }

    bool    has_non_default_value(const QString& target, const QString& name) const;
    QString property_type(const QString& target, const QString& name) const;

    std::optional<std::reference_wrapper<Property>> property(const QString& target, const QString& name) const;

    bool contains_type(const QString& type) const {
        return types_.contains(type);
    }

    bool contains_property(const QString& target, const QString& name) const {
        return target_property_table_.value(target).contains(name);
    }

    bool has_properties(const QString& target) {
        return target_property_table_.contains(target);
    }

    PropertyType* type(const QString& type) const {
        return types_.value(type, nullptr);
    }

    QStringList properties(const QString& target) const {
        return target_property_table_.value(target, QStringList());
    }

    bool                   add_public_type(PropertyType* type);
    std::optional<QString> add_private_type(PropertyType* type);

    bool add_property(const QString& target, const QString& name, const QString& type, const std::any& default_value);
    bool add_property(const QString& target, const QString& name, const QString& type, const json::value& default_value);
    bool add_property(const QString& target, const QString& name, PropertyType* type, const std::any& default_value);
    bool add_property(const QString& target, const QString& name, PropertyType* type, const json::value& default_value);

    std::optional<Property>    deserialize(const QString& type, const json::value& value) const;
    std::optional<json::value> serialize(const Property& value) const;
    std::optional<json::value> serialize(const QString& type, const std::any& value) const;

    bool verify_value(const QString& type, const std::any& value);
    bool verify_value(const QString& type, const json::value& value);

protected:
    PropertyContext();

private:
    struct PropertyValue {
        QString  type;
        std::any data;
    };

    QMap<QString, PropertyValue> property_default_value_table_;
    QMap<QString, Property>      property_value_table_;
    QMap<QString, QStringList>   target_property_table_;
    QMap<QString, PropertyType*> types_;
    QMap<QString, int>           type_refs_;
};

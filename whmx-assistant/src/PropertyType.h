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

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <limits>
#include <memory>
#include <optional>

struct PropertyBoolType;
struct PropertyIntType;
struct PropertyRealType;
struct PropertyStrType;
struct PropertyEnumType;
struct PropertyOptType;
struct PropertyListType;
struct PropertyObjectType;

class Property;

enum class PropertyMetaType {
    Bool,   //<! boolean
    Int,    //<! 32-bit signed integer
    Real,   //<! double precision floating point number
    Str,    //<! string
    Enum,   //<! enum[string, int]
    Opt,    //<! optional[property]
    List,   //<! list[property]
    Object, //<! object[string, property]
};

namespace details {

template <PropertyMetaType Type>
struct property_meta_type;

template <>
struct property_meta_type<PropertyMetaType::Bool> {
    using value_type = bool;
};

template <>
struct property_meta_type<PropertyMetaType::Int> {
    using value_type = int32_t;
};

template <>
struct property_meta_type<PropertyMetaType::Real> {
    using value_type = double;
};

template <>
struct property_meta_type<PropertyMetaType::Str> {
    using value_type = QString;
};

template <>
struct property_meta_type<PropertyMetaType::Enum> {
    using value_type = QString;
};

template <>
struct property_meta_type<PropertyMetaType::Opt> {
    using value_type = std::optional<Property>;
};

template <>
struct property_meta_type<PropertyMetaType::List> {
    using value_type = QList<Property>;
};

template <>
struct property_meta_type<PropertyMetaType::Object> {
    using value_type = QMap<QString, Property>;
};

} // namespace details

template <PropertyMetaType Type>
using property_meta_type_t = typename details::property_meta_type<Type>::value_type;

class PropertyType : public std::enable_shared_from_this<PropertyType> {
protected:
    PropertyType(PropertyMetaType meta, const QString& name = QString())
        : meta(meta)
        , name(name) {}

public:
    static PropertyBoolType*   make_bool(const QString& name = "bool");
    static PropertyIntType*    make_int(const QString& name = "int");
    static PropertyRealType*   make_real(const QString& name = "real");
    static PropertyStrType*    make_str(const QString& name = "str");
    static PropertyEnumType*   make_enum(const QString& name);
    static PropertyOptType*    make_opt(const QString& name);
    static PropertyListType*   make_list(const QString& name);
    static PropertyObjectType* make_object(const QString& name);

    PropertyBoolType*   as_bool();
    PropertyIntType*    as_int();
    PropertyRealType*   as_real();
    PropertyStrType*    as_str();
    PropertyEnumType*   as_enum();
    PropertyOptType*    as_opt();
    PropertyListType*   as_list();
    PropertyObjectType* as_object();

    const PropertyBoolType*   as_bool() const;
    const PropertyIntType*    as_int() const;
    const PropertyRealType*   as_real() const;
    const PropertyStrType*    as_str() const;
    const PropertyEnumType*   as_enum() const;
    const PropertyOptType*    as_opt() const;
    const PropertyListType*   as_list() const;
    const PropertyObjectType* as_object() const;

    bool is_bool() const {
        return meta == PropertyMetaType::Bool;
    }

    bool is_int() const {
        return meta == PropertyMetaType::Int;
    }

    bool is_real() const {
        return meta == PropertyMetaType::Real;
    }

    bool is_str() const {
        return meta == PropertyMetaType::Str;
    }

    bool is_enum() const {
        return meta == PropertyMetaType::Enum;
    }

    bool is_opt() const {
        return meta == PropertyMetaType::Opt;
    }

    bool is_list() const {
        return meta == PropertyMetaType::List;
    }

    bool is_object() const {
        return meta == PropertyMetaType::Object;
    }

    PropertyBoolType* try_into_bool() {
        return is_bool() ? as_bool() : nullptr;
    }

    PropertyIntType* try_into_int() {
        return is_int() ? as_int() : nullptr;
    }

    PropertyRealType* try_into_real() {
        return is_real() ? as_real() : nullptr;
    }

    PropertyStrType* try_into_str() {
        return is_str() ? as_str() : nullptr;
    }

    PropertyEnumType* try_into_enum() {
        return is_enum() ? as_enum() : nullptr;
    }

    PropertyOptType* try_into_opt() {
        return is_opt() ? as_opt() : nullptr;
    }

    PropertyListType* try_into_list() {
        return is_list() ? as_list() : nullptr;
    }

    PropertyObjectType* try_into_object() {
        return is_object() ? as_object() : nullptr;
    }

    const PropertyBoolType* try_into_bool() const {
        return is_bool() ? as_bool() : nullptr;
    }

    const PropertyIntType* try_into_int() const {
        return is_int() ? as_int() : nullptr;
    }

    const PropertyRealType* try_into_real() const {
        return is_real() ? as_real() : nullptr;
    }

    const PropertyStrType* try_into_str() const {
        return is_str() ? as_str() : nullptr;
    }

    const PropertyEnumType* try_into_enum() const {
        return is_enum() ? as_enum() : nullptr;
    }

    const PropertyOptType* try_into_opt() const {
        return is_opt() ? as_opt() : nullptr;
    }

    const PropertyListType* try_into_list() const {
        return is_list() ? as_list() : nullptr;
    }

    const PropertyObjectType* try_into_object() const {
        return is_object() ? as_object() : nullptr;
    }

public:
    const PropertyMetaType meta;
    QString                name;
};

struct PropertyEnumItem {
    QString                name;
    std::optional<QString> opt_display_name;
    std::optional<QString> opt_desc;
};

struct PropertyBoolType final : public PropertyType {
    PropertyBoolType(const QString& name = QString())
        : PropertyType(PropertyMetaType::Bool, name) {}
};

struct PropertyIntType final : public PropertyType {
    PropertyIntType(const QString& name = QString())
        : PropertyType(PropertyMetaType::Int, name) {}

    int32_t minval = std::numeric_limits<int32_t>::min();
    int32_t maxval = std::numeric_limits<int32_t>::max();
    int32_t step   = 1;
};

struct PropertyRealType final : public PropertyType {
    PropertyRealType(const QString& name = QString())
        : PropertyType(PropertyMetaType::Real, name) {}

    double minval = std::numeric_limits<double>::lowest();
    double maxval = std::numeric_limits<double>::max();
    double step   = 1.0;
};

struct PropertyStrType final : public PropertyType {
    PropertyStrType(const QString& name = QString())
        : PropertyType(PropertyMetaType::Str, name) {}

    std::optional<QStringList> opt_values = std::nullopt;
};

struct PropertyEnumType final : public PropertyType {
    PropertyEnumType(const QString& name = QString())
        : PropertyType(PropertyMetaType::Enum, name) {}

    QList<PropertyEnumItem> items = QList<PropertyEnumItem>();
};

struct PropertyOptType final : public PropertyType {
    PropertyOptType(const QString& name = QString())
        : PropertyType(PropertyMetaType::Opt, name) {}

    QString value_type = "str";
};

struct PropertyListType final : public PropertyType {
    PropertyListType(const QString& name = QString())
        : PropertyType(PropertyMetaType::List, name) {}

    QString            item_type      = "str";
    std::optional<int> opt_fixed_size = std::nullopt;
};

struct PropertyObjectType final : public PropertyType {
    PropertyObjectType(const QString& name = QString())
        : PropertyType(PropertyMetaType::Object, name) {}

    std::optional<QString>                opt_generic_value_type    = std::make_optional("str");
    std::optional<QMap<QString, QString>> opt_key_value_type_schema = std::nullopt;
};

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

#include "PropertyType.h"

#include <any>

class PropertyContext;

class Property {
private:
    friend class PropertyContext;

    Property()
        : context_(nullptr) {}

    Property(const PropertyContext* context, const QString& type)
        : context_(context)
        , type_(type) {}

    Property(const PropertyContext* context, const QString& type, std::any&& value)
        : context_(context)
        , type_(type)
        , value_(value) {}

public:
    static Property nullprop() {
        return Property();
    }

    PropertyType*    type() const;
    PropertyMetaType meta_type() const;

    bool valid() const {
        return context_ != nullptr && type() != nullptr;
    }

    bool is_null() const {
        return context_ == nullptr;
    }

    QString type_name() const {
        return type_;
    }

    bool has_value() const {
        return value_.has_value();
    }

    const std::any& value() const& {
        return value_;
    }

    std::any& value() & {
        return value_;
    }

    bool is_bool() const {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::Bool>>;
        return value_.has_value() && value_.type() == typeid(value_type);
    }

    bool is_int() const {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::Int>>;
        return value_.has_value() && value_.type() == typeid(value_type);
    }

    bool is_real() const {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::Real>>;
        return value_.has_value() && value_.type() == typeid(value_type);
    }

    bool is_str() const {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::Str>>;
        return value_.has_value() && value_.type() == typeid(value_type);
    }

    bool is_enum() const {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::Enum>>;
        return value_.has_value() && value_.type() == typeid(value_type);
    }

    bool is_opt() const {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::Opt>>;
        return value_.has_value() && value_.type() == typeid(value_type);
    }

    bool is_list() const {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::List>>;
        return value_.has_value() && value_.type() == typeid(value_type);
    }

    bool is_object() const {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::Object>>;
        return value_.has_value() && value_.type() == typeid(value_type);
    }

    decltype(auto) as_bool() & {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::Bool>>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_int() & {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::Int>>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_real() & {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::Real>>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_str() & {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::Str>>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_enum() & {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::Enum>>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_opt() & {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::Opt>>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_list() & {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::List>>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_object() & {
        using value_type = std::add_lvalue_reference_t<property_meta_type_t<PropertyMetaType::Object>>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_bool() const& {
        using value_type = property_meta_type_t<PropertyMetaType::Bool>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_int() const& {
        using value_type = property_meta_type_t<PropertyMetaType::Int>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_real() const& {
        using value_type = property_meta_type_t<PropertyMetaType::Real>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_str() const& {
        using value_type = property_meta_type_t<PropertyMetaType::Str>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_enum() const& {
        using value_type = property_meta_type_t<PropertyMetaType::Enum>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_opt() const& {
        using value_type = property_meta_type_t<PropertyMetaType::Opt>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_list() const& {
        using value_type = property_meta_type_t<PropertyMetaType::List>;
        return std::any_cast<value_type>(value_);
    }

    decltype(auto) as_object() const& {
        using value_type = property_meta_type_t<PropertyMetaType::Object>;
        return std::any_cast<value_type>(value_);
    }

protected:
    void set_value_unsafe(std::any&& value) {
        value_ = std::move(value);
    }

private:
    const PropertyContext* context_;
    QString                type_;
    std::any               value_;
};

inline PropertyBoolType* PropertyType::make_bool(const QString& name) {
    return new PropertyBoolType(name);
}

inline PropertyIntType* PropertyType::make_int(const QString& name) {
    return new PropertyIntType(name);
}

inline PropertyRealType* PropertyType::make_real(const QString& name) {
    return new PropertyRealType(name);
}

inline PropertyStrType* PropertyType::make_str(const QString& name) {
    return new PropertyStrType(name);
}

inline PropertyEnumType* PropertyType::make_enum(const QString& name) {
    return new PropertyEnumType(name);
}

inline PropertyOptType* PropertyType::make_opt(const QString& name) {
    return new PropertyOptType(name);
}

inline PropertyListType* PropertyType::make_list(const QString& name) {
    return new PropertyListType(name);
}

inline PropertyObjectType* PropertyType::make_object(const QString& name) {
    return new PropertyObjectType(name);
}

inline PropertyBoolType* PropertyType::as_bool() {
    return static_cast<PropertyBoolType*>(this);
}

inline PropertyIntType* PropertyType::as_int() {
    return static_cast<PropertyIntType*>(this);
}

inline PropertyRealType* PropertyType::as_real() {
    return static_cast<PropertyRealType*>(this);
}

inline PropertyStrType* PropertyType::as_str() {
    return static_cast<PropertyStrType*>(this);
}

inline PropertyEnumType* PropertyType::as_enum() {
    return static_cast<PropertyEnumType*>(this);
}

inline PropertyOptType* PropertyType::as_opt() {
    return static_cast<PropertyOptType*>(this);
}

inline PropertyListType* PropertyType::as_list() {
    return static_cast<PropertyListType*>(this);
}

inline PropertyObjectType* PropertyType::as_object() {
    return static_cast<PropertyObjectType*>(this);
}

inline const PropertyBoolType* PropertyType::as_bool() const {
    return static_cast<const PropertyBoolType*>(this);
}

inline const PropertyIntType* PropertyType::as_int() const {
    return static_cast<const PropertyIntType*>(this);
}

inline const PropertyRealType* PropertyType::as_real() const {
    return static_cast<const PropertyRealType*>(this);
}

inline const PropertyStrType* PropertyType::as_str() const {
    return static_cast<const PropertyStrType*>(this);
}

inline const PropertyEnumType* PropertyType::as_enum() const {
    return static_cast<const PropertyEnumType*>(this);
}

inline const PropertyOptType* PropertyType::as_opt() const {
    return static_cast<const PropertyOptType*>(this);
}

inline const PropertyListType* PropertyType::as_list() const {
    return static_cast<const PropertyListType*>(this);
}

inline const PropertyObjectType* PropertyType::as_object() const {
    return static_cast<const PropertyObjectType*>(this);
}

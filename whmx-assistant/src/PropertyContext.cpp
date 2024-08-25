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

#include "PropertyContext.h"
#include "Algorithm.h"

#include <QtCore/QUuid>

PropertyContext::PropertyContext() {
    add_public_type(PropertyType::make_bool());
    add_public_type(PropertyType::make_int());
    add_public_type(PropertyType::make_real());
    add_public_type(PropertyType::make_str());
}

bool PropertyContext::is_type_valid(PropertyType* type) {
    //! TODO: ensure non-recursive
    if (false) {
    } else if (auto enum_type = type->try_into_enum()) {
        if (enum_type->items.empty()) { return false; }
    } else if (auto object_type = type->try_into_object()) {
        const bool has_fixed_type   = object_type->opt_generic_value_type.has_value();
        const bool has_various_type = object_type->opt_key_value_type_schema.has_value();
        if (has_fixed_type == has_various_type) { return false; }
    }
    return true;
}

std::optional<std::reference_wrapper<Property>> PropertyContext::property(const QString& target, const QString& name) const {
    if (!contains_property(target, name)) { return std::nullopt; }
    const auto property_full_name = target + "." + name;
    auto       self               = const_cast<PropertyContext*>(this);
    if (!property_value_table_.contains(property_full_name)) {
        Q_ASSERT(property_default_value_table_.contains(property_full_name));
        const auto& prop_value = property_default_value_table_.find(property_full_name).value();
        Property    prop(this, prop_value.type);
        prop.value() = prop_value.data;
        self->property_value_table_.insert(property_full_name, prop);
    }
    return std::make_optional(std::ref(self->property_value_table_.find(property_full_name).value()));
}

bool PropertyContext::add_public_type(PropertyType* type) {
    if (!type) { return false; }
    if (contains_type(type->name)) { return false; }
    if (!is_type_valid(type)) { return false; }
    types_.insert(type->name, type);
    type_refs_.insert(type->name, 0);
    return true;
}

std::optional<QString> PropertyContext::add_private_type(PropertyType* type) {
    if (!type) { return std::nullopt; }
    if (!is_type_valid(type)) { return std::nullopt; }
    type->name = QUuid::createUuid().toString(QUuid::Id128);
    if (contains_type(type->name)) { return std::nullopt; }
    types_.insert(type->name, type);
    type_refs_.insert(type->name, 0);
    return std::make_optional(type->name);
}

bool PropertyContext::add_property(
    const QString& target, const QString& name, const QString& type, const std::any& default_value) {
    if (!contains_type(type)) { return false; }
    if (contains_property(target, name)) { return false; }
    if (!verify_value(type, default_value)) { return false; }
    if (!target_property_table_.contains(target)) { target_property_table_.insert(target, QStringList()); }
    const auto property_full_name = target + "." + name;
    target_property_table_.find(target)->append(name);
    PropertyValue prop_value{.type = type, .data = std::move(default_value)};
    property_default_value_table_.insert(property_full_name, std::move(prop_value));
    Q_ASSERT(type_refs_.contains(type));
    ++type_refs_[type];
    return true;
}

bool PropertyContext::add_property(
    const QString& target, const QString& name, const QString& type, const json::value& default_value) {
    if (!contains_type(type)) { return false; }
    if (contains_property(target, name)) { return false; }
    if (!verify_value(type, default_value)) { return false; }
    if (!target_property_table_.contains(target)) { target_property_table_.insert(target, QStringList()); }
    const auto full_property_name = target + "." + name;
    target_property_table_.find(target)->append(name);
    PropertyValue prop_value{.type = type, .data = std::move(deserialize(type, default_value)->value())};
    property_default_value_table_.insert(full_property_name, std::move(prop_value));
    Q_ASSERT(type_refs_.contains(type));
    ++type_refs_[type];
    return true;
}

bool PropertyContext::add_property(
    const QString& target, const QString& name, PropertyType* type, const std::any& default_value) {
    const auto opt_type_name = add_private_type(type);
    if (!opt_type_name.has_value()) { return false; }
    return add_property(target, name, opt_type_name.value(), default_value);
}

bool PropertyContext::add_property(
    const QString& target, const QString& name, PropertyType* type, const json::value& default_value) {
    const auto opt_type_name = add_private_type(type);
    if (!opt_type_name.has_value()) { return false; }
    return add_property(target, name, opt_type_name.value(), default_value);
}

std::optional<Property> PropertyContext::deserialize(const QString& type, const json::value& value) const {
    if (!contains_type(type)) { return std::nullopt; }
    const auto prop_type = types_.find(type).value();
    Property   prop(this, type);
    switch (prop_type->meta) {
        case PropertyMetaType::Bool: {
            using value_type = property_meta_type_t<PropertyMetaType::Bool>;
            if (!value.is_boolean()) { return std::nullopt; }
            prop.set_value_unsafe(std::make_any<value_type>(value.as_boolean()));
            return std::make_optional(prop);
        } break;
        case PropertyMetaType::Int: {
            using value_type = property_meta_type_t<PropertyMetaType::Int>;
            if (!value.is_number()) { return std::nullopt; }
            prop.set_value_unsafe(std::make_any<value_type>(value.as_integer()));
            return std::make_optional(prop);
        } break;
        case PropertyMetaType::Real: {
            using value_type = property_meta_type_t<PropertyMetaType::Real>;
            if (!value.is_number()) { return std::nullopt; }
            prop.set_value_unsafe(std::make_any<value_type>(value.as_double()));
            return std::make_optional(prop);
        } break;
        case PropertyMetaType::Str: {
            using value_type = property_meta_type_t<PropertyMetaType::Str>;
            if (!value.is_string()) { return std::nullopt; }
            prop.set_value_unsafe(std::make_any<value_type>(QString::fromUtf8(value.as_string())));
            return std::make_optional(prop);
        } break;
        case PropertyMetaType::Enum: {
            using value_type = property_meta_type_t<PropertyMetaType::Enum>;
            if (!value.is_string()) { return std::nullopt; }
            const auto  enum_value = QString::fromUtf8(value.as_string());
            const auto& enum_items = prop_type->as_enum()->items;
            if (find_element_opt(enum_items, &PropertyEnumItem::name, enum_value).has_value()) {
                prop.set_value_unsafe(std::make_any<value_type>(enum_value));
            } else {
                return std::nullopt;
            }
            return std::make_optional(prop);
        } break;
        case PropertyMetaType::Opt: {
            using value_type = property_meta_type_t<PropertyMetaType::Opt>;
            if (value.is_null()) {
                prop.set_value_unsafe(std::make_any<value_type>(std::nullopt));
            } else if (const auto opt_inner_prop = deserialize(prop_type->as_opt()->value_type, value)) {
                const auto& inner_prop = opt_inner_prop.value();
                prop.set_value_unsafe(std::make_any<value_type>(std::make_optional(inner_prop)));
            } else {
                return std::nullopt;
            }
            return std::make_optional(prop);
        } break;
        case PropertyMetaType::List: {
            using value_type = property_meta_type_t<PropertyMetaType::List>;
            if (!value.is_array()) { return std::nullopt; }
            const auto& unwrapped      = value.as_array();
            const auto& opt_fixed_size = prop_type->as_list()->opt_fixed_size;
            const auto& item_type      = prop_type->as_list()->item_type;
            if (opt_fixed_size.has_value() && unwrapped.size() != opt_fixed_size.value()) { return std::nullopt; }
            value_type list;
            for (const auto& item : unwrapped) {
                if (const auto opt_inner_prop = deserialize(item_type, item)) {
                    list.append(opt_inner_prop.value());
                } else {
                    return std::nullopt;
                }
            }
            prop.set_value_unsafe(std::make_any<value_type>(list));
            return std::make_optional(prop);
        } break;
        case PropertyMetaType::Object: {
            using value_type = property_meta_type_t<PropertyMetaType::Object>;
            if (!value.is_object()) { return std::nullopt; }
            const auto& unwrapped = value.as_object();
            value_type  object;
            if (const auto obj_type = prop_type->as_object(); obj_type->opt_generic_value_type.has_value()) {
                const auto& type = obj_type->opt_generic_value_type.value();
                for (const auto& [key, value] : unwrapped) {
                    if (const auto opt_inner_prop = deserialize(type, value)) {
                        object.insert(QString::fromUtf8(key), opt_inner_prop.value());
                    } else {
                        return std::nullopt;
                    }
                }
            } else if (const auto& schema = obj_type->opt_key_value_type_schema.value(); schema.size() == unwrapped.size()) {
                for (const auto& [key, type] : schema.asKeyValueRange()) {
                    const auto stdkey = key.toStdString();
                    if (!unwrapped.contains(stdkey)) { return std::nullopt; }
                    if (const auto opt_inner_prop = deserialize(type, unwrapped.at(stdkey))) {
                        object.insert(key, opt_inner_prop.value());
                    } else {
                        return std::nullopt;
                    }
                }
            } else {
                return std::nullopt;
            }
            prop.set_value_unsafe(std::make_any<value_type>(object));
            return std::make_optional(prop);
        } break;
    }
    Q_UNREACHABLE();
}

std::optional<json::value> PropertyContext::serialize(const Property& value) const {
    return serialize(value.type_name(), value.value());
}

std::optional<json::value> PropertyContext::serialize(const QString& type, const std::any& value) const {
    if (!contains_type(type)) { return std::nullopt; }
    const auto prop_type = types_.find(type).value();
    switch (prop_type->meta) {
        case PropertyMetaType::Bool: {
            using value_type = property_meta_type_t<PropertyMetaType::Bool>;
            if (value.type() != typeid(value_type)) { return std::nullopt; }
            return std::make_optional<json::value>(std::any_cast<value_type>(value));
        } break;
        case PropertyMetaType::Int: {
            using value_type = property_meta_type_t<PropertyMetaType::Int>;
            if (value.type() != typeid(value_type)) { return std::nullopt; }
            return std::make_optional<json::value>(std::any_cast<value_type>(value));
        } break;
        case PropertyMetaType::Real: {
            using value_type = property_meta_type_t<PropertyMetaType::Real>;
            if (value.type() != typeid(value_type)) { return std::nullopt; }
            return std::make_optional<json::value>(std::any_cast<value_type>(value));
        } break;
        case PropertyMetaType::Str: {
            using value_type = property_meta_type_t<PropertyMetaType::Str>;
            if (value.type() != typeid(value_type)) { return std::nullopt; }
            return std::make_optional<json::value>(std::any_cast<value_type>(value).toStdString());
        } break;
        case PropertyMetaType::Enum: {
            using value_type = property_meta_type_t<PropertyMetaType::Enum>;
            if (value.type() != typeid(value_type)) { return std::nullopt; }
            const auto  enum_value = std::any_cast<value_type>(value);
            const auto& enum_items = prop_type->as_enum()->items;
            if (find_element_opt(enum_items, &PropertyEnumItem::name, enum_value).has_value()) {
                return std::make_optional<json::value>(enum_value.toStdString());
            } else {
                return std::nullopt;
            }
        } break;
        case PropertyMetaType::Opt: {
            using value_type = property_meta_type_t<PropertyMetaType::Opt>;
            if (!value.has_value()) { return std::make_optional<json::value>(); }
            return serialize(prop_type->as_opt()->value_type, value);
        } break;
        case PropertyMetaType::List: {
            using value_type = property_meta_type_t<PropertyMetaType::List>;
            if (value.type() != typeid(value_type)) { return std::nullopt; }
            const auto& unwrapped      = std::any_cast<value_type>(value);
            const auto& opt_fixed_size = prop_type->as_list()->opt_fixed_size;
            const auto& item_type      = prop_type->as_list()->item_type;
            if (opt_fixed_size.has_value() && unwrapped.size() != opt_fixed_size.value()) { return std::nullopt; }
            json::array list;
            for (const auto& item : unwrapped) {
                if (const auto opt_inner_prop = serialize(item_type, item.value())) {
                    list.push_back(opt_inner_prop.value());
                } else {
                    return std::nullopt;
                }
            }
            return std::make_optional<json::value>(list);
        } break;
        case PropertyMetaType::Object: {
            using value_type = property_meta_type_t<PropertyMetaType::Object>;
            if (value.type() != typeid(value_type)) { return std::nullopt; }
            const auto&  unwrapped = std::any_cast<value_type>(value);
            json::object object;
            if (const auto obj_type = prop_type->as_object(); obj_type->opt_generic_value_type.has_value()) {
                const auto& type = obj_type->opt_generic_value_type.value();
                for (const auto& [key, value] : unwrapped.asKeyValueRange()) {
                    if (const auto opt_inner_prop = serialize(type, value)) {
                        object[key.toStdString()] = opt_inner_prop.value();
                    } else {
                        return std::nullopt;
                    }
                }
            } else if (const auto& schema = obj_type->opt_key_value_type_schema.value(); schema.size() == unwrapped.size()) {
                for (const auto& [key, type] : schema.asKeyValueRange()) {
                    if (!unwrapped.contains(key)) { return std::nullopt; }
                    if (const auto opt_inner_prop = serialize(type, unwrapped.find(key)->value())) {
                        object[key.toStdString()] = opt_inner_prop.value();
                    } else {
                        return std::nullopt;
                    }
                }
            } else {
                return std::nullopt;
            }
            return std::make_optional<json::value>(object);
        } break;
    }
    Q_UNREACHABLE();
}

bool PropertyContext::verify_value(const QString& type, const std::any& value) {
    Q_ASSERT(contains_type(type));
    const auto prop_type = types_.value(type);
    Q_ASSERT(prop_type != nullptr);
    Q_ASSERT(is_type_valid(prop_type));
    try {
        switch (prop_type->meta) {
            case PropertyMetaType::Bool: {
                using value_type = property_meta_type_t<PropertyMetaType::Bool>;
                return value.type() == typeid(value_type);
            } break;
            case PropertyMetaType::Int: {
                using value_type = property_meta_type_t<PropertyMetaType::Int>;
                return value.type() == typeid(value_type);
            } break;
            case PropertyMetaType::Real: {
                using value_type = property_meta_type_t<PropertyMetaType::Real>;
                return value.type() == typeid(value_type);
            } break;
            case PropertyMetaType::Str: {
                using value_type = property_meta_type_t<PropertyMetaType::Str>;
                return value.type() == typeid(value_type);
            } break;
            case PropertyMetaType::Enum: {
                const auto  enum_value = std::any_cast<property_meta_type_t<PropertyMetaType::Enum>>(value);
                const auto& enum_items = prop_type->as_enum()->items;
                return find_element_opt(enum_items, &PropertyEnumItem::name, enum_value).has_value();
            } break;
            case PropertyMetaType::Opt: {
                const auto unwrapped = std::any_cast<property_meta_type_t<PropertyMetaType::Opt>>(value);
                if (!unwrapped.has_value()) { return true; }
                return verify_value(prop_type->as_opt()->value_type, std::any(unwrapped.value()));
            } break;
            case PropertyMetaType::List: {
                const auto  unwrapped      = std::any_cast<property_meta_type_t<PropertyMetaType::List>>(value);
                const auto& opt_fixed_size = prop_type->as_list()->opt_fixed_size;
                const auto& item_type      = prop_type->as_list()->item_type;
                if (opt_fixed_size.has_value() && unwrapped.size() != opt_fixed_size.value()) { return false; }
                if (unwrapped.empty()) { return true; }
                return std::all_of(unwrapped.begin(), unwrapped.end(), [&, this](const auto& item) {
                    return verify_value(item_type, std::any(item));
                });
            } break;
            case PropertyMetaType::Object: {
                const auto unwrapped = std::any_cast<property_meta_type_t<PropertyMetaType::Object>>(value);
                if (const auto obj_type = prop_type->as_object(); obj_type->opt_generic_value_type.has_value()) {
                    const auto& type = obj_type->opt_generic_value_type.value();
                    const auto  keys = unwrapped.keys();
                    return std::all_of(keys.begin(), keys.end(), [&, this](const auto& key) {
                        return verify_value(type, unwrapped.find(key)->value_);
                    });
                } else {
                    const auto& schema      = obj_type->opt_key_value_type_schema.value();
                    const auto& schema_keys = schema.keys();
                    if (unwrapped.keys() != schema_keys) { return false; }
                    return std::all_of(schema_keys.begin(), schema_keys.end(), [&, this](const auto& key) {
                        return verify_value(schema.value(key), unwrapped.find(key)->value_);
                    });
                }
            } break;
        }
    } catch (const std::bad_any_cast& e) { return false; }
    Q_UNREACHABLE();
}

bool PropertyContext::verify_value(const QString& type, const json::value& value) {
    Q_ASSERT(contains_type(type));
    const auto prop_type = types_.value(type);
    Q_ASSERT(prop_type != nullptr);
    Q_ASSERT(is_type_valid(prop_type));
    switch (prop_type->meta) {
        case PropertyMetaType::Bool: {
            return value.is_boolean();
        } break;
        case PropertyMetaType::Int: {
            if (!value.is_number()) { return false; }
            const int    intval = value.as_integer();
            const double fltval = value.as_double();
            return static_cast<double>(intval) == fltval;
        } break;
        case PropertyMetaType::Real: {
            if (!value.is_number()) { return false; }
            const int    intval = value.as_integer();
            const double fltval = value.as_double();
            return static_cast<double>(intval) != fltval;
        } break;
        case PropertyMetaType::Str: {
            return value.is_string();
        } break;
        case PropertyMetaType::Enum: {
            if (!value.is_string()) { return false; }
            const auto  enum_value = QString::fromUtf8(value.as_string());
            const auto& enum_items = prop_type->as_enum()->items;
            return find_element_opt(enum_items, &PropertyEnumItem::name, enum_value).has_value();
        } break;
        case PropertyMetaType::Opt: {
            if (value.is_null()) { return true; }
            return verify_value(prop_type->as_opt()->value_type, value);
        } break;
        case PropertyMetaType::List: {
            if (!value.is_array()) { return false; }
            const auto& unwrapped      = value.as_array();
            const auto& opt_fixed_size = prop_type->as_list()->opt_fixed_size;
            const auto& item_type      = prop_type->as_list()->item_type;
            if (opt_fixed_size.has_value() && unwrapped.size() != opt_fixed_size.value()) { return false; }
            if (unwrapped.empty()) { return true; }
            return std::all_of(unwrapped.begin(), unwrapped.end(), [&, this](const auto& item) {
                return verify_value(item_type, item);
            });
        } break;
        case PropertyMetaType::Object: {
            if (!value.is_object()) { return false; }
            const auto& unwrapped = value.as_object();
            if (const auto obj_type = prop_type->as_object(); obj_type->opt_generic_value_type.has_value()) {
                const auto& type = obj_type->opt_generic_value_type.value();
                return std::all_of(unwrapped.begin(), unwrapped.end(), [&, this](const auto& pair) {
                    const auto& [_, value] = pair;
                    return verify_value(type, value);
                });
            } else {
                const auto& schema      = obj_type->opt_key_value_type_schema.value();
                const auto& schema_keys = schema.keys();
                QStringList keys;
                for (const auto& [key, _] : unwrapped) { keys.append(QString::fromUtf8(key)); }
                if (keys != schema_keys) { return false; }
                return std::all_of(schema_keys.begin(), schema_keys.end(), [&, this](const auto& key) {
                    return verify_value(schema.value(key), unwrapped.at(key.toStdString()));
                });
            }
        } break;
    }
    Q_UNREACHABLE();
}

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

#include "TaskInterface.h"
#include "PropertyType.h"

#include <meojson/json.hpp>
#include <QtCore/QFile>

std::shared_ptr<TaskInterface> TaskInterface::load(const char* utf8_path) {
    return load(QString::fromUtf8(utf8_path));
}

std::shared_ptr<TaskInterface> TaskInterface::load(const QString& path) {
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text); !file.isOpen()) { return nullptr; }
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    const auto opt_interface = json::parse(content.toStdString());
    if (!opt_interface.has_value()) { return nullptr; }
    const auto interface = opt_interface.value();

    return load(interface);
}

std::shared_ptr<TaskInterface> TaskInterface::load(const json::value& interface) {
    //! load defined tasks
    QMap<QString, TaskMetaInfo> tasks;

    for (const auto& [task, config] : interface.get("tasks", json::object())) {
        Q_ASSERT(config.contains("name") && config.at("name").is_string());
        Q_ASSERT(!config.contains("category") || config.at("category").is_string());
        Q_ASSERT(!config.contains("desc") || config.at("desc").is_string());
        const auto   task_name = QString::fromUtf8(task);
        TaskMetaInfo meta;
        meta.name = QString::fromUtf8(config.at("name").as_string());
        if (config.contains("category")) { meta.opt_category = QString::fromUtf8(config.at("category").as_string()); }
        if (config.contains("desc")) { meta.opt_desc = QString::fromUtf8(config.at("desc").as_string()); }
        tasks.insert(task_name, meta);
    }

    //! load property context
    auto context = PropertyContext::create();

    for (const auto& [name, define] : interface.get("typedef", json::object())) {
        Q_ASSERT(define.contains("type") && define.at("type").is_string());
        const auto    meta_type = QString::fromUtf8(define.at("type").as_string());
        PropertyType* type      = nullptr;
        if (false) {
        } else if (meta_type == "enum") {
            Q_ASSERT(define.contains("values") && define.at("values").is_array());
            Q_ASSERT(!define.at("values").as_array().empty());
            Q_ASSERT(define.at("values").as_array().all<json::object>());
            auto enum_type = PropertyType::make_enum(QString::fromUtf8(name));
            for (const auto item : define.at("values").as_array()) {
                PropertyEnumItem enum_item;
                Q_ASSERT(item.contains("name") && item.at("name").is_string());
                enum_item.name = QString::fromUtf8(item.at("name").as_string());
                if (item.contains("display_name")) {
                    Q_ASSERT(item.at("display_name").is_string());
                    const auto display_name    = QString::fromUtf8(item.at("display_name").as_string());
                    enum_item.opt_display_name = std::make_optional(display_name);
                }
                if (item.contains("desc")) {
                    Q_ASSERT(item.at("desc").is_string());
                    const auto desc    = QString::fromUtf8(item.at("desc").as_string());
                    enum_item.opt_desc = std::make_optional(desc);
                }
                enum_type->items.append(enum_item);
            }
            type = enum_type;
        } else {
            Q_UNREACHABLE();
        }
        if (!type) { continue; }
        const bool ok = context->add_public_type(type);
        Q_ASSERT(ok);
    }

    for (const auto& [task, config] : interface.get("tasks", json::object())) {
        if (!config.contains("property")) { continue; }
        Q_ASSERT(config.at("property").is_object());
        const auto target_name = QString::fromUtf8(task);
        for (const auto& [name, property] : config.at("property").as_object()) {
            Q_ASSERT(property.contains("type") && property.at("type").is_string());
            const auto prop_name = QString::fromUtf8(name);
            QString    property_type_name;
            if (const auto type_name = QString::fromUtf8(property.at("type").as_string()); type_name.startsWith(".")) {
                property_type_name = type_name.mid(1);
            } else if (const auto scoped_type_name = target_name + "." + type_name; context->contains_type(scoped_type_name)) {
                property_type_name = scoped_type_name;
            } else {
                property_type_name = type_name;
            }
            const auto ref_type = context->type(property_type_name);
            Q_ASSERT(ref_type);
            if (false) {
            } else if (ref_type->name == "int") {
                Q_ASSERT(!property.contains("min") || property.at("min").is_number());
                Q_ASSERT(!property.contains("max") || property.at("max").is_number());
                Q_ASSERT(!property.contains("step") || property.at("step").is_number());
                auto type    = PropertyType::make_int();
                bool differs = false;
                if (const int minval = property.get("min", type->minval); minval != type->minval) {
                    type->minval = minval;
                    differs      = true;
                }
                if (const int maxval = property.get("max", type->maxval); maxval != type->maxval) {
                    type->maxval = maxval;
                    differs      = true;
                }
                if (const int step = property.get("step", type->step); step != type->step) {
                    type->step = step;
                    differs    = true;
                }
                if (differs) {
                    const auto opt_private_type_name = context->add_private_type(type);
                    Q_ASSERT(opt_private_type_name.has_value());
                    property_type_name = opt_private_type_name.value();
                } else {
                    delete type;
                }
            } else if (ref_type->name == "real") {
                Q_ASSERT(!property.contains("min") || property.at("min").is_number());
                Q_ASSERT(!property.contains("max") || property.at("max").is_number());
                Q_ASSERT(!property.contains("step") || property.at("step").is_number());
                auto type    = PropertyType::make_real();
                bool differs = false;
                if (const double minval = property.get("min", type->minval); minval != type->minval) {
                    type->minval = minval;
                    differs      = true;
                }
                if (const double maxval = property.get("max", type->maxval); maxval != type->maxval) {
                    type->maxval = maxval;
                    differs      = true;
                }
                if (const double step = property.get("step", type->step); step != type->step) {
                    type->step = step;
                    differs    = true;
                }
                if (differs) {
                    const auto opt_private_type_name = context->add_private_type(type);
                    Q_ASSERT(opt_private_type_name.has_value());
                    property_type_name = opt_private_type_name.value();
                } else {
                    delete type;
                }
            } else if (ref_type->name == "str") {
                Q_ASSERT(!property.contains("values") || property.at("values").is_array());
                Q_ASSERT(!property.get("values", json::array()).empty());
                Q_ASSERT(property.get("values", json::array()).all<std::string>());
                if (property.contains("values")) {
                    auto        type = PropertyType::make_str();
                    QStringList values;
                    for (const auto item : property.at("values").as_array()) {
                        values.append(QString::fromUtf8(item.as_string()));
                    }
                    type->opt_values                 = std::make_optional(values);
                    const auto opt_private_type_name = context->add_private_type(type);
                    Q_ASSERT(opt_private_type_name.has_value());
                    property_type_name = opt_private_type_name.value();
                }
            }
            const bool added = context->add_property(target_name, prop_name, property_type_name, property.at("default"));
            Q_ASSERT(added);
        }
    }

    auto resp          = std::make_shared<TaskInterface>();
    resp->tasks        = std::move(tasks);
    resp->prop_context = std::move(context);
    return resp;
}

json::object TaskInterface::dump_props() const {
    json::object props;
    for (const auto& task_name : tasks.keys()) {
        if (!prop_context->has_properties(task_name)) { continue; }
        const auto key_task = task_name.toStdString();
        for (const auto& prop_name : prop_context->properties(task_name)) {
            if (!prop_context->has_non_default_value(task_name, prop_name)) { continue; }
            const auto  key_prop      = prop_name.toStdString();
            const auto& prop          = prop_context->property(task_name, prop_name)->get();
            props[key_task][key_prop] = prop_context->serialize(prop).value();
        }
    }
    return props;
}

bool TaskInterface::load_props(const json::object& props) {
    for (const auto& [task_name, prop_map] : props) {
        if (!prop_map.is_object()) { return false; }
        const auto  target     = QString::fromUtf8(task_name);
        const auto& properties = prop_context->properties(target);
        if (prop_map.as_object().size() != properties.size()) { return false; }
        for (const auto& [prop_name, prop] : prop_map.as_object()) {
            const auto name = QString::fromUtf8(prop_name);
            if (!properties.contains(name)) { return false; }
            const auto type = prop_context->property_type(target, name);
            if (!prop_context->verify_value(type, prop)) { return false; }
        }
    }

    for (const auto& [task_name, prop_map] : props) {
        const auto target = QString::fromUtf8(task_name);
        for (const auto& [prop_name, prop] : prop_map.as_object()) {
            const auto name     = QString::fromUtf8(prop_name);
            const auto type     = prop_context->property_type(target, name);
            auto&      property = prop_context->property(target, name).value().get();
            property.value()    = prop_context->deserialize(type, prop)->value();
        }
    }

    return true;
}

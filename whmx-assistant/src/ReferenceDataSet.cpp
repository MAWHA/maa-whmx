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

#include "ReferenceDataSet.h"

#include <filesystem>
#include <fstream>
#include <QtCore/QDebug>

namespace fs = std::filesystem;

namespace Ref {

std::optional<ResearchAnecdoteOption> ResearchAnecdoteOption::parse(const json::value &value) {
    if (!value.is_object()) { return std::nullopt; }

    ResearchAnecdoteOption resp;
    resp.name     = value.get("name", "");
    resp.positive = value.get("positive", false);

    const auto &type = value.get("type", "normal");
    if (type == "normal") {
        resp.type = ResearchAnecdoteOption::Normal;
    } else if (type == "random") {
        resp.type = ResearchAnecdoteOption::Random;
    } else {
        resp.type = ResearchAnecdoteOption::Normal;
    }

    return std::make_optional(std::move(resp));
}

std::optional<ResearchAnecdoteOptionGroup> ResearchAnecdoteOptionGroup::parse(const json::value &value) {
    if (!value.is_object()) { return std::nullopt; }

    ResearchAnecdoteOptionGroup resp;
    resp.recommended = value.get("recommended", -1);

    if (value.contains("content")) {
        resp.content = value.at("content").as_string();
    } else {
        return std::nullopt;
    }

    if (value.contains("options")) {
        for (const auto &option : value.at("options").as_array()) {
            if (auto opt = ResearchAnecdoteOption::parse(option)) {
                resp.options.emplace_back(std::move(opt.value()));
            } else {
                return std::nullopt;
            }
        }
    }

    return std::make_optional(std::move(resp));
}

std::optional<ResearchAnecdoteRecord> ResearchAnecdoteRecord::parse(const std::string &name, const json::value &value) {
    if (!value.is_array()) { return std::nullopt; }

    ResearchAnecdoteRecord resp;
    resp.name_ = name;

    for (const auto &option_group : value.as_array()) {
        if (auto opt = ResearchAnecdoteOptionGroup::parse(option_group)) {
            resp.option_stages_.emplace_back(std::move(opt.value()));
        } else {
            return std::nullopt;
        }
    }

    return std::make_optional(std::move(resp));
}

std::optional<ResearchAnecdoteEntry> ResearchAnecdoteEntry::parse(const std::string &category, const json::value &value) {
    if (!value.is_object()) { return std::nullopt; }

    ResearchAnecdoteEntry resp;
    resp.category_ = category;

    for (const auto &[name, entry] : value.as_object()) {
        if (auto opt = ResearchAnecdoteRecord::parse(name, entry)) {
            resp.entries_.insert_or_assign(name, std::move(opt.value()));
        } else {
            return std::nullopt;
        }
    }

    return std::make_optional(std::move(resp));
}

std::optional<ResearchAnecdoteSet> ResearchAnecdoteSet::parse(const json::value &value) {
    if (!value.is_object()) { return std::nullopt; }

    ResearchAnecdoteSet resp;

    for (const auto &[category, entry] : value.as_object()) {
        if (auto opt = ResearchAnecdoteEntry::parse(category, entry)) {
            resp.entries_.insert_or_assign(category, std::move(opt.value()));
        } else {
            return std::nullopt;
        }
    }

    return std::make_optional(std::move(resp));
}

std::shared_ptr<ResearchAnecdoteSet> ResearchAnecdoteSet::instance() {
    static std::shared_ptr<ResearchAnecdoteSet> instance;
    if (!instance) { instance = std::make_shared<ResearchAnecdoteSet>(); }
    return instance;
}

bool ResearchAnecdoteSet::load(const std::string &path) {
    //! FIMXE: resolve concurrent issues
    if (!fs::exists(path)) { return false; }
    std::ifstream fin(path);
    std::string   raw((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    if (auto opt = ResearchAnecdoteSet::parse(raw)) {
        *this = std::move(opt.value());
    } else {
        return false;
    }
    return true;
}

} // namespace Ref

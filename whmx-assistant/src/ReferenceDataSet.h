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

#include <meojson/json.hpp>
#include <map>
#include <string>
#include <optional>
#include <vector>
#include <memory>

namespace Ref {

class ResearchAnecdoteOption {
public:
    enum TypeKind {
        Random,
        Normal,
    };

public:
    static std::optional<ResearchAnecdoteOption> parse(const std::string &raw_text) {
        if (const auto opt = json::parse(raw_text)) {
            return parse(opt.value());
        } else {
            return std::nullopt;
        }
    }

    static std::optional<ResearchAnecdoteOption> parse(const json::value &value);

public:
    std::string text;
    TypeKind    type;
    bool        positive;
    int         next_entry_hint;
};

class ResearchAnecdoteOptionGroup {
public:
    static std::optional<ResearchAnecdoteOptionGroup> parse(const std::string &raw_text) {
        if (const auto opt = json::parse(raw_text)) {
            return parse(opt.value());
        } else {
            return std::nullopt;
        }
    }

    static std::optional<ResearchAnecdoteOptionGroup> parse(const json::value &value);

    bool has_recommended_option() const {
        return recommended != -1;
    }

    const ResearchAnecdoteOption &recommended_option() const {
        return options.at(recommended);
    }

public:
    std::string                         content;
    int                                 recommended;
    std::vector<ResearchAnecdoteOption> options;
};

class ResearchAnecdoteRecord {
public:
    static std::optional<ResearchAnecdoteRecord> parse(const std::string &name, const std::string &raw_text) {
        if (const auto opt = json::parse(raw_text)) {
            return parse(name, opt.value());
        } else {
            return std::nullopt;
        }
    }

    static std::optional<ResearchAnecdoteRecord> parse(const std::string &name, const json::value &value);

    std::string name() const {
        return name_;
    }

    size_t total_stages() const {
        return option_stages_.size();
    }

    const ResearchAnecdoteOptionGroup &stage(int index) const {
        return option_stages_.at(index);
    }

    auto as_stage_range() const {
        class Range {
        public:
            Range(const ResearchAnecdoteRecord &owner)
                : owner_(owner) {}

            auto begin() const {
                return owner_.option_stages_.begin();
            }

            auto end() const {
                return owner_.option_stages_.end();
            }

        private:
            const ResearchAnecdoteRecord &owner_;
        };

        return Range(*this);
    }

private:
    std::string                              name_;
    std::vector<ResearchAnecdoteOptionGroup> option_stages_;
};

class ResearchAnecdoteEntry {
public:
    static std::optional<ResearchAnecdoteEntry> parse(const std::string &category, const std::string &raw_text) {
        if (const auto opt = json::parse(raw_text)) {
            return parse(category, opt.value());
        } else {
            return std::nullopt;
        }
    }

    static std::optional<ResearchAnecdoteEntry> parse(const std::string &category, const json::value &value);

    std::optional<std::reference_wrapper<const ResearchAnecdoteRecord>> entry(const std::string &name) const {
        return entries_.contains(name) ? std::make_optional(std::ref(entries_.at(name))) : std::nullopt;
    }

    std::string category() const {
        return category_;
    }

    std::vector<std::string> entry_names() const {
        std::vector<std::string> list;
        for (const auto &[name, _] : entries_) { list.push_back(name); }
        return list;
    }

private:
    std::string                                   category_;
    std::map<std::string, ResearchAnecdoteRecord> entries_;
};

struct ResearchAnecdoteSet {
public:
    static std::optional<ResearchAnecdoteSet> parse(const std::string &raw_text) {
        if (const auto opt = json::parse(raw_text)) {
            return parse(opt.value());
        } else {
            return std::nullopt;
        }
    }

    static std::optional<ResearchAnecdoteSet> parse(const json::value &value);

    static std::shared_ptr<ResearchAnecdoteSet> instance();

    bool load(const std::string &path);

    std::optional<std::reference_wrapper<const ResearchAnecdoteEntry>> entry(const std::string &name) const {
        return entries_.contains(name) ? std::make_optional(std::ref(entries_.at(name))) : std::nullopt;
    }

    std::vector<std::string> categories() const {
        std::vector<std::string> list;
        for (const auto &[category, _] : entries_) { list.push_back(category); }
        return list;
    }

    bool contains(const std::string &category) const {
        return entries_.contains(category);
    }

private:
    std::map<std::string, ResearchAnecdoteEntry> entries_;
};

}; // namespace Ref

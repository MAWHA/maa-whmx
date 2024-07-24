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

#include "Decode.h"

#include <algorithm>

std::optional<OcrRecord> parse_and_get_best_ocr_record(const json::value &result) {
    if (!result.contains("all")) { return std::nullopt; }
    if (result.contains("best")) {
        const auto   best  = result.at("best");
        const auto  &text  = best.at("text").as_string();
        const double score = best.at("score").as_double();
        return std::make_optional(OcrRecord{text, score});
    }
    std::vector<OcrRecord> records;
    for (const auto &record : result.at("all").as_array()) {
        const auto  &text  = record.at("text").as_string();
        const double score = record.at("score").as_double();
        records.emplace_back(OcrRecord{text, score});
    }
    if (records.empty()) { return std::nullopt; }
    std::sort(records.begin(), records.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.score > rhs.score;
    });
    return std::make_optional(records.front());
}

std::optional<OcrRecord> parse_and_get_full_text_ocr_result(const json::value &result) {
    struct Record {
        OcrRecord data;
        int       y_pos;
    };

    if (!result.contains("all")) { return std::nullopt; }
    std::vector<Record> records;
    for (const auto &item : result.at("all").as_array()) {
        Record record;
        record.data.text  = item.at("text").as_string();
        record.data.score = item.at("score").as_double();
        record.y_pos      = item.at("box").as_array().at(1).as_integer();
        records.push_back(record);
    }

    std::sort(records.begin(), records.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.y_pos <= rhs.y_pos;
    });

    double weighted_score = 0.0;
    int    total_chars    = 0;

    std::string full_text;
    for (const auto &record : records) {
        full_text      += record.data.text;
        weighted_score += record.data.text.size() * record.data.score;
        total_chars    += record.data.text.size();
    }

    return std::make_optional(OcrRecord{full_text, weighted_score / std::max(total_chars, 1)});
}

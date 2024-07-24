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

#include <string>
#include <optional>
#include <meojson/json.hpp>

struct OcrRecord {
    std::string text;
    double      score;
};

std::optional<OcrRecord> parse_and_get_best_ocr_record(const json::value &result);
std::optional<OcrRecord> parse_and_get_full_text_ocr_result(const json::value &result);
json::value              unwrap_custom_recognizer_analyze_result(std::string_view rec_detail);

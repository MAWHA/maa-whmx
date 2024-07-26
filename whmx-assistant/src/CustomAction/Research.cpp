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

#include "Research.h"
#include "../Decode.h"

#include <array>
#include <vector>
#include <algorithm>
#include <random>
#include <utility>
#include <QtCore/QDebug>

namespace Action::Research {

using namespace maa;

bool SelectGradeOption::parse_params(SelectGradeOptionParam &param_out, MaaStringView raw_param) {
    using Policy = SelectGradeOptionParam::Policy;
    using Mode   = SelectGradeOptionParam::Mode;

    auto opt_param = json::parse(raw_param);
    if (!opt_param.has_value()) { return false; }

    const auto &param = opt_param.value().as_object();

    const auto policy = param.get("policy", "auto");
    if (false) {
    } else if (policy == "auto") {
        param_out.policy = Policy::Auto;
    } else if (policy == "greedy") {
        param_out.policy = Policy::Greedy;
    } else if (policy == "defensive") {
        param_out.policy = Policy::Defensive;
    } else {
        param_out.policy = Policy::Auto;
    }

    if (!param.contains("mode")) { return false; }
    const auto mode = param.at("mode").as_string();
    if (false) {
    } else if (mode == "upgrade") {
        param_out.mode = Mode::Upgrade;
    } else if (mode == "downgrade") {
        param_out.mode = Mode::Downgrade;
    } else {
        return false;
    }

    return true;
}

coro::Promise<bool> SelectGradeOption::research__select_grade_option(
    std::shared_ptr<SyncContext> context,
    MaaStringView                task_name,
    MaaStringView                param,
    const MaaRect               &cur_box,
    MaaStringView                cur_rec_detail) {
    SelectGradeOptionParam opt;
    if (!parse_params(opt, param)) {
        qDebug("%s: invalid arguments", task_name);
        co_return false;
    }

    struct FaceInfo {
        int     index;
        int     grade;
        MaaRect box;
    };

    const auto face_data = unwrap_custom_recognizer_analyze_result(cur_rec_detail);

    std::array<FaceInfo, 6> faces;
    for (const auto &obj : face_data.as_array()) {
        const auto &face_in  = obj.as_object();
        const int   index    = face_in.at("index").as_integer();
        auto       &face_out = faces[index];
        face_out.index       = index;
        face_out.grade       = face_in.at("grade").as_integer();
        const auto &box      = face_in.at("box").as_array();
        face_out.box.x       = box[0].as_integer();
        face_out.box.y       = box[1].as_integer();
        face_out.box.width   = box[2].as_integer();
        face_out.box.height  = box[3].as_integer();
    }

    int total_max_grade     = 0;
    int total_min_grade     = 0;
    int total_unknown_grade = 0;
    for (const auto &[_0, grade, _2] : faces) {
        if (false) {
        } else if (grade == 0) {
            ++total_min_grade;
        } else if (grade == 2) {
            ++total_max_grade;
        } else if (grade == -1) {
            ++total_unknown_grade;
        }
    }

    //! TODO: impl solution to unknown grade due failure of recognition
    if (total_unknown_grade > 0) { co_return false; }

    std::vector<FaceInfo> sorted_faces(faces.begin(), faces.end());
    std::sort(sorted_faces.begin(), sorted_faces.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.grade < rhs.grade;
    });

    std::mt19937 rng(std::random_device{}());

    int selected_face = -1;
    switch (opt.mode) {
        case SelectGradeOptionParam::Mode::Upgrade: {
            if (total_max_grade == 6) { co_return true; }
            const auto it    = std::find_if(sorted_faces.begin(), sorted_faces.end(), [](const auto &face) {
                return face.grade == 2;
            });
            const int  first = 0;
            const int  last  = it == sorted_faces.end() ? 6 : std::distance(sorted_faces.begin(), it);
            if (first >= last) { std::unreachable(); }
            switch (opt.policy) {
                case SelectGradeOptionParam::Policy::Auto: {
                    selected_face = sorted_faces[rng() % (last - first) + first].index;
                } break;
                case SelectGradeOptionParam::Policy::Greedy: {
                    selected_face = sorted_faces[last - 1].index;
                } break;
                case SelectGradeOptionParam::Policy::Defensive: {
                    selected_face = sorted_faces[first].index;
                } break;
            }
        } break;
        case SelectGradeOptionParam::Mode::Downgrade: {
            if (total_min_grade == 6) { co_return true; }
            const auto it    = std::find_if(sorted_faces.rbegin(), sorted_faces.rend(), [](const auto &face) {
                return face.grade == 0;
            });
            const int  first = it == sorted_faces.rend() ? 0 : std::distance(sorted_faces.rbegin(), it);
            const int  last  = 6;
            if (first >= last) { std::unreachable(); }
            switch (opt.policy) {
                case SelectGradeOptionParam::Policy::Auto: {
                    selected_face = sorted_faces[rng() % (last - first) + first].index;
                } break;
                case SelectGradeOptionParam::Policy::Greedy: {
                    selected_face = sorted_faces[first].index;
                } break;
                case SelectGradeOptionParam::Policy::Defensive: {
                    selected_face = sorted_faces[last - 1].index;
                } break;
            }
        } break;
    }

    if (selected_face != -1) {
        const auto &face    = faces[selected_face];
        const auto &box     = face.box;
        const int   click_x = box.x + box.width / 2;
        const int   click_y = box.y + box.height / 2;
        co_await context->click(click_x, click_y);
    }

    co_return true;
}

bool ResolveAnecdote::parse_params(ResolveAnecdoteParam &param_out, MaaStringView raw_param) {
    Q_UNIMPLEMENTED();
    return true;
}

coro::Promise<bool> ResolveAnecdote::research__resolve_anecdote(
    std::shared_ptr<SyncContext> context,
    MaaStringView                task_name,
    MaaStringView                param,
    const MaaRect               &cur_box,
    MaaStringView                cur_rec_detail) {
    qDebug().noquote() << QString::fromUtf8(unwrap_custom_recognizer_analyze_result(cur_rec_detail).to_string());
    Q_UNIMPLEMENTED();
    co_return true;
}

coro::Promise<bool> PerformItemPairsMatch::research__perform_item_pairs_match(
    std::shared_ptr<SyncContext> context,
    MaaStringView                task_name,
    MaaStringView                param,
    const MaaRect               &cur_box,
    MaaStringView                cur_rec_detail) {
    const int     n_hori      = 4;
    const int     n_vert      = 3;
    const int     total_items = n_hori * n_vert;
    const MaaRect roi_all{550, 152, 596, 478};
    const int     roi_width  = roi_all.width / n_hori;
    const int     roi_height = roi_all.height / n_vert;

    std::vector<std::pair<int, int>> item_pairs;
    for (const auto data = unwrap_custom_recognizer_analyze_result(cur_rec_detail); const auto &pair : data.as_array()) {
        item_pairs.push_back({pair.at(0).as_integer(), pair.at(1).as_integer()});
    }

    qDebug() << "wait matching game to start";
    co_await context->run_task("Research.WaitMatchingGameToStart");

    qDebug() << "perform item pairs match";
    for (const auto &pair : item_pairs) {
        qDebug().noquote() << QString("match pair (%1)[row=%2,col=%3], (%4)[row=%5,col=%6]")
                                  .arg(pair.first)
                                  .arg(pair.first / n_hori + 1)
                                  .arg(pair.first % n_hori + 1)
                                  .arg(pair.second)
                                  .arg(pair.second / n_hori + 1)
                                  .arg(pair.second % n_hori + 1);
        for (const int item : std::initializer_list<int>{pair.first, pair.second}) {
            const int row      = item / n_hori;
            const int col      = item % n_hori;
            const int center_x = roi_all.x + col * roi_width + roi_width / 2;
            const int center_y = roi_all.y + row * roi_height + roi_height / 2;
            co_await context->click(center_x, center_y);
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    co_return true;
}

} // namespace Action::Research

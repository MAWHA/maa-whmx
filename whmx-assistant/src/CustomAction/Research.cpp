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
#include "../Algorithm.h"
#include "../ReferenceDataSet.h"
#include "../Task/Config.h"
#include "../Task/TaskParam.h"

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
    const int opt_size_w  = 350;
    const int opt_size_h  = 28;
    const int opt_x       = 620;
    const int first_opt_y = 400;
    const int opt_dy      = 68;

    const auto anecdote_data = unwrap_custom_recognizer_analyze_result(cur_rec_detail);

    const auto category = anecdote_data.at("category").as_string();
    const auto name     = anecdote_data.at("name").as_string();
    const int  stage    = anecdote_data.at("stage").as_integer();

    const auto &anecdote_entry = Ref::ResearchAnecdoteSet::instance()->entry(category).value().get().entry(name).value().get();

    int current_stage = stage;
    while (current_stage >= 0 && current_stage < anecdote_entry.total_stages()) {
        const auto &event_stage = anecdote_entry.stage(current_stage);
        if (event_stage.options.empty()) {
            qCritical() << "no options for stage" << current_stage << "of anecdote" << category << name;
            co_return false;
        }

        int best_choice = -1;
        if (event_stage.has_recommended_option()) {
            best_choice = event_stage.recommended;
        } else {
            for (int i = 0; i < event_stage.options.size(); ++i) {
                if (event_stage.options[i].positive) {
                    best_choice = i;
                    break;
                }
            }
        }
        if (best_choice == -1) {
            qWarning() << "no positive option for stage" << current_stage << "of anecdote" << category << name
                       << ", make random choice";
            best_choice = choice(0, event_stage.options.size() - 1);
        }

        const auto &option = event_stage.options[best_choice];

        //! TODO: check whether the given option is valid
        const int click_y_pos = first_opt_y + best_choice * opt_dy + opt_size_h / 2;
        const int click_x_pos = opt_x + opt_size_w / 4;

        co_await context->click(click_x_pos, click_y_pos);

        //! FIXME: not support random option yet

        const bool has_next = option.next_entry_hint >= 0 && option.next_entry_hint < anecdote_entry.total_stages();
        if (has_next) { co_await context->run_task("Research.ResolveResultOfAnecdoteChoice"); }

        current_stage = option.next_entry_hint;
    }

    co_await context->run_task("Research.ResolveGotExtraResourceOnEventDone");

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

coro::Promise<bool> ResolveBuffSelection::research__resolve_buff_selection(
    std::shared_ptr<SyncContext> context,
    MaaStringView                task_name,
    MaaStringView                param,
    const MaaRect               &cur_box,
    MaaStringView                cur_rec_detail) {
    QStringList preferred_buffs;
    if (false) {
        //! TODO: enable input param to override the value provieded by shared-task-config
    } else if (const auto config = Task::shared_task_config()) {
        const auto param = config->task_params.value(Task::MajorTask::DoResearch, QVariant::fromValue(Task::DoResearchParam{}));
        preferred_buffs  = param.value<Task::DoResearchParam>().buff_preference;
    }

    QStringList buff_names;
    for (const auto data = unwrap_custom_recognizer_analyze_result(cur_rec_detail); const auto &buff : data.as_array()) {
        buff_names.append(QString::fromUtf8(buff.as_string()));
    }

    const int center_pos_x = 640;
    const int pos_y        = 340;
    const int dx           = 274;

    bool need_select     = true;
    int  choice_expected = 0;

    if (const int total_buffs = buff_names.size(); total_buffs == 1) {
        need_select = false;
    } else if (total_buffs == 3) {
        choice_expected = 1;
    } else if (total_buffs == 5) {
        choice_expected = 2;
    } else {
        qCritical() << "invalid number of buffs" << total_buffs;
        co_return false;
    }

    QList<int> choices;

    if (need_select) {
        Q_ASSERT(choice_expected > 0);
        for (const auto &preferred_buff : preferred_buffs) {
            const int index = buff_names.indexOf(preferred_buff);
            if (index == -1) { continue; }
            choices.append(index);
            if (choices.size() >= choice_expected) { break; }
        }
        Q_ASSERT(choices.size() <= choice_expected);
        if (choices.size() < choice_expected) {
            QList<int> candidates;
            for (int i = 0; i < buff_names.size(); ++i) {
                if (choices.contains(i)) { continue; }
                candidates.append(i);
            }
            const auto indicies = multi_choice(choice_expected, 0, candidates.size() - 1);
            for (const int index : indicies) { choices.append(candidates[index]); }
        }
    } else {
        choices.append(0);
    }

    {
        QStringList selected_buffs;
        for (const int choice_index : choices) { selected_buffs.append(QString("\"%1\"").arg(buff_names[choice_index])); }
        qInfo().noquote() << "select buffs: [" << selected_buffs.join(", ") << "]";
    }

    if (need_select) {
        for (const int choice_index : choices) {
            const int pos_x = center_pos_x + dx * (choice_index - buff_names.size() / 2);
            co_await context->click(pos_x, pos_y);
        }
        co_await context->run_task("Research.ConfirmBuffSelection");
    }

    co_await context->run_task("Research.ResolveGotBuff");

    co_return true;
}

} // namespace Action::Research

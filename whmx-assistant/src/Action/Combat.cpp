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

#include "Combat.h"
#include "../Decode.h"

#include <QtCore/QDebug>
#include <QtCore/QUuid>
#include <QtCore/QElapsedTimer>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace Action::Combat {

using namespace maa;

static json::object make_template_match_params(
    const std::string &template_image, const MaaRect &roi = MaaRect{0, 0, 0, 0}, bool green_mask = false) {
    const json::object params{
        {"recognition", "TemplateMatch"                                 },
        {"template",    template_image                                  },
        {"roi",         json::array{roi.x, roi.y, roi.width, roi.height}},
        {"green_mask",  green_mask                                      },
    };
    return json::object{
        {"TemplateMatch", params},
    };
}

static cv::Mat crop_image(const cv::Mat &src, const MaaRect &rect) {
    return src.rowRange(rect.y, rect.y + rect.height).colRange(rect.x, rect.x + rect.width);
}

coro::Promise<bool> FillSquad::combat__fill_squad(
    std::shared_ptr<SyncContext> context,
    MaaStringView                task_name,
    MaaStringView                param,
    const MaaRect               &cur_box,
    MaaStringView                cur_rec_detail) {
    const MaaRect squad_roi{26, 193, 917, 333};
    const int     slot_width           = 147;
    const int     total_slots          = 6;
    const int     gap_width            = (squad_roi.width - slot_width * total_slots) / (total_slots - 1);
    const auto    free_slot_template   = "Combat/FreeSlot.png";
    const auto    locked_slot_template = "Combat/MaskedLockedSlot.png";

    const auto tmp_task_name = QUuid::createUuid().toString().toStdString();

    auto screen = details::Image::make();
    co_await context->screencap(screen);

    int locked_place = total_slots;
    int joined_slots = 0;

    for (int i = 0; i < total_slots; ++i) {
        const MaaRect slot_roi{squad_roi.x + i * (slot_width + gap_width), squad_roi.y, slot_width, squad_roi.height};
        {
            const auto params = make_template_match_params(locked_slot_template, slot_roi, true);
            const auto resp   = co_await context->run_recognition(screen, "TemplateMatch", params);
            if (has_expected_match(resp.rec_detail)) {
                locked_place = i;
                break;
            }
        }
        {
            const auto params = make_template_match_params(free_slot_template, slot_roi);
            const auto resp   = co_await context->run_recognition(screen, "TemplateMatch", params);
            if (!has_expected_match(resp.rec_detail)) { continue; }
            ++joined_slots;
        }
        co_await context->click(slot_roi.x + slot_roi.width / 2, slot_roi.y + slot_roi.height / 2);
        co_await context->run_task("Combat.JoinSingleRole");
        co_await context->run_task("Combat.WaitSquadLoaded");
    }

    qInfo().noquote() << QString("Combat.FillSquad: found %1 locked slots, %2 roles joined")
                             .arg(total_slots - locked_place)
                             .arg(joined_slots);
    co_return true;
}

} // namespace Action::Combat

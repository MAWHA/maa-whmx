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

#include "Utils.h"

#include <QtCore/QDebug>
#include <QtCore/QUuid>
#include <QtCore/QElapsedTimer>

namespace Rec::Utils {

using namespace maa;

bool TwoStageTest::parse_params(TwoStageTestParam &param_out, MaaStringView raw_param) {
    auto opt_params = json::parse(raw_param);
    if (!opt_params.has_value()) { return false; }
    if (!opt_params->is_object()) { return false; }

    const auto &params = opt_params.value().as_object();

    if (!params.contains("prerequisite") || !params.at("prerequisite").is_string()) { return false; }
    if (!params.contains("recognition") || !params.at("recognition").is_object()) { return false; }

    param_out.prerequisite_task = params.at("prerequisite").as_string();
    param_out.recog_task        = params.at("recognition").as_object();
    param_out.timeout           = params.get("timeout", 20000);

    return true;
}

coro::Promise<AnalyzeResult> TwoStageTest::utils__two_stage_test(
    SyncContextHandle context, ImageHandle image, std::string_view task_name, std::string_view param) {
    AnalyzeResult resp;
    resp.result = false;

    TwoStageTestParam opt;
    if (!parse_params(opt, param.data())) {
        qDebug("%s: invalid arguments", task_name.data());
        co_return resp;
    }

    if (const auto done = co_await context->run_task(opt.prerequisite_task); !done) { co_return resp; }

    if (!opt.recog_task.contains("recognition")) {
        //! NOTE: direct-hit
        resp.result = true;
        co_return resp;
    }

    const auto         recognition = opt.recog_task.at("recognition").as_string();
    const json::object recog_param{
        {recognition, opt.recog_task},
    };

    QElapsedTimer timer;
    timer.start();
    do {
        co_await context->screencap(image);
        const auto recog_resp = co_await context->run_recognition(image, recognition, recog_param);
        const auto result     = json::parse(recog_resp.rec_detail).value_or(json::object()).as_object();
        Q_ASSERT(result.contains("best"));
        if (const auto best = result.at("best").as_object(); !best.empty()) { co_return recog_resp; }
    } while (timer.elapsed() < opt.timeout);

    co_return resp;
}

} // namespace Rec::Utils

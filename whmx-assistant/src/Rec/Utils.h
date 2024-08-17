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

#include <MaaPP/MaaPP.hpp>

namespace Rec::Utils {

struct TwoStageTestParam {
    std::string  prerequisite_task; //<! required
    json::object recog_task;        //<! required
    int          timeout;           //<! default: 20000, in milliseconds
};

class TwoStageTest {
public:
    static std::string name() {
        return "Utils.TwoStageTest";
    }

    static std::shared_ptr<maa::CustomRecognizer> make() {
        return maa::CustomRecognizer::make(&TwoStageTest::utils__two_stage_test);
    }

    static bool parse_params(TwoStageTestParam &param_out, MaaStringView raw_param);

private:
    static maa::coro::Promise<maa::AnalyzeResult> utils__two_stage_test(
        maa::SyncContextHandle context, maa::ImageHandle image, std::string_view task_name, std::string_view param);
};

} // namespace Rec::Utils

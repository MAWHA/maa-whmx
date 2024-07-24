#pragma once

#include <MaaPP/MaaPP.hpp>

namespace Rec::Research {

class ParseGradeOptionsOnModify {
public:
    static std::string name() {
        return "Research.ParseGradeOptionsOnModify";
    }

    static std::shared_ptr<maa::CustomRecognizer> make() {
        return maa::CustomRecognizer::make(&ParseGradeOptionsOnModify::research__parse_grade_options_on_modify);
    }

private:
    static maa::coro::Promise<maa::AnalyzeResult> research__parse_grade_options_on_modify(
        maa::SyncContextHandle context, maa::ImageHandle image, std::string_view task_name, std::string_view param);
};

} // namespace Rec::Research

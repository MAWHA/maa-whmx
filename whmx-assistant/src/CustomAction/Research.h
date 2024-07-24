#pragma once

#include <MaaPP/MaaPP.hpp>

namespace Action::Research {

struct SelectGradeOptionParam {
    enum class Policy {
        Auto,      //<! random select
        Greedy,    //<! intend to get more buff
        Defensive, //<! intend to avoid debuff
    };

    enum class Mode {
        Upgrade,
        Downgrade
    };

    Policy policy;
    Mode   mode;
};

class SelectGradeOption {
public:
    static std::string name() {
        return "Research.SelectGradeOption";
    }

    static std::shared_ptr<maa::CustomAction> make() {
        return maa::CustomAction::make(&SelectGradeOption::research__select_grade_option);
    }

    static bool parse_params(SelectGradeOptionParam &param_out, MaaStringView raw_param);

private:
    static maa::coro::Promise<bool> research__select_grade_option(
        std::shared_ptr<maa::SyncContext> context,
        MaaStringView                     task_name,
        MaaStringView                     param,
        const MaaRect                    &cur_box,
        MaaStringView                     cur_rec_detail);
};

} // namespace Action::Research

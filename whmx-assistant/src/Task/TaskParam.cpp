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

#include "TaskParam.h"

#include <string_view>

namespace Task {

std::shared_ptr<PropGetter> dump_task_param_to_prop_getter(const QVariant& param) {
    if (false) {
    } else if (param.typeName() == std::string_view("Task::GetFreeGiftPackParam")) {
        const auto value = param.value<Task::GetFreeGiftPackParam>();
        return std::shared_ptr<PropGetter>(new Task::GetFreeGiftPackParam(value));
    } else if (param.typeName() == std::string_view("Task::ConsumeExpiringVitalityPotionParam")) {
        const auto value = param.value<Task::ConsumeExpiringVitalityPotionParam>();
        return std::shared_ptr<PropGetter>(new Task::ConsumeExpiringVitalityPotionParam(value));
    } else if (param.typeName() == std::string_view("Task::PurchaseVitalityParam")) {
        const auto value = param.value<Task::PurchaseVitalityParam>();
        return std::shared_ptr<PropGetter>(new Task::PurchaseVitalityParam(value));
    } else if (param.typeName() == std::string_view("Task::PurchaseRandomProductParam")) {
        const auto value = param.value<Task::PurchaseRandomProductParam>();
        return std::shared_ptr<PropGetter>(new Task::PurchaseRandomProductParam(value));
    } else if (param.typeName() == std::string_view("Task::MopupResourceLevelParam")) {
        const auto value = param.value<Task::MopupResourceLevelParam>();
        return std::shared_ptr<PropGetter>(new Task::MopupResourceLevelParam(value));
    } else if (param.typeName() == std::string_view("Task::EnlistCharacterParam")) {
        const auto value = param.value<Task::EnlistCharacterParam>();
        return std::shared_ptr<PropGetter>(new Task::EnlistCharacterParam(value));
    } else if (param.typeName() == std::string_view("Task::RecruitCharacterParam")) {
        const auto value = param.value<Task::RecruitCharacterParam>();
        return std::shared_ptr<PropGetter>(new Task::RecruitCharacterParam(value));
    } else if (param.typeName() == std::string_view("Task::AssignOfficeProductParam")) {
        const auto value = param.value<Task::AssignOfficeProductParam>();
        return std::shared_ptr<PropGetter>(new Task::AssignOfficeProductParam(value));
    } else if (param.typeName() == std::string_view("Task::AssignWorkshopProductParam")) {
        const auto value = param.value<Task::AssignWorkshopProductParam>();
        return std::shared_ptr<PropGetter>(new Task::AssignWorkshopProductParam(value));
    } else if (param.typeName() == std::string_view("Task::AssignEquipmentOrderParam")) {
        const auto value = param.value<Task::AssignEquipmentOrderParam>();
        return std::shared_ptr<PropGetter>(new Task::AssignEquipmentOrderParam(value));
    } else if (param.typeName() == std::string_view("Task::ReplaceFullFavorabilityCharacterParam")) {
        const auto value = param.value<Task::ReplaceFullFavorabilityCharacterParam>();
        return std::shared_ptr<PropGetter>(new Task::ReplaceFullFavorabilityCharacterParam(value));
    } else if (param.typeName() == std::string_view("Task::ChatOverTeaParam")) {
        const auto value = param.value<Task::ChatOverTeaParam>();
        return std::shared_ptr<PropGetter>(new Task::ChatOverTeaParam(value));
    } else if (param.typeName() == std::string_view("Task::PlayMiZongPanParam")) {
        const auto value = param.value<Task::PlayMiZongPanParam>();
        return std::shared_ptr<PropGetter>(new Task::PlayMiZongPanParam(value));
    } else if (param.typeName() == std::string_view("Task::PlayFourInRowParam")) {
        const auto value = param.value<Task::PlayFourInRowParam>();
        return std::shared_ptr<PropGetter>(new Task::PlayFourInRowParam(value));
    } else if (param.typeName() == std::string_view("Task::SwitchFurnitureLayoutParam")) {
        const auto value = param.value<Task::SwitchFurnitureLayoutParam>();
        return std::shared_ptr<PropGetter>(new Task::SwitchFurnitureLayoutParam(value));
    } else if (param.typeName() == std::string_view("Task::DoResearchParam")) {
        const auto value = param.value<Task::DoResearchParam>();
        return std::shared_ptr<PropGetter>(new Task::DoResearchParam(value));
    } else if (param.typeName() == std::string_view("Task::PurchasePiecesOfPaintingParam")) {
        const auto value = param.value<Task::PurchasePiecesOfPaintingParam>();
        return std::shared_ptr<PropGetter>(new Task::PurchasePiecesOfPaintingParam(value));
    } else if (param.typeName() == std::string_view("Task::PlayDongguCompetitionParam")) {
        const auto value = param.value<Task::PlayDongguCompetitionParam>();
        return std::shared_ptr<PropGetter>(new Task::PlayDongguCompetitionParam(value));
    } else if (param.typeName() == std::string_view("Task::PurchaseDongguProductParam")) {
        const auto value = param.value<Task::PurchaseDongguProductParam>();
        return std::shared_ptr<PropGetter>(new Task::PurchaseDongguProductParam(value));
    } else if (param.typeName() == std::string_view("Task::GetDongguRewardParam")) {
        const auto value = param.value<Task::GetDongguRewardParam>();
        return std::shared_ptr<PropGetter>(new Task::GetDongguRewardParam(value));
    } else {
        std::unreachable();
    }
}

} // namespace Task

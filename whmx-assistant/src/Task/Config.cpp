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

#include "Config.h"
#include "TaskParam.h"
#include "Router.h"

#include <QtCore/QFile>
#include <QtCore/QMap>
#include <meojson/json.hpp>
#include <magic_enum.hpp>

namespace Task {

std::shared_ptr<Config> SHARED_TASK_CONFIG;

void reset_shared_task_config(const std::shared_ptr<Config> config) {
    SHARED_TASK_CONFIG = config;
}

std::shared_ptr<Config> shared_task_config() {
    return SHARED_TASK_CONFIG;
}

bool load_task_config(Config& config, const QString& file_path, std::shared_ptr<Router> router) {
    //! TODO: parse task_params

    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly)) { return false; }

    const auto data = json::parse(QString::fromUtf8(file.readAll()).toStdString());
    if (!data.has_value()) { return false; }

    if (!data->contains("bindings")) { return false; }

    QMap<QString, MajorTask> rev_index_table;
    for (const auto& major_task : magic_enum::enum_values<MajorTask>()) {
        rev_index_table.insert(QString::fromLatin1(magic_enum::enum_name(major_task)), major_task);
    }

    const auto bindings = data->at("bindings");
    for (const auto& binding : bindings.as_array()) {
        if (!binding.contains("major_task")) { continue; }
        if (!binding.contains("entry")) { continue; }
        const auto major_task_name = QString::fromUtf8(binding.at("major_task").as_string());
        const auto task_entry      = QString::fromUtf8(binding.at("entry").as_string());
        if (task_entry.isEmpty()) { continue; }
        if (!rev_index_table.contains(major_task_name)) { continue; }
        const auto major_task           = rev_index_table.value(major_task_name);
        config.task_entries[major_task] = task_entry;
    }

    if (router && data->contains("router") && data->at("router").is_object()) {
        Router::parse(*router, data->at("router").as_object());
    }

    return true;
}

QVariant get_default_task_param(MajorTask task) {
    switch (task) {
        case MajorTask::GetFreeGiftPack: {
            return QVariant::fromValue(Task::GetFreeGiftPackParam{});
        } break;
        case MajorTask::ConsumeExpiringVitalityPotion: {
            return QVariant::fromValue(Task::ConsumeExpiringVitalityPotionParam{});
        } break;
        case MajorTask::PurchaseVitality: {
            return QVariant::fromValue(Task::PurchaseVitalityParam{});
        } break;
        case MajorTask::PurchaseRandomProduct: {
            return QVariant::fromValue(Task::PurchaseRandomProductParam{});
        } break;
        case MajorTask::MopupResourceLevel: {
            return QVariant::fromValue(Task::MopupResourceLevelParam{});
        } break;
        case MajorTask::EnlistCharacter: {
            return QVariant::fromValue(Task::EnlistCharacterParam{});
        } break;
        case MajorTask::RecruitCharacter: {
            return QVariant::fromValue(Task::RecruitCharacterParam{});
        } break;
        case MajorTask::AssignOfficeProduct: {
            return QVariant::fromValue(Task::AssignOfficeProductParam{});
        } break;
        case MajorTask::AssignWorkshopProduct: {
            return QVariant::fromValue(Task::AssignWorkshopProductParam{});
        } break;
        case MajorTask::AssignEquipmentOrder: {
            return QVariant::fromValue(Task::AssignEquipmentOrderParam{});
        } break;
        case MajorTask::ReplaceFullFavorabilityCharacter: {
            return QVariant::fromValue(Task::ReplaceFullFavorabilityCharacterParam{});
        } break;
        case MajorTask::ChatOverTea: {
            return QVariant::fromValue(Task::ChatOverTeaParam{});
        } break;
        case MajorTask::PlayMiZongPan: {
            return QVariant::fromValue(Task::PlayMiZongPanParam{});
        } break;
        case MajorTask::PlayFourInRow: {
            return QVariant::fromValue(Task::PlayFourInRowParam{});
        } break;
        case MajorTask::SwitchFurnitureLayout: {
            return QVariant::fromValue(Task::SwitchFurnitureLayoutParam{});
        } break;
        case MajorTask::DoResearch: {
            return QVariant::fromValue(Task::DoResearchParam{});
        } break;
        case MajorTask::PurchasePiecesOfPainting: {
            return QVariant::fromValue(Task::PurchasePiecesOfPaintingParam{});
        } break;
        case MajorTask::PlayDongguCompetition: {
            return QVariant::fromValue(Task::PlayDongguCompetitionParam{});
        } break;
        case MajorTask::PurchaseDongguProduct: {
            return QVariant::fromValue(Task::PurchaseDongguProductParam{});
        } break;
        default: {
            std::unreachable();
        } break;
    }
}

} // namespace Task

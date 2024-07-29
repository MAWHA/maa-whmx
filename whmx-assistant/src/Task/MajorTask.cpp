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

#include "MajorTask.h"

#include <map>

namespace Task {

static std::map<MajorTask, TaskInfo> TASK_INFO_TABLE{
    {MajorTask::DailySignin,
     {
         .name       = "签到",
         .routine    = true,
         .has_config = false,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::GetFreeGiftPack,
     {
         .name       = "商亭免费礼包",
         .routine    = true,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::ConsumeExpiringVitalityPotion,
     {
         .name       = "消费临期体力水",
         .routine    = true,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::PurchaseVitality,
     {
         .name       = "购买体力",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::PurchaseRandomProduct,
     {
         .name       = "易物所采购",
         .routine    = true,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::GetRoutineTaskReward,
     {
         .name       = "日常/周常任务",
         .routine    = true,
         .has_config = false,
         .scheduable = true,
         .difficult  = false,
     }},
    {MajorTask::GetJourneyReward,
     {
         .name       = "游历",
         .routine    = true,
         .has_config = false,
         .scheduable = true,
         .difficult  = false,
     }},
    {MajorTask::ReceiveMail,
     {
         .name       = "邮件",
         .routine    = true,
         .has_config = false,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::MopupResourceLevel,
     {
         .name       = "演训-基础训练",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::EnlistCharacter,
     {
         .name       = "器者征集",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::RecruitCharacter,
     {
         .name       = "器者招集",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::SpecialActivitySignin,
     {
         .name       = "活动签到",
         .routine    = true,
         .has_config = false,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::AssignOfficeProduct,
     {
         .name       = "派遣公司-办公室制造替换",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::AssignWorkshopProduct,
     {
         .name       = "派遣公司-工作间制造替换",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::FindCat,
     {
         .name       = "派遣公司-找猫猫",
         .routine    = false,
         .has_config = false,
         .scheduable = false,
         .difficult  = true,
     }},
    {MajorTask::AssignEquipmentOrder,
     {
         .name       = "派遣公司-装备订购",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::GetOrderedEquipment,
     {
         .name       = "派遣公司-装备订购获取",
         .routine    = true,
         .has_config = false,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::GetOrderedProduct,
     {
         .name       = "派遣公司-资源获取",
         .routine    = true,
         .has_config = false,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::GetFavorability,
     {
         .name       = "派遣公司-感应获取",
         .routine    = true,
         .has_config = false,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::GetAccumulatedVitality,
     {
         .name       = "派遣公司-体力获取",
         .routine    = true,
         .has_config = false,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::ReplaceFullFavorabilityCharacter,
     {
         .name       = "派遣公司-满感应度器者替换",
         .routine    = true,
         .has_config = true,
         .scheduable = true,
         .difficult  = false,
     }},
    {MajorTask::ChatOverTea,
     {
         .name       = "派遣公司-品茗",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::PlayMergeGame,
     {
         .name       = "派遣公司-和合",
         .routine    = false,
         .has_config = false,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::PlayMiZongPan,
     {
         .name       = "派遣公司-迷踪盘",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = true,
     }},
    {MajorTask::PlayFourInARow,
     {
         .name       = "派遣公司-四子棋",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::GetFurnitureBlueprint,
     {
         .name       = "派遣公司-家具蓝图获取",
         .routine    = true,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::SwitchFurnitureLayout,
     {
         .name       = "派遣公司-家具布局替换",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::GetResearchReward,
     {
         .name       = "博物研学-周常奖励",
         .routine    = false,
         .has_config = false,
         .scheduable = true,
         .difficult  = false,
     }},
    {MajorTask::DoResearch,
     {
         .name       = "博物研学-研学",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::PurchasePiecesOfPainting,
     {
         .name       = "博物研学-易市碎片",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::UnlockPaintingAndBuff,
     {
         .name       = "博物研学-图鉴解锁",
         .routine    = false,
         .has_config = false,
         .scheduable = true,
         .difficult  = false,
     }},
    {MajorTask::UnlockResearchNews,
     {
         .name       = "博物研学-见闻解锁",
         .routine    = false,
         .has_config = false,
         .scheduable = true,
         .difficult  = false,
     }},
    {MajorTask::PlayDongguPreliminary,
     {
         .name       = "冬古竞赛-预选赛",
         .routine    = false,
         .has_config = false,
         .scheduable = false,
         .difficult  = true,
     }},
    {MajorTask::PlayDongguCompetition,
     {
         .name       = "冬古竞赛-正赛",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = true,
     }},
    {MajorTask::PurchaseDongguProduct,
     {
         .name       = "冬古竞赛-手信采购",
         .routine    = false,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
    {MajorTask::GetDongguReward,
     {
         .name       = "冬古竞赛-奖励获取",
         .routine    = true,
         .has_config = true,
         .scheduable = false,
         .difficult  = false,
     }},
};

TaskInfo get_task_info(MajorTask task) {
    return TASK_INFO_TABLE.at(task);
}

} // namespace Task

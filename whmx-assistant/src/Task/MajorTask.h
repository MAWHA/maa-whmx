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

#include <QtCore/QString>
#include <QtCore/QMetaType>

namespace Task {

/*!
 * 常规 - 默认选中的任务
 * 配置 - 包含可配置选项的任务
 * 调度 - 不推荐手动执行，由任务调度决定何时运行的任务
 * 高难 - 难以实现，建议自行手动完成的任务
 */
enum class MajorTask {
    DailySignin,                      //<! [常规] 签到
    GetFreeGiftPack,                  //<! [常规][配置] 商亭免费礼包
    ConsumeExpiringVitalityPotion,    //<! [常规][配置] 消费临期体力水
    PurchaseVitality,                 //<! [配置] 购买体力
    PurchaseRandomProduct,            //<! [常规][配置] 易物所采购
    GetRoutineTaskReward,             //<! [常规][调度] 日常/周常任务
    GetJourneyReward,                 //<! [常规][调度] 游历
    ReceiveMail,                      //<! [常规] 邮件
    MopupResourceLevel,               //<! [配置] 演训-基础训练
    EnlistCharacter,                  //<! [配置] 器者征集
    RecruitCharacter,                 //<! [配置] 器者招集
    SpecialActivitySignin,            //<! [常规] 活动签到
    AssignOfficeProduct,              //<! [配置] 派遣公司-办公室制造替换
    AssignWorkshopProduct,            //<! [配置] 派遣公司-工作间制造替换
    FindCat,                          //<! [高难] 派遣公司-找猫猫
    AssignEquipmentOrder,             //<! [配置] 派遣公司-装备订购
    GetOrderedEquipment,              //<! [常规] 派遣公司-装备订购获取
    GetOrderedProduct,                //<! [常规] 派遣公司-资源获取
    GetFavorability,                  //<! [常规] 派遣公司-感应获取
    GetAccumulatedVitality,           //<! [常规] 派遣公司-体力获取
    ReplaceFullFavorabilityCharacter, //<! [常规][配置][调度] 派遣公司-满感应度器者替换
    ChatOverTea,                      //<! [配置] 派遣公司-品茗
    PlayMergeGame,                    //<! 派遣公司-和合
    PlayMiZongPan,                    //<! [高难][配置] 派遣公司-迷踪盘
    PlayFourInRow,                    //<! [配置] 派遣公司-四子棋
    GetFurnitureBlueprint,            //<! [常规][调度] 派遣公司-家具蓝图获取
    SwitchFurnitureLayout,            //<! [配置] 派遣公司-家具布局替换
    GetResearchReward,                //<! [调度] 博物研学-周常奖励
    DoResearch,                       //<! [配置] 博物研学-研学
    PurchasePiecesOfPainting,         //<! [配置] 博物研学-易市碎片
    UnlockPaintingAndBuff,            //<! [调度] 博物研学-图鉴解锁
    UnlockResearchNews,               //<! [调度] 博物研学-见闻解锁
    PlayDongguPreliminary,            //<! [高难] 冬古竞赛-预选赛
    PlayDongguCompetition,            //<! [高难][配置] 冬古竞赛-正赛
    PurchaseDongguProduct,            //<! [配置] 冬古竞赛-手信采购
    GetDongguReward,                  //<! [常规][调度] 冬古竞赛-奖励获取
};

struct TaskInfo {
    QString name;
    bool    routine;
    bool    has_config;
    bool    scheduable;
    bool    difficult;
};

TaskInfo get_task_info(MajorTask task);

} // namespace Task

Q_DECLARE_METATYPE(Task::MajorTask);
Q_DECLARE_METATYPE(Task::TaskInfo);

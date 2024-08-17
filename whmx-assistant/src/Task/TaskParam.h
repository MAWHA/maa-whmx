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

#include "PropGetter.h"

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMetaType>

namespace Task {

//! 道具分页
enum class PropCategory {
    ExpBook,     //<! 办公室-制造-“经验书”分页
    Prop,        //<! 工作间-制造-“道具"分页
    Material,    //<! 工作间-物质交换机-“材料”分页
    Certificate, //<! 工作间-物质交换机-“核定牌”分页
};

//! 产品制造策略
enum class ProductionStrategy {
    AlwaysReassign, //<! 总是停止当前工作立即替换
    ReassignOnDiff, //<! 仅当工作内容不同时替换工作
    AssignOnIdle,   //<! 仅当空闲时分配工作
};

//! 关卡类别
enum class LevelCategory {
    Unknown,    //<! 未知类型
    Develop,    //<! 演训-基础训练-培养
    Assess,     //<! 演训-基础训练-考核
    MainLevel,  //<! 执行-主线关卡
    SuperLevel, //<! 执行-裂隙
    Resource,   //<! 执行-资源关卡
};

struct GetFreeGiftPackParam : public PropGetter {
    enum class Strategy {
        DailyAllowanceOnly, //<! [默认] 仅“冬谷每日补贴”
        FindAll,            //<! 尝试寻找并领取所有免费礼包
    };

    //! 领取策略
    Strategy strategy = Strategy::DailyAllowanceOnly;

    DECLARE_PROPS(GetFreeGiftPackParam, strategy)
};

struct ConsumeExpiringVitalityPotionParam : public PropGetter {
    enum class Strategy {
        LowVitalityPotionOnly,  //<! [默认] 仅“冬谷畅饮水”
        HighVitalityPotionOnly, //<! 仅“冬谷神秘水”
        All,                    //<! 尝试所有体力水，优先消耗“冬谷畅饮水”
    };

    //! 消费策略
    Strategy strategy = Strategy::LowVitalityPotionOnly;
    //! 消费的体力水数量，-1 表示消费至最大值；消费的数量最多不超过体力上限
    int number = 1;
    //! 临期阈值（天），小于等于该值时消费体力水
    int expire_threshold = 2;

    DECLARE_PROPS(ConsumeExpiringVitalityPotionParam, strategy, number, expire_threshold)
};

struct PurchaseVitalityParam : public PropGetter {
    enum class Strategy {
        Fixed,   //<! [默认] 固定数量购买
        Maximum, //<! 购买至最大数量，直到无法继续购买（体力到上限，清仓，灰珀不足）
        Auto,    //<! 购买至最大数量，直到低于设定的灰珀阈值或无法继续购买
    };

    //! 购买策略
    Strategy strategy = Strategy::Fixed;
    //! Fixed 策略下的购买数量
    int number = 1;
    //! Auto 策略下的灰珀阈值（下限）
    int coin_threshold = 500;

    DECLARE_PROPS(PurchaseVitalityParam, strategy, number, coin_threshold)
};

struct PurchaseRandomProductParam : public PropGetter {
    //! TODO: impl PurchaseRandomProduct

    DECLARE_PROPS(PurchaseRandomProductParam)
};

struct MopupResourceLevelParam : public PropGetter {
    //! 关卡类别
    LevelCategory category = LevelCategory::Develop;
    //! 关卡名称
    QString level_name = "冬谷币";
    //! 关卡序号
    int level_index = 5;
    //! 挑战次数
    int repeat_times = 1;
    //! 是否扫荡
    bool mopup = true;

    DECLARE_PROPS(MopupResourceLevelParam, category, level_name, level_index, repeat_times, mopup)
};

struct EnlistCharacterParam : public PropGetter {
    //! TODO: impl EnlistCharacter

    DECLARE_PROPS(EnlistCharacterParam)
};

struct RecruitCharacterParam : public PropGetter {
    //! TODO: impl RecruitCharacter

    DECLARE_PROPS(RecruitCharacterParam)
};

struct AssignOfficeProductParam : public PropGetter {
    //! 产品所在道具分页
    PropCategory category = PropCategory::ExpBook;
    //! 产品名称
    QString product_name = "进阶社会学";
    //! 分配策略
    ProductionStrategy strategy = ProductionStrategy::AlwaysReassign;
    //! 是否自动添加新订单
    bool auto_assign = true;

    DECLARE_PROPS(AssignOfficeProductParam, category, product_name, strategy, auto_assign)
};

struct AssignWorkshopProductParam : public PropGetter {
    //! 产品所在道具分页
    PropCategory category = PropCategory::Prop;
    //! 产品名称
    QString product_name = "錾银石墨条";
    //! 分配策略
    ProductionStrategy strategy = ProductionStrategy::AlwaysReassign;
    //! 是否自动添加新订单
    bool auto_assign = true;

    DECLARE_PROPS(AssignWorkshopProductParam, category, product_name, strategy, auto_assign)
};

struct AssignEquipmentOrderParam : public PropGetter {
    //! TODO: impl AssignEquipmentOrder

    DECLARE_PROPS(AssignEquipmentOrderParam)
};

struct ReplaceFullFavorabilityCharacterParam : public PropGetter {
    //! TODO: impl ReplaceFullFavorabilityCharacter

    DECLARE_PROPS(ReplaceFullFavorabilityCharacterParam)
};

struct ChatOverTeaParam : public PropGetter {
    //! TODO: impl ChatOverTea

    DECLARE_PROPS(ChatOverTeaParam)
};

struct PlayMiZongPanParam : public PropGetter {
    //! TODO: impl PlayMiZongPan

    DECLARE_PROPS(PlayMiZongPanParam)
};

struct PlayFourInRowParam : public PropGetter {
    enum class Role {
        Black,  //<! [默认] 黑棋（先手）
        White,  //<! 白棋（后手）
        Random, //<! 随机
    };

    enum class Mode {
        Normal,    //<! [默认] 普通模式
        Difficult, //<! 困难模式
    };

    //! 棋手角色
    Role role = Role::Black;
    //! 难度模式
    Mode mode = Mode::Normal;
    //! 期望胜局局数
    int rounds = 1;

    DECLARE_PROPS(PlayFourInRowParam, role, mode, rounds)
};

struct GetFurnitureBlueprintParam : public PropGetter {
    //! TODO: impl GetFurnitureBlueprint

    DECLARE_PROPS(GetFurnitureBlueprintParam)
};

struct SwitchFurnitureLayoutParam : public PropGetter {
    //! TODO: impl SwitchFurnitureLayout

    DECLARE_PROPS(SwitchFurnitureLayoutParam)
};

struct DoResearchParam : public PropGetter {
    enum CombatLevel {
        Level60 = 60, //<! [默认] 推荐等级：60
        Level70 = 70, //<! 推荐等级：70
        Level80 = 80, //<! 推荐等级：80
    };

    //! 研学收集方向
    QString category = "雪景寒林图";
    //! 战斗路线
    int combat_level = CombatLevel::Level60;
    //! 偏好增益列表
    QList<QString> buff_preference = QList<QString>();

    DECLARE_PROPS(DoResearchParam, category, combat_level, buff_preference)
};

struct PurchasePiecesOfPaintingParam : public PropGetter {
    //! 购买方案，按给出的画作顺序购买对应碎片
    QList<QString> queue = QList<QString>();

    DECLARE_PROPS(PurchasePiecesOfPaintingParam, queue)
};

struct PlayDongguCompetitionParam : public PropGetter {
    //! TODO: impl PlayDongguCompetition

    DECLARE_PROPS(PlayDongguCompetitionParam)
};

struct PurchaseDongguProductParam : public PropGetter {
    struct Order {
        enum class Strategy {
            One,         //<! [默认] 购买一件
            Fixed,       //<! 购买 number 指定的固定数量
            MaximumOnce, //<! 以单次购买的最大数量购买一次
            Maximum,     //<! 购买最大数量（直到清仓或余额不足）
        };

        QString  product_name = "";            //<! 商品名称
        Strategy strategy     = Strategy::One; //<! 购买策略
        int      number       = -1;            //<! Fixed 策略下的购买数量
    };

    //! 购买方案，按顺序购买商品
    QList<Order> order_plan;

    DECLARE_PROPS(PurchaseDongguProductParam, order_plan)
};

struct GetDongguRewardParam : public PropGetter {
    //! TODO: impl GetDongguReward

    DECLARE_PROPS(GetDongguRewardParam)
};

[[nodiscard]] std::shared_ptr<PropGetter> dump_task_param_to_prop_getter(const QVariant& param);

} // namespace Task

Q_DECLARE_METATYPE(Task::PropCategory);
Q_DECLARE_METATYPE(Task::ProductionStrategy);
Q_DECLARE_METATYPE(Task::LevelCategory);
Q_DECLARE_METATYPE(Task::GetFreeGiftPackParam);
Q_DECLARE_METATYPE(Task::GetFreeGiftPackParam::Strategy);
Q_DECLARE_METATYPE(Task::ConsumeExpiringVitalityPotionParam);
Q_DECLARE_METATYPE(Task::ConsumeExpiringVitalityPotionParam::Strategy);
Q_DECLARE_METATYPE(Task::PurchaseVitalityParam);
Q_DECLARE_METATYPE(Task::PurchaseRandomProductParam);
Q_DECLARE_METATYPE(Task::MopupResourceLevelParam);
Q_DECLARE_METATYPE(Task::EnlistCharacterParam);
Q_DECLARE_METATYPE(Task::RecruitCharacterParam);
Q_DECLARE_METATYPE(Task::AssignOfficeProductParam);
Q_DECLARE_METATYPE(Task::AssignWorkshopProductParam);
Q_DECLARE_METATYPE(Task::AssignEquipmentOrderParam);
Q_DECLARE_METATYPE(Task::ReplaceFullFavorabilityCharacterParam);
Q_DECLARE_METATYPE(Task::ChatOverTeaParam);
Q_DECLARE_METATYPE(Task::PlayMiZongPanParam);
Q_DECLARE_METATYPE(Task::PlayFourInRowParam);
Q_DECLARE_METATYPE(Task::PlayFourInRowParam::Role);
Q_DECLARE_METATYPE(Task::PlayFourInRowParam::Mode);
Q_DECLARE_METATYPE(Task::GetFurnitureBlueprintParam);
Q_DECLARE_METATYPE(Task::SwitchFurnitureLayoutParam);
Q_DECLARE_METATYPE(Task::DoResearchParam);
Q_DECLARE_METATYPE(Task::PurchasePiecesOfPaintingParam);
Q_DECLARE_METATYPE(Task::PlayDongguCompetitionParam);
Q_DECLARE_METATYPE(Task::PurchaseDongguProductParam);
Q_DECLARE_METATYPE(Task::PurchaseDongguProductParam::Order);
Q_DECLARE_METATYPE(Task::PurchaseDongguProductParam::Order::Strategy);
Q_DECLARE_METATYPE(Task::GetDongguRewardParam);

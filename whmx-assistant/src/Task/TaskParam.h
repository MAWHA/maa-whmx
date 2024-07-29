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
#include <QtCore/QList>

namespace Task {

struct GetFreeGiftPackParam {};

struct ConsumeExpiringVitalityPotionParam {};

struct PurchaseVitalityParam {};

struct PurchaseRandomProductParam {};

struct MopupResourceLevelParam {};

struct EnlistCharacterParam {};

struct RecruitCharacterParam {};

struct AssignOfficeProductParam {};

struct AssignWorkshopProductParam {};

struct AssignEquipmentOrderParam {};

struct ReplaceFullFavorabilityCharacterParam {};

struct ChatOverTeaParam {};

struct PlayMiZongPanParam {};

struct PlayFourInARowParam {};

struct SwitchFurnitureLayoutParam {};

struct DoResearchParam {};

struct PurchasePiecesOfPaintingParam {
    //! 购买方案，按给出的画作顺序购买对应碎片
    QList<QString> queue;
};

struct PlayDongguCompetitionParam {};

struct PurchaseDongguProductParam {
    struct Order {
        enum class Strategy {
            One,         //<! [默认] 购买一件
            Fixed,       //<! 购买 number 指定的固定数量
            MaximumOnce, //<! 以单次购买的最大数量购买一次
            Maximum,     //<! 购买最大数量（直到清仓或余额不足）
        };

        QString  product_name = "";
        Strategy strategy     = Strategy::One;
        int      number       = -1;
    };

    //! 购买方案，按顺序购买商品
    QList<Order> order_plan;
};

} // namespace Task

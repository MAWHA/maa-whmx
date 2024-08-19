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

#include "TaskConfigPanel.h"
#include "Helper.h"
#include "Scrollbar.h"
#include "../Task/TaskParam.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <qtmaterialtextfield.h>
#include <ElaComboBox.h>
#include <ElaSpinBox.h>
#include <ElaToggleSwitch.h>
#include <ElaLineEdit.h>

namespace UI {

using namespace Task;

void setup(TaskConfigPanel *panel, GetFreeGiftPackParam &param) {
    using Strategy = GetFreeGiftPackParam::Strategy;

    auto strategy = new ElaComboBox;
    {
        strategy->addItem("仅购买“冬谷每日补贴”", QVariant::fromValue(Strategy::DailyAllowanceOnly));
        strategy->addItem("获取所有免费礼包", QVariant::fromValue(Strategy::FindAll));
        combo_set_current_item(strategy, param.strategy);
    }

    panel->append_config_item("领取策略", "指定礼包的领取策略", strategy);

    QObject::connect(strategy, &ElaComboBox::currentIndexChanged, strategy, [=, &param](int index) {
        param.strategy = strategy->itemData(index).value<Strategy>();
        panel->notify_config_changed();
    });
}

void setup(TaskConfigPanel *panel, ConsumeExpiringVitalityPotionParam &param) {
    using Strategy = ConsumeExpiringVitalityPotionParam::Strategy;

    auto strategy = new ElaComboBox;
    {
        strategy->addItem("仅“冬谷畅饮水”", QVariant::fromValue(Strategy::LowVitalityPotionOnly));
        strategy->addItem("仅“冬谷神秘水”", QVariant::fromValue(Strategy::HighVitalityPotionOnly));
        strategy->addItem("全部类型", QVariant::fromValue(Strategy::All));
        combo_set_current_item(strategy, param.strategy);
    }

    auto number = new ElaSpinBox;
    {
        number->setMinimum(-1);
        number->setMaximum(65536);
        number->setSingleStep(1);
        number->setValue(param.number);
    }

    auto expire_threshold = new ElaSpinBox;
    {
        expire_threshold->setMinimum(1);
        expire_threshold->setMaximum(14);
        expire_threshold->setSingleStep(1);
        expire_threshold->setValue(param.expire_threshold);
    }

    panel->append_config_item("目标药水", "指定需要使用的体力药水", strategy);
    panel->append_config_item("使用数量", "需要使用的药水最大数量\n-1 表示使用至药水耗尽或达到体力上限", number);
    panel->append_config_item("临期天数", "需要使用的药水的天数阈值\n仅当小于等于该值时使用药水", expire_threshold);

    QObject::connect(strategy, &ElaComboBox::currentIndexChanged, strategy, [=, &param](int index) {
        param.strategy = strategy->itemData(index).value<Strategy>();
        panel->notify_config_changed();
    });

    QObject::connect(number, &ElaSpinBox::valueChanged, number, [=, &param](int value) {
        param.number = value;
        panel->notify_config_changed();
    });

    QObject::connect(expire_threshold, &ElaSpinBox::valueChanged, expire_threshold, [=, &param](int value) {
        param.expire_threshold = value;
        panel->notify_config_changed();
    });
}

void setup(TaskConfigPanel *panel, PurchaseVitalityParam &param) {
    using Strategy = PurchaseVitalityParam::Strategy;

    auto strategy = new ElaComboBox;
    {
        strategy->addItem("固定数量", QVariant::fromValue(Strategy::Fixed));
        strategy->addItem("最大值", QVariant::fromValue(Strategy::Maximum));
        strategy->addItem("自动", QVariant::fromValue(Strategy::Auto));
        combo_set_current_item(strategy, param.strategy);
    }

    auto number = new ElaSpinBox;
    {
        number->setMinimum(1);
        number->setMaximum(5);
        number->setSingleStep(1);
        number->setValue(param.number);
    }

    auto coin_threshold = new ElaSpinBox;
    {
        coin_threshold->setMinimum(0);
        coin_threshold->setMaximum(std::numeric_limits<int>::max());
        coin_threshold->setSingleStep(50);
        coin_threshold->setValue(param.coin_threshold);
    }

    panel->append_config_item(
        "购买策略", "指定购买体力的策略\n“自动”策略将根据设定的“灰珀阈值”购买最大次数的体力药水", strategy);
    panel->append_config_item("购买次数", "购买体力的次数\n该配置仅在“固定数量”策略下有效", number);
    panel->append_config_item(
        "灰珀阈值", "购买体力的灰珀阈值，仅当剩余灰珀不小于该值时进行购买\n该配置仅在“自动”策略下有效", coin_threshold);

    QObject::connect(strategy, &ElaComboBox::currentIndexChanged, strategy, [=, &param](int index) {
        param.strategy = strategy->itemData(index).value<Strategy>();
        panel->notify_config_changed();
    });

    QObject::connect(number, &ElaSpinBox::valueChanged, number, [=, &param](int value) {
        param.number = value;
        panel->notify_config_changed();
    });

    QObject::connect(coin_threshold, &ElaSpinBox::valueChanged, coin_threshold, [=, &param](int value) {
        param.coin_threshold = value;
        panel->notify_config_changed();
    });
}

void setup(TaskConfigPanel *panel, PurchaseRandomProductParam &param) {
    Q_UNIMPLEMENTED();
    panel->put_notice("暂未开放此项核心任务的配置面板，敬请期待之后的更新~");
}

void setup(TaskConfigPanel *panel, MopupResourceLevelParam &param) {
    using Category = LevelCategory;

    static QMap<QString, Category> LEVEL_TABLE{
        {"冬古币",     Category::Develop },
        {"教材",        Category::Develop },
        {"装备",        Category::Develop },
        {"宿卫",        Category::Assess  },
        {"构术",        Category::Assess  },
        {"远击",        Category::Assess  },
        {"轻锐",        Category::Assess  },
        {"战略",        Category::Assess  },
        {"来古-资源", Category::Resource},
        {"汴都-资源", Category::Resource},
        {"申海-资源", Category::Resource},
        {"粤州-资源", Category::Resource},
        {"常安-资源", Category::Resource},
    };

    auto level_name = new ElaComboBox;
    {
        level_name->addItems(LEVEL_TABLE.keys());
        level_name->setCurrentText(param.level_name);
    }

    auto level_index = new ElaSpinBox;
    {
        level_index->setMinimum(1);
        level_index->setMaximum(5);
        level_index->setSingleStep(1);
        level_index->setValue(param.level_index);
    }

    auto repeat_times = new ElaSpinBox;
    {
        repeat_times->setMinimum(1);
        repeat_times->setMaximum(10);
        repeat_times->setSingleStep(1);
        repeat_times->setValue(param.repeat_times);
    }

    auto mopup = new ElaToggleSwitch;
    { mopup->setIsToggled(param.mopup); }

    panel->append_config_item("关卡名称", "指定目标关卡名称/系列", level_name);
    panel->append_config_item("关卡序号", "指定需要挑战的关卡在目标系列中的序号", level_index);
    panel->append_config_item("挑战次数", "指定挑战次数\n体力不足时将自动截断", repeat_times);
    panel->append_config_item("扫荡", "指定是否扫荡\n未解锁速通时可使用非扫荡模式", mopup);

    QObject::connect(level_name, &ElaComboBox::currentIndexChanged, level_name, [=, &param](int index) {
        param.level_name = level_name->itemText(index);
        param.category   = LEVEL_TABLE.value(param.level_name, Category::Unknown);
        panel->notify_config_changed();
    });

    QObject::connect(level_index, &ElaSpinBox::valueChanged, level_index, [=, &param](int value) {
        param.level_index = value;
        panel->notify_config_changed();
    });

    QObject::connect(repeat_times, &ElaSpinBox::valueChanged, repeat_times, [=, &param](int value) {
        param.repeat_times = value;
        panel->notify_config_changed();
    });

    QObject::connect(mopup, &ElaToggleSwitch::toggled, mopup, [=, &param](bool checked) {
        param.mopup = checked;
        panel->notify_config_changed();
    });
}

void setup(TaskConfigPanel *panel, EnlistCharacterParam &param) {
    Q_UNIMPLEMENTED();
    panel->put_notice("暂未开放此项核心任务的配置面板，敬请期待之后的更新~");
}

void setup(TaskConfigPanel *panel, RecruitCharacterParam &param) {
    Q_UNIMPLEMENTED();
    panel->put_notice("暂未开放此项核心任务的配置面板，敬请期待之后的更新~");
}

void setup(TaskConfigPanel *panel, AssignOfficeProductParam &param) {
    using Strategy = ProductionStrategy;

    auto product_name = new ElaComboBox;
    {
        product_name->addItems({
            "初级社会指南",
            "中级社会指南",
            "高级社会指南",
            "进阶社会学",
        });
        product_name->setCurrentText(param.product_name);
    }

    auto strategy = new ElaComboBox;
    {
        strategy->addItem("立即重制", QVariant::fromValue(Strategy::AlwaysReassign));
        strategy->addItem("仅不同产品", QVariant::fromValue(Strategy::ReassignOnDiff));
        strategy->addItem("仅空闲", QVariant::fromValue(Strategy::AssignOnIdle));
        combo_set_current_item(strategy, param.strategy);
    }

    auto auto_assign = new ElaToggleSwitch;
    { auto_assign->setIsToggled(param.auto_assign); }

    panel->append_config_item("目标产品", "指定需要制造的产品\n默认制造最大值", product_name);
    panel->append_config_item("制造策略", "指定分配制造任务的策略", strategy);
    panel->append_config_item("自动添加新订单", "指定是否自动添加新订单", auto_assign);

    QObject::connect(product_name, &ElaComboBox::currentIndexChanged, product_name, [=, &param](int index) {
        param.product_name = product_name->itemText(index);
        panel->notify_config_changed();
    });

    QObject::connect(strategy, &ElaComboBox::currentIndexChanged, strategy, [=, &param](int index) {
        param.strategy = strategy->itemData(index).value<Strategy>();
        panel->notify_config_changed();
    });

    QObject::connect(auto_assign, &ElaToggleSwitch::toggled, auto_assign, [=, &param](int checked) {
        param.auto_assign = checked;
        panel->notify_config_changed();
    });
}

void setup(TaskConfigPanel *panel, AssignWorkshopProductParam &param) {
    using Strategy = ProductionStrategy;

    auto product_name = new ElaComboBox;
    {
        product_name->addItems({
            "随机凌级装备",
            "饰铜石墨条",
            "錾银石墨条",
            "灰珀碎片涂料",
            "I型拟态钢材",
            "II型拟态钢材",
            "资质证明I",
            "资质证明II",
            "资质证明III",
            "高级制造申请",
        });
        product_name->setCurrentText(param.product_name);
    }

    auto strategy = new ElaComboBox;
    {
        strategy->addItem("立即重制", QVariant::fromValue(Strategy::AlwaysReassign));
        strategy->addItem("仅不同产品", QVariant::fromValue(Strategy::ReassignOnDiff));
        strategy->addItem("仅空闲", QVariant::fromValue(Strategy::AssignOnIdle));
        combo_set_current_item(strategy, param.strategy);
    }

    auto auto_assign = new ElaToggleSwitch;
    { auto_assign->setIsToggled(param.auto_assign); }

    panel->append_config_item("目标产品", "指定需要制造的产品\n默认制造最大值", product_name);
    panel->append_config_item("制造策略", "指定分配制造任务的策略", strategy);
    panel->append_config_item("自动添加新订单", "指定是否自动添加新订单", auto_assign);

    QObject::connect(product_name, &ElaComboBox::currentIndexChanged, product_name, [=, &param](int index) {
        param.product_name = product_name->itemText(index);
        panel->notify_config_changed();
    });

    QObject::connect(strategy, &ElaComboBox::currentIndexChanged, strategy, [=, &param](int index) {
        param.strategy = strategy->itemData(index).value<Strategy>();
        panel->notify_config_changed();
    });

    QObject::connect(auto_assign, &ElaToggleSwitch::toggled, auto_assign, [=, &param](int checked) {
        param.auto_assign = checked;
        panel->notify_config_changed();
    });
}

void setup(TaskConfigPanel *panel, AssignEquipmentOrderParam &param) {
    Q_UNIMPLEMENTED();
    panel->put_notice("暂未开放此项核心任务的配置面板，敬请期待之后的更新~");
}

void setup(TaskConfigPanel *panel, ReplaceFullFavorabilityCharacterParam &param) {
    Q_UNIMPLEMENTED();
    panel->put_notice("暂未开放此项核心任务的配置面板，敬请期待之后的更新~");
}

void setup(TaskConfigPanel *panel, ChatOverTeaParam &param) {
    Q_UNIMPLEMENTED();
    panel->put_notice("暂未开放此项核心任务的配置面板，敬请期待之后的更新~");
}

void setup(TaskConfigPanel *panel, PlayMiZongPanParam &param) {
    Q_UNIMPLEMENTED();
    panel->put_notice("暂未开放此项核心任务的配置面板，敬请期待之后的更新~");
}

void setup(TaskConfigPanel *panel, PlayFourInRowParam &param) {
    using Role = PlayFourInRowParam::Role;
    using Mode = PlayFourInRowParam::Mode;

    auto role = new ElaComboBox;
    {
        role->addItem("黑棋（先手）", QVariant::fromValue(Role::Black));
        role->addItem("白棋（后手）", QVariant::fromValue(Role::White));
        role->addItem("随机", QVariant::fromValue(Role::Random));
        combo_set_current_item(role, param.role);
    }

    auto mode = new ElaComboBox;
    {
        mode->addItem("普通", QVariant::fromValue(Mode::Normal));
        mode->addItem("困难", QVariant::fromValue(Mode::Difficult));
        combo_set_current_item(mode, param.mode);
    }

    auto rounds = new ElaSpinBox;
    {
        rounds->setMinimum(1);
        rounds->setMaximum(std::numeric_limits<int>::max());
        rounds->setSingleStep(1);
        rounds->setValue(param.rounds);
    }

    panel->append_config_item("棋手角色", "指定玩家先后手", role);
    panel->append_config_item("模式", "指定棋局难度", mode);
    panel->append_config_item("期望胜场", "指定需要取得的胜局局数", rounds);

    QObject::connect(role, &ElaComboBox::currentIndexChanged, role, [=, &param](int index) {
        param.role = role->itemData(index).value<Role>();
        panel->notify_config_changed();
    });

    QObject::connect(mode, &ElaComboBox::currentIndexChanged, mode, [=, &param](int index) {
        param.mode = mode->itemData(index).value<Mode>();
        panel->notify_config_changed();
    });

    QObject::connect(rounds, &ElaSpinBox::valueChanged, rounds, [=, &param](int value) {
        param.rounds = value;
        panel->notify_config_changed();
    });
}

void setup(TaskConfigPanel *panel, GetFurnitureBlueprintParam &param) {
    Q_UNIMPLEMENTED();
    panel->put_notice("暂未开放此项核心任务的配置面板，敬请期待之后的更新~");
}

void setup(TaskConfigPanel *panel, SwitchFurnitureLayoutParam &param) {
    Q_UNIMPLEMENTED();
    panel->put_notice("暂未开放此项核心任务的配置面板，敬请期待之后的更新~");
}

void setup(TaskConfigPanel *panel, DoResearchParam &param) {
    auto category = new ElaComboBox;
    {
        category->addItems({
            "雪景寒林图",
            "千里江山图",
            "溪山行旅图",
        });
        category->setCurrentText(param.category);
    }

    auto combat_level = new ElaComboBox;
    {
        combat_level->addItem("推荐等级：60", DoResearchParam::Level60);
        combat_level->addItem("推荐等级：70", DoResearchParam::Level70);
        combat_level->addItem("推荐等级：80", DoResearchParam::Level80);
        combo_set_current_item(combat_level, param.combat_level);
    }

    auto buff_preference = new ElaLineEdit;
    {
        buff_preference->setFixedHeight(35);
        buff_preference->setFont(panel->font());
        buff_preference->setPlaceholderText("请按优先级输入偏好的增益列表");
        buff_preference->setText(param.buff_preference.join(";"));
        buff_preference->setMaximumWidth(640);
    }

    panel->append_config_item("研学方向", "指定研学的收集方向", category);
    panel->append_config_item("战斗路线", "指定研学战斗的关卡推荐等级", combat_level);
    panel->append_config_item(
        "增益偏好", "偏好的增益列表，按分号划分\n选择增益时按给定顺序优先选取对应增幅，不存在时则随机选取", buff_preference);

    QObject::connect(category, &ElaComboBox::currentIndexChanged, category, [=, &param](int index) {
        param.category = category->itemText(index);
        panel->notify_config_changed();
    });

    QObject::connect(combat_level, &ElaComboBox::currentIndexChanged, combat_level, [=, &param](int index) {
        param.combat_level = combat_level->itemData(index).toInt();
        panel->notify_config_changed();
    });

    QObject::connect(buff_preference, &ElaLineEdit::textChanged, buff_preference, [=, &param](const QString &text) {
        QStringList buff_preference;
        for (const auto &buff : QString(text).replace(QChar(U'；'), ';').split(';')) {
            const auto buff_name = buff.trimmed();
            if (buff_name.isEmpty()) { continue; }
            buff_preference.append(buff_name);
        }
        param.buff_preference = buff_preference;
        panel->notify_config_changed();
    });
}

void setup(TaskConfigPanel *panel, PurchasePiecesOfPaintingParam &param) {
    Q_UNIMPLEMENTED();
    panel->put_notice("暂未开放此项核心任务的配置面板，敬请期待之后的更新~");
}

void setup(TaskConfigPanel *panel, PlayDongguCompetitionParam &param) {
    Q_UNIMPLEMENTED();
    panel->put_notice("暂未开放此项核心任务的配置面板，敬请期待之后的更新~");
}

void setup(TaskConfigPanel *panel, PurchaseDongguProductParam &param) {
    Q_UNIMPLEMENTED();
    panel->put_notice("暂未开放此项核心任务的配置面板，敬请期待之后的更新~");
}

void setup(TaskConfigPanel *panel, GetDongguRewardParam &param) {
    Q_UNIMPLEMENTED();
    panel->put_notice("暂未开放此项核心任务的配置面板，敬请期待之后的更新~");
}

template <MajorTask T>
static void setup(TaskConfigPanel *panel) {
    if constexpr (false) {
    } else if constexpr (T == MajorTask::GetFreeGiftPack) {
        setup(panel, std::ref(panel->config<GetFreeGiftPackParam>()));
    } else if constexpr (T == MajorTask::ConsumeExpiringVitalityPotion) {
        setup(panel, std::ref(panel->config<ConsumeExpiringVitalityPotionParam>()));
    } else if constexpr (T == MajorTask::PurchaseVitality) {
        setup(panel, std::ref(panel->config<PurchaseVitalityParam>()));
    } else if constexpr (T == MajorTask::PurchaseRandomProduct) {
        setup(panel, std::ref(panel->config<PurchaseRandomProductParam>()));
    } else if constexpr (T == MajorTask::MopupResourceLevel) {
        setup(panel, std::ref(panel->config<MopupResourceLevelParam>()));
    } else if constexpr (T == MajorTask::EnlistCharacter) {
        setup(panel, std::ref(panel->config<EnlistCharacterParam>()));
    } else if constexpr (T == MajorTask::RecruitCharacter) {
        setup(panel, std::ref(panel->config<RecruitCharacterParam>()));
    } else if constexpr (T == MajorTask::AssignOfficeProduct) {
        setup(panel, std::ref(panel->config<AssignOfficeProductParam>()));
    } else if constexpr (T == MajorTask::AssignWorkshopProduct) {
        setup(panel, std::ref(panel->config<AssignWorkshopProductParam>()));
    } else if constexpr (T == MajorTask::AssignEquipmentOrder) {
        setup(panel, std::ref(panel->config<AssignEquipmentOrderParam>()));
    } else if constexpr (T == MajorTask::ReplaceFullFavorabilityCharacter) {
        setup(panel, std::ref(panel->config<ReplaceFullFavorabilityCharacterParam>()));
    } else if constexpr (T == MajorTask::ChatOverTea) {
        setup(panel, std::ref(panel->config<ChatOverTeaParam>()));
    } else if constexpr (T == MajorTask::PlayMiZongPan) {
        setup(panel, std::ref(panel->config<PlayMiZongPanParam>()));
    } else if constexpr (T == MajorTask::PlayFourInRow) {
        setup(panel, std::ref(panel->config<PlayFourInRowParam>()));
    } else if constexpr (T == MajorTask::GetFurnitureBlueprint) {
        setup(panel, std::ref(panel->config<GetFurnitureBlueprintParam>()));
    } else if constexpr (T == MajorTask::SwitchFurnitureLayout) {
        setup(panel, std::ref(panel->config<SwitchFurnitureLayoutParam>()));
    } else if constexpr (T == MajorTask::DoResearch) {
        setup(panel, std::ref(panel->config<DoResearchParam>()));
    } else if constexpr (T == MajorTask::PurchasePiecesOfPainting) {
        setup(panel, std::ref(panel->config<PurchasePiecesOfPaintingParam>()));
    } else if constexpr (T == MajorTask::PlayDongguCompetition) {
        setup(panel, std::ref(panel->config<PlayDongguCompetitionParam>()));
    } else if constexpr (T == MajorTask::PurchaseDongguProduct) {
        setup(panel, std::ref(panel->config<PurchaseDongguProductParam>()));
    } else if constexpr (T == MajorTask::GetDongguReward) {
        setup(panel, std::ref(panel->config<GetDongguRewardParam>()));
    } else {
        std::unreachable();
    }
}

TaskConfigPanel *TaskConfigPanel::build(Task::MajorTask task, QVariant config) {
    auto panel = new TaskConfigPanel(task, config);
    switch (task) {
        case MajorTask::GetFreeGiftPack: {
            UI::setup<MajorTask::GetFreeGiftPack>(panel);
        } break;
        case MajorTask::ConsumeExpiringVitalityPotion: {
            UI::setup<MajorTask::ConsumeExpiringVitalityPotion>(panel);
        } break;
        case MajorTask::PurchaseVitality: {
            UI::setup<MajorTask::PurchaseVitality>(panel);
        } break;
        case MajorTask::PurchaseRandomProduct: {
            UI::setup<MajorTask::PurchaseRandomProduct>(panel);
        } break;
        case MajorTask::MopupResourceLevel: {
            UI::setup<MajorTask::MopupResourceLevel>(panel);
        } break;
        case MajorTask::EnlistCharacter: {
            UI::setup<MajorTask::EnlistCharacter>(panel);
        } break;
        case MajorTask::RecruitCharacter: {
            UI::setup<MajorTask::RecruitCharacter>(panel);
        } break;
        case MajorTask::AssignOfficeProduct: {
            UI::setup<MajorTask::AssignOfficeProduct>(panel);
        } break;
        case MajorTask::AssignWorkshopProduct: {
            UI::setup<MajorTask::AssignWorkshopProduct>(panel);
        } break;
        case MajorTask::AssignEquipmentOrder: {
            UI::setup<MajorTask::AssignEquipmentOrder>(panel);
        } break;
        case MajorTask::ReplaceFullFavorabilityCharacter: {
            UI::setup<MajorTask::ReplaceFullFavorabilityCharacter>(panel);
        } break;
        case MajorTask::ChatOverTea: {
            UI::setup<MajorTask::ChatOverTea>(panel);
        } break;
        case MajorTask::PlayMiZongPan: {
            UI::setup<MajorTask::PlayMiZongPan>(panel);
        } break;
        case MajorTask::PlayFourInRow: {
            UI::setup<MajorTask::PlayFourInRow>(panel);
        } break;
        case MajorTask::GetFurnitureBlueprint: {
            UI::setup<MajorTask::GetFurnitureBlueprint>(panel);
        } break;
        case MajorTask::SwitchFurnitureLayout: {
            UI::setup<MajorTask::SwitchFurnitureLayout>(panel);
        } break;
        case MajorTask::DoResearch: {
            UI::setup<MajorTask::DoResearch>(panel);
        } break;
        case MajorTask::PurchasePiecesOfPainting: {
            UI::setup<MajorTask::PurchasePiecesOfPainting>(panel);
        } break;
        case MajorTask::PlayDongguCompetition: {
            UI::setup<MajorTask::PlayDongguCompetition>(panel);
        } break;
        case MajorTask::PurchaseDongguProduct: {
            UI::setup<MajorTask::PurchaseDongguProduct>(panel);
        } break;
        case MajorTask::GetDongguReward: {
            UI::setup<MajorTask::GetDongguReward>(panel);
        } break;
        default: {
            std::unreachable();
        } break;
    }
    return panel;
}

void TaskConfigPanel::append_config_item(const QString &name, const QString &desc, QWidget *widget) {
    auto container      = new QWidget;
    auto desc_container = new QWidget;
    {
        auto layout = new QVBoxLayout(desc_container);
        layout->setContentsMargins({});
        layout->setSpacing(8);
        auto w = new QLabel(name);
        {
            auto font = this->font();
            font.setPointSize(13);
            w->setFont(font);
        }
        w->setAlignment(Qt::AlignLeft);
        {
            auto pal = w->palette();
            pal.setColor(QPalette::Window, Qt::transparent);
            pal.setColor(QPalette::WindowText, QColor(15, 15, 15));
            w->setPalette(pal);
        }
        auto label = new QLabel(desc);
        label->setWordWrap(true);
        {
            auto font = this->font();
            font.setPointSize(10);
            label->setFont(font);
        }
        label->setAlignment(Qt::AlignLeft);
        {
            auto pal = label->palette();
            pal.setColor(QPalette::Window, Qt::transparent);
            pal.setColor(QPalette::WindowText, QColor(130, 130, 130));
            label->setPalette(pal);
        }
        layout->addWidget(w);
        layout->addWidget(label);
    }
    auto layout = new QHBoxLayout(container);
    layout->setContentsMargins({});
    layout->setSpacing(16);
    layout->addWidget(desc_container);
    layout->addWidget(widget);
    {
        // if (ui_layout_->count() > 2) {
        //     auto sep = new QFrame;
        //     sep->setFrameShape(QFrame::HLine);
        //     {
        //         auto pal = sep->palette();
        //         pal.setColor(QPalette::WindowText, QColor(230, 230, 230));
        //         sep->setPalette(pal);
        //     }
        //     ui_layout_->insertWidget(ui_layout_->count() - 1, sep);
        // }
        ui_layout_->insertWidget(ui_layout_->count() - 1, container);
    }
}

void TaskConfigPanel::put_notice(const QString &notice) {
    auto layout = qobject_cast<QVBoxLayout *>(this->layout());
    while (!layout->isEmpty()) {
        auto item = layout->takeAt(0);
        delete item;
    }
    auto label = new QLabel(notice);
    label->setAlignment(Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(label);
    layout->addStretch();
}

void TaskConfigPanel::notify_config_changed() {
    emit on_config_change(task_, config_data_);
}

TaskConfigPanel::TaskConfigPanel(Task::MajorTask task, QVariant config)
    : task_(task)
    , config_data_(config) {
    setup();
}

void TaskConfigPanel::setup() {
    auto container = new QWidget;
    ui_layout_     = new QVBoxLayout(container);
    // ui_layout_->setContentsMargins(54, 0, 54, 0);
    ui_layout_->setSpacing(12);
    ui_layout_->addSpacing(24);
    ui_layout_->addStretch();

    auto scroll_area = new QScrollArea;
    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    scroll_area->setVerticalScrollBar(new Scrollbar);
    scroll_area->setFrameShape(QFrame::NoFrame);
    scroll_area->setWidgetResizable(true);
    scroll_area->setWidget(container);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins({});
    layout->addWidget(scroll_area);
}

} // namespace UI

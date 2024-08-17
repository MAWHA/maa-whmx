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

#include "DeviceConn.h"
#include "Helper.h"
#include "Notification.h"
#include "../DeviceHelper.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QFileDialog>
#include <ElaContentDialog.h>
#include <filesystem>
#include <qtmaterialdialog.h>

namespace fs = std::filesystem;

using namespace maa;

namespace UI {

int32_t DeviceConn::default_device_type() {
    AdbType device_type{0};
    device_type.set_key(MaaAdbControllerType_Key_AutoDetect);
    device_type.set_touch(MaaAdbControllerType_Touch_AutoDetect);
    device_type.set_screencap(MaaAdbControllerType_Screencap_FastestLosslessWay);
    return device_type;
}

QString DeviceConn::encode_device_type(int32_t type) {
    const AdbType device_type{type};

    QString touch_type;
    QString key_type;
    QString screencap_type;

    for (const auto &[type, value] : AdbType::touch_index) {
        if (value == device_type.touch()) {
            touch_type = QString::fromLatin1(type);
            break;
        }
    }

    for (const auto &[type, value] : AdbType::key_index) {
        if (value == device_type.key()) {
            key_type = QString::fromLatin1(type);
            break;
        }
    }

    for (const auto &[type, value] : AdbType::screencap_index) {
        if (value == device_type.screencap()) {
            screencap_type = QString::fromLatin1(type);
            break;
        }
    }

    return QString(R"(0x%1 key=(0x%2, "%3") touch=(0x%4, "%5") screencap=(0x%6, "%7"))")
        .arg(QString::number(type, 16))
        .arg(QString::number(device_type.touch(), 16))
        .arg(touch_type)
        .arg(QString::number(device_type.key(), 16))
        .arg(key_type)
        .arg(QString::number(device_type.screencap(), 16))
        .arg(screencap_type);
}

void DeviceConn::request_list_devices() {
    if (wait_connect_response_) {
        Notification::warning(this, "检测设备", "当前有未完成的连接请求，请等待请求结束后再试");
        return;
    }
    if (fut_list_devices_.state_->task_.has_value() && !fut_list_devices_.fulfilled()) { return; }
    set_waiting_for_list_devices(true);
    fut_list_devices_ = coro::EventLoop::current()->eval([this] {
        emit on_list_devices_done(list_adb_devices().sync_wait());
        QMetaObject::invokeMethod(this, "set_waiting_for_list_devices", Qt::QueuedConnection, Q_ARG(bool, false));
    });
}

void DeviceConn::add_device(const MaaAdbDevice &device) {
    int  device_index = -1;
    bool duplicate    = false;
    for (int i = 0; i < ui_devices_->count(); ++i) {
        const auto dev = ui_devices_->itemData(i).value<MaaAdbDeviceInfo>().device;
        if (dev.path != device.path) { continue; }
        if (device_index == -1) { device_index = i; }
        if (dev.name == device.name && dev.auto_detect == device.auto_detect) { duplicate = true; }
        if (device_index != -1 && duplicate) { break; }
    }

    if (device_index != -1) {
        auto dev = ui_devices_->itemData(device_index).value<MaaAdbDeviceInfo>().device;
        if (device.address == dev.address && device.config == dev.config) { return; }
    }

    MaaAdbDeviceInfo device_info;
    device_info.device        = device;
    device_info.connect_state = AdbDeviceConnectState_Available;

    const auto type        = device.auto_detect ? "自动" : "手动";
    const auto device_name = !duplicate ? device.name : QString("%1 (%2)").arg(device.name).arg(device.path);

    ui_devices_->addItem(QString("[%1] %2").arg(type).arg(device_name), QVariant::fromValue(device_info));

    update_device_list_state();
}

void DeviceConn::handle_on_list_devices_done(QList<maa::AdbDevice> devices) {
    qInfo() << "list adb devices done: found" << devices.size() << "devices";

    QList<int> detected_devices;
    for (int i = 0; i < ui_devices_->count(); ++i) {
        const auto device_info = ui_devices_->itemData(i).value<MaaAdbDeviceInfo>();
        if (i == ui_devices_->currentIndex() && wait_connect_response_) { continue; }
        if (device_info.connect_state == AdbDeviceConnectState_Available && device_info.device.auto_detect) {
            detected_devices.prepend(i);
        }
    }
    const bool should_reset_selected_device = detected_devices.contains(ui_devices_->currentIndex());
    for (const int index : detected_devices) { ui_devices_->removeItem(index); }

    for (const auto &device : devices) {
        MaaAdbDevice adb_device{
            .name        = QString::fromUtf8(device.name),
            .path        = QString::fromUtf8(device.adb_path),
            .address     = QString::fromUtf8(device.address),
            .config      = QString::fromUtf8(device.config),
            .type        = device.type,
            .auto_detect = true,
        };
        add_device(adb_device);
    }

    if (should_reset_selected_device) {
        ui_devices_->setCurrentIndex(-1);
        ui_device_status_container_->setVisible(false);
    }
}

void DeviceConn::set_waiting_for_list_devices(bool on) {
    if (wait_progress_timer_.isActive() == on) { return; }
    if (on) {
        wait_progress_timer_.setInterval(500);
        wait_progress_timer_.setSingleShot(false);
        wait_progress_timer_.start();
        wait_progress_status_ = 0;
    } else {
        wait_progress_timer_.stop();
        wait_progress_status_ = -1;
    }
    handle_on_update_wait_progress();
}

void DeviceConn::post_connect_request() {
    if (wait_connect_response_) {
        Notification::warning(this, "连接设备", "当前有未完成的连接请求，请稍后再试");
        return;
    }
    if (ui_devices_->currentIndex() == -1) {
        Notification::warning(this, "连接设备", "请选择设备");
        return;
    }
    auto device_info = ui_devices_->currentData().value<MaaAdbDeviceInfo>();
    if (device_info.connect_state == AdbDeviceConnectState_Connected) {
        Notification::warning(this, "连接设备", "设备已经连接");
        return;
    }

    wait_connect_response_ = true;
    ui_devices_->setEnabled(false);

    device_info.connect_state = AdbDeviceConnectState_Pending;
    ui_devices_->setItemData(ui_devices_->currentIndex(), QVariant::fromValue(device_info));
    update_device_status();

    qInfo().noquote() << "request to connect device:" << device_info.device.name;
    emit on_request_connect_device(device_info.device);
}

void DeviceConn::update_device_status() {
    const int index = ui_devices_->currentIndex();
    if (index == -1) { ui_device_status_container_->setVisible(false); }

    const auto device_info = ui_devices_->itemData(index).value<MaaAdbDeviceInfo>();
    const auto device      = device_info.device;

    //! TODO: update device connection status

    ui_device_status_name_->setText(device.name.isEmpty() ? "未命名" : device.name);
    ui_device_status_path_->setText(device.path);
    ui_device_status_type_->setText(encode_device_type(device.type));
    ui_device_status_address_->setText(device.address.isEmpty() ? "未知" : device.address);
    ui_device_status_config_->setText(device.config.isEmpty() ? "{}" : device.config);
    ui_device_status_valid_->setText(fs::exists(device.path.toLocal8Bit().toStdString()) ? "有效" : "无效");

    switch (device_info.connect_state) {
        case AdbDeviceConnectState_Available: {
            ui_device_status_connected_->setText("空闲");
        } break;
        case AdbDeviceConnectState_Pending: {
            ui_device_status_connected_->setText("请求中");
        } break;
        case AdbDeviceConnectState_Connected: {
            ui_device_status_connected_->setText("已连接");
        } break;
        default: {
            std::unreachable();
        } break;
    }

    ui_device_status_container_->setVisible(true);
}

void DeviceConn::update_device_list_state() {
    if (wait_progress_status_ == -1) {
        if (const int n = ui_devices_->count(); n == 0) {
            ui_device_list_state_->setText("未找到可用设备");
        } else {
            ui_device_list_state_->setText(QString("当前共 %1 个设备").arg(n));
        }
    } else {
        ui_device_list_state_->setText(QString("请求设备中%1").arg(QString().fill('.', wait_progress_status_)));
        wait_progress_status_ = (wait_progress_status_ + 1) % 4;
    }
}

void DeviceConn::handle_on_update_wait_progress() {
    update_device_list_state();
}

void DeviceConn::handle_on_add_local_device() {
    const auto device_path = ui_device_path_->text().trimmed();
    const auto path        = device_path.toLocal8Bit().toStdString();

    if (path.empty()) {
        qWarning() << "failed to add local device: empty path";
        Notification::warning(this, "添加设备", "adb 路径为空");
        return;
    }
    if (fs::is_directory(path)) {
        qWarning() << "failed to add local device: specified path is a directory";
        Notification::warning(this, "添加设备", "目标路径不合法");
        return;
    }
    if (!fs::exists(path)) {
        qWarning() << "failed to add local device: specified file not exists";
        const int option =
            ElaContentDialog::showMessageBox(this, "添加设备", "目标文件不存在，是否确认添加？", ElaContentDialog::Confirm);
        if (option != ElaContentDialog::Yes) { return; }
    }

    const auto opt_device_name = ui_device_name_->text().trimmed();
    const auto device_name     = opt_device_name.isEmpty() ? device_path : opt_device_name;
    const auto device_address  = ui_device_address_->text().trimmed();

    MaaAdbDevice adb_device{
        .name        = device_name,
        .path        = device_path,
        .address     = device_address,
        .config      = "{}",
        .type        = default_device_type(),
        .auto_detect = false,
    };
    add_device(adb_device);

    ui_device_path_->clear();
    ui_device_address_->clear();
    ui_device_name_->clear();
}

void DeviceConn::handle_on_select_local_device() {
    const auto adb_path = QFileDialog::getOpenFileName(this, "选择本地 adb 设备", "", "adb 设备 (*.exe)");
    if (adb_path.isEmpty()) { return; }
    ui_device_path_->setText(adb_path);
}

void DeviceConn::handle_on_selected_device_changed() {
    update_device_status();
}

void DeviceConn::handle_on_request_connect_device_done(int maa_status) {
    Q_ASSERT(wait_connect_response_);
    auto device_info = ui_devices_->currentData().value<MaaAdbDeviceInfo>();
    if (maa_status != MaaStatus_Success) {
        qCritical().noquote() << "failed to connect device:" << device_info.device.name;
        device_info.connect_state = AdbDeviceConnectState_Available;
    } else {
        qInfo().noquote() << "successfully connected to device:" << device_info.device.name;
        device_info.connect_state = AdbDeviceConnectState_Connected;
    }
    ui_devices_->setItemData(ui_devices_->currentIndex(), QVariant::fromValue(device_info));
    update_device_status();
    wait_connect_response_ = false;
    ui_devices_->setEnabled(true);
}

DeviceConn::DeviceConn(QWidget *parent)
    : QWidget(parent)
    , wait_progress_timer_(this)
    , wait_progress_status_(-1)
    , wait_connect_response_(false) {
    setup();
    request_list_devices();
    handle_on_update_wait_progress();
}

DeviceConn::~DeviceConn() {
    wait_progress_timer_.stop();
}

void DeviceConn::setup() {
    ui_detect_devices_          = new FlatButton;
    ui_connect_                 = new FlatButton;
    ui_devices_                 = new ElaComboBox;
    ui_device_list_state_       = new QLabel;
    ui_add_device_              = new FlatButton;
    ui_select_device_           = new FlatButton;
    ui_device_path_             = new QtMaterialTextField;
    ui_device_address_label_    = new QLabel;
    ui_device_address_          = new QtMaterialTextField;
    ui_add_device_label_        = new QLabel;
    ui_device_name_             = new QtMaterialTextField;
    ui_device_status_name_      = new QLabel;
    ui_device_status_path_      = new QLabel;
    ui_device_status_type_      = new QLabel;
    ui_device_status_address_   = new QLabel;
    ui_device_status_config_    = new QLabel;
    ui_device_status_valid_     = new QLabel;
    ui_device_status_connected_ = new QLabel;

    config_flat_button(ui_detect_devices_, "检测设备");
    config_flat_button(ui_connect_, "连接");
    config_flat_button(ui_add_device_, "添加设备");
    config_flat_button(ui_select_device_, "选择设备");
    ui_devices_->setPlaceholderText("<请选择需要连接的 adb 设备>");

    auto device_list_container   = new QWidget;
    auto add_device_container    = new QWidget;
    auto device_status_container = new QWidget;

    ui_device_status_container_ = device_status_container;
    ui_device_status_container_->setVisible(false);

    ui_detect_devices_->setFixedWidth(80);
    ui_connect_->setFixedWidth(80);
    ui_add_device_->setFixedWidth(80);
    ui_select_device_->setFixedWidth(80);
    ui_device_list_state_->setFixedWidth(120);
    ui_device_list_state_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui_devices_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    ui_device_path_->setFont(font());
    ui_device_path_->setPlaceholderText("请输入需要添加的本地 adb 设备路径");
    ui_device_path_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    ui_device_address_label_->setText("设备地址：");
    ui_device_address_->setFont(font());
    ui_device_address_->setPlaceholderText("请输入设备连接地址");
    ui_device_address_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    ui_add_device_label_->setText("添加为：");
    ui_device_name_->setFont(font());
    ui_device_name_->setPlaceholderText("请输入设备别名（默认为设备路径）");
    ui_device_name_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    //! FIXME: word wrap for device status labels

    auto device_status_area = new QWidget;
    {
        auto status_table = new QWidget;
        {
            auto layout = new QGridLayout(status_table);
            layout->addWidget(new QLabel("设备名："), 0, 0);
            layout->addWidget(ui_device_status_name_, 0, 1);
            layout->addWidget(new QLabel("设备路径："), 1, 0);
            layout->addWidget(ui_device_status_path_, 1, 1);
            layout->addWidget(new QLabel("类型编码："), 2, 0);
            layout->addWidget(ui_device_status_type_, 2, 1);
            layout->addWidget(new QLabel("连接地址："), 3, 0);
            layout->addWidget(ui_device_status_address_, 3, 1);
            layout->addWidget(new QLabel("配置项："), 4, 0);
            layout->addWidget(ui_device_status_config_, 4, 1);
            layout->addWidget(new QLabel("设备状态："), 5, 0);
            layout->addWidget(ui_device_status_valid_, 5, 1);
            layout->addWidget(new QLabel("连接状态："), 6, 0);
            layout->addWidget(ui_device_status_connected_, 6, 1);
        }
        auto layout = new QHBoxLayout(device_status_area);
        layout->addWidget(status_table);
        layout->addStretch();
    }

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins({});

    layout->addWidget(device_list_container);
    layout->addWidget(add_device_container);
    layout->addWidget(device_status_container);
    layout->addStretch();

    {
        auto layout = new QHBoxLayout(device_list_container);
        layout->addWidget(ui_detect_devices_);
        layout->addWidget(ui_connect_);
        layout->addWidget(ui_devices_);
        layout->addWidget(ui_device_list_state_);
    }
    {
        auto layout = new QHBoxLayout(add_device_container);
        layout->addWidget(ui_add_device_);
        layout->addWidget(ui_select_device_);
        layout->addWidget(ui_device_path_);
        layout->addWidget(ui_device_address_label_);
        layout->addWidget(ui_device_address_);
        layout->addWidget(ui_add_device_label_);
        layout->addWidget(ui_device_name_);
    }
    {
        auto layout = new QVBoxLayout(device_status_container);
        layout->addWidget(make_titled_widget("当前设备", device_status_area));
    }

    connect(ui_detect_devices_, &FlatButton::pressed, this, &DeviceConn::request_list_devices);
    connect(ui_connect_, &FlatButton::pressed, this, &DeviceConn::post_connect_request);
    connect(this, &DeviceConn::on_list_devices_done, this, &DeviceConn::handle_on_list_devices_done);
    connect(&wait_progress_timer_, &QTimer::timeout, this, &DeviceConn::handle_on_update_wait_progress);
    connect(ui_add_device_, &FlatButton::pressed, this, &DeviceConn::handle_on_add_local_device);
    connect(ui_select_device_, &FlatButton::pressed, this, &DeviceConn::handle_on_select_local_device);
    connect(ui_devices_, &QComboBox::currentIndexChanged, this, &DeviceConn::handle_on_selected_device_changed);
}

} // namespace UI

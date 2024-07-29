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

#include "FlatButton.h"

#include <MaaPP/MaaPP.hpp>
#include <QtCore/QTimer>
#include <QtWidgets/QWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <qtmaterialtextfield.h>
#include <stdint.h>

struct MaaAdbDevice {
    QString name;
    QString path;
    QString address;
    QString config;
    int32_t type;
    bool    auto_detect;
};

Q_DECLARE_METATYPE(MaaAdbDevice);

enum AdbDeviceConnectState {
    AdbDeviceConnectState_Available,
    AdbDeviceConnectState_Pending,
    AdbDeviceConnectState_Connected,
};

struct MaaAdbDeviceInfo {
    MaaAdbDevice device;
    int          connect_state;
};

Q_DECLARE_METATYPE(MaaAdbDeviceInfo);

namespace UI {

class DeviceConn : public QWidget {
    Q_OBJECT

protected:
    friend class Client;

public:
    static int32_t default_device_type();
    static QString encode_device_type(int32_t type);

protected:
    void request_list_devices();
    void add_device(const MaaAdbDevice &device);

public slots:
    void handle_on_list_devices_done(QList<maa::AdbDevice> devices);

protected slots:
    void set_waiting_for_list_devices(bool on);
    void post_connect_request();
    void update_device_status();
    void update_device_list_state();

    void handle_on_update_wait_progress();
    void handle_on_add_local_device();
    void handle_on_select_local_device();
    void handle_on_selected_device_changed();
    void handle_on_request_connect_device_done(int maa_status);

signals:
    void on_list_devices_done(QList<maa::AdbDevice> devices);
    void on_request_connect_device(MaaAdbDevice device);

public:
    DeviceConn(QWidget *parent = nullptr);
    ~DeviceConn() override;

protected:
    void setup();

private:
    maa::coro::Promise<void> fut_list_devices_;
    QTimer                   wait_progress_timer_;
    int                      wait_progress_status_;
    bool                     wait_connect_response_;
    FlatButton              *ui_detect_devices_          = nullptr;
    FlatButton              *ui_connect_                 = nullptr;
    QComboBox               *ui_devices_                 = nullptr;
    QLabel                  *ui_device_list_state_       = nullptr;
    FlatButton              *ui_add_device_              = nullptr;
    FlatButton              *ui_select_device_           = nullptr;
    QtMaterialTextField     *ui_device_path_             = nullptr;
    QLabel                  *ui_device_address_label_    = nullptr;
    QtMaterialTextField     *ui_device_address_          = nullptr;
    QLabel                  *ui_add_device_label_        = nullptr;
    QtMaterialTextField     *ui_device_name_             = nullptr;
    QWidget                 *ui_device_status_container_ = nullptr;
    QLabel                  *ui_device_status_name_      = nullptr;
    QLabel                  *ui_device_status_path_      = nullptr;
    QLabel                  *ui_device_status_type_      = nullptr;
    QLabel                  *ui_device_status_address_   = nullptr;
    QLabel                  *ui_device_status_config_    = nullptr;
    QLabel                  *ui_device_status_valid_     = nullptr;
    QLabel                  *ui_device_status_connected_ = nullptr;
};

} // namespace UI

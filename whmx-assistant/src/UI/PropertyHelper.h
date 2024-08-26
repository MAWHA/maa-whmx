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

#define Q_DECLARE_PROPERTY(type, name)                                                                   \
    Q_PROPERTY(type name READ name WRITE set_##name MEMBER property_##name##_ NOTIFY on_##name##_change) \
                                                                                                         \
signals:                                                                                                 \
    Q_SIGNAL void on_##name##_change();                                                                  \
                                                                                                         \
public:                                                                                                  \
    void set_##name(type value) {                                                                        \
        property_##name##_ = value;                                                                      \
        emit on_##name##_change();                                                                       \
    }                                                                                                    \
                                                                                                         \
    type name() const& {                                                                                 \
        return property_##name##_;                                                                       \
    }                                                                                                    \
    type& name()& {                                                                                      \
        return property_##name##_;                                                                       \
    }                                                                                                    \
                                                                                                         \
private:                                                                                                 \
    type property_##name##_;

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
#include <algorithm>
#include <optional>

int           min_edit_distance(const QString &src, const QString &dst);
QList<double> softmax(const QList<double> &vec);
int           choice(const QList<double> &weights);
int           choice(int min_index, int max_index);
QList<int>    multi_choice(int n, int min_index, int max_index);
double        eval_color_distance(const int (&lhs_rgb)[3], const int (&rhs_rgb)[3]);

template <typename Container, typename T>
    requires std::convertible_to<T, decltype(*std::declval<Container>().begin())>
auto find_element(const Container &container, const T &value) {
    return std::find(container.begin(), container.end(), value);
}

template <typename Container, typename T, typename G = std::remove_cvref_t<decltype(*std::declval<Container>().begin())>>
    requires std::is_invocable_r_v<bool, T, const G &>
auto find_element(const Container &container, T &&pred) {
    return std::find_if(container.begin(), container.end(), pred);
}

template <
    typename Container,
    typename T,
    typename E     = decltype(*std::declval<Container>().begin()),
    typename MemFn = T (E::*)()>
auto find_element(Container &&container, MemFn member, const T &value) {
    return std::find_if(container.begin(), container.end(), [=](const auto &elem) {
        return elem.*member == value;
    });
}

template <typename Container, typename... Args, typename T = std::remove_cvref_t<decltype(*std::declval<Container>().begin())>>
std::optional<T> find_element_opt(Container &&container, Args &&...args) {
    const auto it = find_element(container, std::forward<Args>(args)...);
    if (it == container.end()) {
        return std::nullopt;
    } else {
        return std::make_optional(*it);
    }
}

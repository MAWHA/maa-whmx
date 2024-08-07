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

#include "Algorithm.h"

#include <vector>
#include <algorithm>
#include <random>

int min_edit_distance(const QString &src, const QString &dst) {
    const int m = dst.length() + 1;
    const int n = src.length() + 1;

    std::vector<int> dp(m * n);

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            const int k = i * n + j;
            if (i == 0 || j == 0) {
                dp[k] = 0;
            } else if (dst.at(i - 1) == src.at(j - 1)) {
                dp[k] = dp[k - n - 1];
            } else {
                dp[k] = std::min(std::min(dp[k - 1], dp[k - n]), dp[k - n - 1]) + 1;
            }
        }
    }

    return dp.back();
}

QList<double> softmax(const QList<double> &vec) {
    auto         probs  = vec;
    const double maxval = *std::max_element(vec.begin(), vec.end());
    for (auto &prob : probs) { prob = exp(prob - maxval); }
    const double sum = std::accumulate(probs.begin(), probs.end(), 0.0);
    for (auto &prob : probs) { prob /= sum; }
    return probs;
}

int choice(const QList<double> &weights) {
    if (weights.empty()) { return -1; }
    QList<double> acc_weights;
    acc_weights.append(weights.front());
    for (int i = 1; i < weights.size(); ++i) { acc_weights.append(acc_weights[i - 1] + weights[i]); }
    auto         dist = std::uniform_real_distribution<double>(0, acc_weights.back());
    std::mt19937 rng(std::random_device{}());
    const double value = dist(rng);
    for (int i = 0; i < weights.size(); ++i) {
        if (value <= acc_weights[i]) { return i; }
    }
    return weights.size() - 1;
}

int choice(int min_index, int max_index) {
    std::mt19937 rng(std::random_device{}());
    return rng() % (max_index - min_index + 1) + min_index;
}

QList<int> multi_choice(int n, int min_index, int max_index) {
    //! TODO: better algorithm
    const int expected = std::min(std::max(1, n), max_index - min_index + 1);
    Q_ASSERT(expected == n);
    QList<int> choices;
    while (choices.size() < expected) {
        const int value = choice(min_index, max_index);
        if (choices.contains(value)) { continue; }
        choices.append(value);
    }
    return choices;
}

double eval_color_distance(const int (&lhs_rgb)[3], const int (&rhs_rgb)[3]) {
    //! NOTE: see https://www.compuphase.com/cmetric.htm
    const double r_mean       = (lhs_rgb[0] + rhs_rgb[0]) * 0.5;
    const double diff_r       = lhs_rgb[0] - rhs_rgb[0];
    const double diff_g       = lhs_rgb[1] - rhs_rgb[1];
    const double diff_b       = lhs_rgb[2] - rhs_rgb[2];
    const double diff_r_squre = diff_r * diff_r;
    const double diff_g_squre = diff_g * diff_g;
    const double diff_b_squre = diff_b * diff_b;
    return sqrt((2 + r_mean / 256) * diff_r_squre + 4 * diff_g_squre + (2 + (255 - r_mean) / 256) * diff_b_squre);
}

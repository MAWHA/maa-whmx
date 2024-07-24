#include "Algorithm.h"

#include <vector>
#include <algorithm>

int min_edit_distance(const QString &src, const QString &dst) {
    const int m = dst.length() + 1;
    const int n = src.length() + 1;

    std::vector<int> dp(m * n);

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            const int k = i * n + j;
            if (i == 0 || j == 0) {
                dp[k] = 0;
            } else if (src.at(i - 1) == dst.at(j - 1)) {
                dp[k] = dp[k - n - 1];
            } else {
                dp[k] = std::min(std::min(dp[k - 1], dp[k - n]), dp[k - n - 1]) + 1;
            }
        }
    }

    return dp.back();
}

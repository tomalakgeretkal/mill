#pragma once
#include <iterator>

namespace mill {
    template<typename C, typename K>
    auto largestBeforeOrEqual(C const& c, K const& k) {
        auto ub = c.upper_bound(k);
        return ub == c.begin() ? c.end() : std::prev(ub);
    }
}

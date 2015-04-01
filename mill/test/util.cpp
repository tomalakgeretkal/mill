#include "../src/util.hpp"
#include <catch.hpp>
#include <cstddef>
#include <set>

using namespace mill;

TEST_CASE("largestBeforeOrEqual", "") {
    std::set<std::size_t> const c{1, 2, 7, 13};
    CHECK(c.end() == largestBeforeOrEqual(c, 0));

    auto lboe = [&] (std::size_t k) {
        auto lboe = largestBeforeOrEqual(c, k);
        if (lboe == c.end()) {
            FAIL("cannot be end() iterator");
        }
        return lboe;
    };

    CHECK(1 == *lboe(1));
    CHECK(2 == *lboe(2));
    CHECK(2 == *lboe(3));
    CHECK(2 == *lboe(4));
    CHECK(2 == *lboe(5));
    CHECK(2 == *lboe(6));
    CHECK(7 == *lboe(7));
    CHECK(7 == *lboe(8));
    CHECK(7 == *lboe(9));
    CHECK(7 == *lboe(10));
    CHECK(7 == *lboe(11));
    CHECK(7 == *lboe(12));
    CHECK(13 == *lboe(13));
    CHECK(13 == *lboe(14));
}

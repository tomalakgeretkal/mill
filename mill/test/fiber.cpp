#include "../src/fiber.hpp"
#include <catch.hpp>

using namespace mill;

TEST_CASE("fiber works", "[fiber]") {
    auto a = false;
    auto b = false;
    fiber fiber([&] {
        REQUIRE(&fiber::current() == &fiber);
        a = true;
        REQUIRE(&fiber::current() == &fiber);
        fiber::pause();
        REQUIRE(&fiber::current() == &fiber);
        b = true;
        REQUIRE(&fiber::current() == &fiber);
    });
    REQUIRE(!a);
    REQUIRE(!b);
    fiber.resume();
    REQUIRE(a);
    REQUIRE(!b);
    fiber.resume();
    REQUIRE(a);
    REQUIRE(b);
}

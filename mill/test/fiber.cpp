#include "../src/fiber.hpp"
#include <catch.hpp>

using namespace mill;

TEST_CASE("fiber works", "[fiber]") {
    auto a = false;
    auto b = false;
    std::shared_ptr<fiber> fiber = make_fiber([&] {
        REQUIRE(fiber::current() == fiber);
        a = true;
        fiber::pause();
        REQUIRE(fiber::current() == fiber);
        b = true;
    });
    REQUIRE(!a);
    REQUIRE(!b);
    fiber->resume();
    REQUIRE(a);
    REQUIRE(!b);
    fiber->resume();
    REQUIRE(a);
    REQUIRE(b);
}

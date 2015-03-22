#include "../src/gc.hpp"
#include "../src/value.hpp"
#include <catch.hpp>

TEST_CASE("GC::alloc allocates memory for a value", "[GC]") {
    mill::GC gc;
    auto& type = mill::PrimitiveType<double>::instance();
    auto value = gc.alloc(type);
    REQUIRE(value->type == &type);
}

#include "../src/gc.hpp"
#include "../src/value.hpp"
#include <catch.hpp>

TEST_CASE("primitive values can be get and set", "[PrimitiveType]") {
    mill::GC gc;
    auto& type = mill::PrimitiveType<double>::instance();
    auto value = gc.alloc(type);
    value->type = &type;
    type.set(value, 1.0);
    REQUIRE(type.get(value) == 1.0);
}

TEST_CASE("struct fields can be get and set", "[Field]") {
    mill::GC gc;
    auto& type1 = mill::PrimitiveType<double>::instance();
    auto& type2 = mill::PrimitiveType<char>::instance();
    mill::StructType type3({&type1, &type2});

    auto value1 = gc.alloc(type1);
    value1->type = &type1;
    auto value2 = gc.alloc(type2);
    value2->type = &type2;
    auto value3 = gc.alloc(type3);
    value3->type = &type3;

    type3.fields().first[0].set(value3, value1);
    type3.fields().first[1].set(value3, value2);

    REQUIRE(type3.fields().first[0].get(gc, value3) == value1);
    REQUIRE(type3.fields().first[1].get(gc, value3) == value2);
}

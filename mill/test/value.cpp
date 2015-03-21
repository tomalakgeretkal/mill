#include "../src/value.hpp"
#include <catch.hpp>

TEST_CASE("primitive values can be get and set", "[PrimitiveType]") {
    auto& type = mill::PrimitiveType<double>::instance();
    auto value = static_cast<mill::Value*>(operator new(type.size()));
    value->type = &type;
    type.set(value, 1.0);
    REQUIRE(type.get(value) == 1.0);
}

TEST_CASE("struct fields can be get and set", "[Field]") {
    auto& type1 = mill::PrimitiveType<double>::instance();
    auto& type2 = mill::PrimitiveType<char>::instance();
    mill::StructType type3({&type1, &type2});

    auto value1 = static_cast<mill::Value*>(operator new(type1.size()));
    value1->type = &type1;
    auto value2 = static_cast<mill::Value*>(operator new(type2.size()));
    value2->type = &type2;
    auto value3 = static_cast<mill::Value*>(operator new(type3.size()));
    value3->type = &type3;

    type3.fields().first[0].set(value3, value1);
    type3.fields().first[1].set(value3, value2);

    REQUIRE(type3.fields().first[0].get(value3) == value1);
    REQUIRE(type3.fields().first[1].get(value3) == value2);
}

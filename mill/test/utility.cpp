#include "../src/utility.hpp"
#include <catch.hpp>
#include <cstdint>
#include <vector>

using namespace mill;

TEST_CASE("read_integer should work", "[utility]") {
    std::vector<char> data{0x01, 0x02, 0x03, 0x04};
    auto result = read_integer<std::uint32_t>(data.begin(), byte_order::little_endian);
    REQUIRE(result == 0x04030201);
}

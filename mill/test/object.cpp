#include "../src/object.hpp"
#include <catch.hpp>
#include <string>
#include <vector>

using namespace mill;

TEST_CASE("read_object should work", "[object]") {
    std::vector<unsigned char> data{
        0xDE, 0xAD, 0xBE, 0xEF,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00,

        0x05, 0x00, 0x00, 0x00,
        0x07, 0x00, 0x00, 0x00, 's', 't', 'd', ':', ':', 'i', 'o',
        0x10, 0x00, 0x00, 0x00, 's', 't', 'd', ':', ':', 'i', 'o', ':', ':', 'w', 'r', 'i', 't', 'e', 'l', 'n',
        0x0D, 0x00, 0x00, 0x00, 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!',
        0x04, 0x00, 0x00, 0x00, 'M', 'A', 'I', 'N',
        0x04, 0x00, 0x00, 0x00, 'm', 'a', 'i', 'n',

        0x04, 0x00, 0x00, 0x00,

        0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,

        0x01, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x11, 0x00, 0x00, 0x00,
        0x01, 0x01, 0x00, 0x00, 0x00,
        0x02, 0x02, 0x00, 0x00, 0x00,
        0x03, 0x01, 0x00, 0x00, 0x00,
        0x04,
        0x05,
    };

    auto object = read_object(data.begin(), data.end());
    REQUIRE(object.strings == (std::vector<std::string>{"std::io", "std::io::writeln", "Hello, world!", "MAIN", "main"}));
    REQUIRE(object.name == 4);
    REQUIRE(object.dependencies == (std::vector<std::size_t>{0}));

    auto main = object.subroutines.at(0);
    REQUIRE(main.name == 3);
    REQUIRE(main.parameter_count == 0);
    REQUIRE(main.body == std::vector<unsigned char>(data.begin() + 106, data.end()));
}

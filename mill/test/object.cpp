#include "../src/object.hpp"
#include <baka/io/pipe.hpp>
#include <catch.hpp>
#include <cstddef>
#include <string>
#include <vector>

TEST_CASE("readObject", "[object_file]") {
    auto pair = baka::io::pipe();

    std::vector<unsigned char> data{
        0xDE, 0xAD, 0xBE, 0xEF,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00,

        0x04, 0x00, 0x00, 0x00,
        0x07, 0x00, 0x00, 0x00, 's', 't', 'd', ':', ':', 'i', 'o',
        0x10, 0x00, 0x00, 0x00, 's', 't', 'd', ':', ':', 'i', 'o', ':', ':', 'w', 'r', 'i', 't', 'e', 'l', 'n',
        0x0D, 0x00, 0x00, 0x00, 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!',
        0x04, 0x00, 0x00, 0x00, 'M', 'A', 'I', 'N',

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
    pair.second.write((char*)data.data(), (char*)data.data() + data.size());

    auto object = mill::readObject(pair.first);
    REQUIRE(object.strings == (std::vector<std::string>{"std::io", "std::io::writeln", "Hello, world!", "MAIN"}));
    REQUIRE(object.dependencies == (std::vector<std::size_t>{0}));

    auto main = object.subroutines.at(0);
    REQUIRE(main.name == 3);
    REQUIRE(main.parameterCount == 0);
    REQUIRE(main.body == std::vector<unsigned char>(data.begin() + 94, data.end()));
}

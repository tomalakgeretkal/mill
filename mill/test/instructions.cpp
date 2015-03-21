#include "../src/instructions.hpp"
#include <baka/io/memory_stream.hpp>
#include <cstdint>
#include <catch.hpp>

TEST_CASE("readInstruction", "[instructions]") {
    std::vector<char> data{0x02, 0x04, 0x03, 0x02, 0x01};
    baka::io::memory_stream stream;
    stream.write(data.data(), data.data() + data.size());
    stream.seek_begin(0);
    struct : mill::DummyInstructionVisitor<std::uint32_t> {
        std::uint32_t visitPushString(std::uint32_t id) {
            return id;
        }
    } visitor;
    REQUIRE(mill::readInstruction(stream, visitor) == 0x01020304);
}

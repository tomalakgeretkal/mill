#include "../src/data.hpp"
#include "../src/interpret.hpp"
#include "../src/tape.hpp"
#include <boost/optional.hpp>
#include <catch.hpp>
#include <vector>

using namespace mill;

TEST_CASE("", "[interpret]") {
    std::vector<unsigned char> code{0x07, 0x01, 0x05};
    tape<decltype(code.begin())> tape(code.begin(), code.end());
    std::vector<handle> arguments;
    interpret(
        tape,
        arguments.begin(),
        arguments.end(),
        [] (auto) { return boost::none; },
        [] (auto) { return boost::none; }
    );
}

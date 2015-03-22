#include "../src/interpreter.hpp"
#include "../src/object.hpp"
#include "../src/vm.hpp"
#include <baka/io/memory_stream.hpp>
#include "nonius.h++"
#include <vector>

NONIUS_BENCHMARK("empty procedure", [] (nonius::chronometer meter) {
    mill::Object object;
    mill::VM vm;

    meter.measure([&] {
        std::vector<char> data{ 0x06, 0x05 };
        baka::io::memory_stream source;
        source.write(data.data(), data.data() + data.size());
        source.seek_begin(0);
        return mill::interpret(vm, object, source);
    });
});

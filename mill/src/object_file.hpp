#pragma once
#include <baka/binary.hpp>
#include <baka/exception.hpp>
#include <baka/io/util.hpp>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace mill {
    BAKA_EXCEPTION(BadObject, std::runtime_error);

    struct Object {
        std::vector<std::string> strings;
        std::vector<std::size_t> dependencies;
    };

    template<typename Reader>
    Object readObject(Reader& reader) {
#define INT(T) baka::read_integer<T>(reader, baka::byte_order::little_endian)

        std::string magic(4, '\0');
        baka::io::read_full(reader, &magic[0], &magic[0] + magic.size());
        if (magic != "\xDE\xAD\xBE\xEF") {
            throw BadObject("bad magic number");
        }

        auto versionMajor = INT(std::uint16_t);
        auto versionMinor = INT(std::uint16_t);
        auto versionPatch = INT(std::uint16_t);
        if (versionMajor != 0 && versionMinor != 0 && versionPatch != 1) {
            throw BadObject("bad version number");
        }

        auto stringCount = INT(std::uint32_t);
        std::vector<std::string> strings(stringCount);
        for (auto& string : strings) {
            auto stringLength = INT(std::uint32_t);
            string.resize(stringLength);
            baka::io::read_full(reader, &string[0], &string[0] + string.size());
        }

        auto dependencyCount = INT(std::uint32_t);
        std::vector<std::size_t> dependencies(dependencyCount);
        for (auto& dependency : dependencies) {
            dependency = INT(std::uint32_t);
        }

        Object object;
        object.strings = std::move(strings);
        object.dependencies = std::move(dependencies);
        return object;

#undef INT
    }
}

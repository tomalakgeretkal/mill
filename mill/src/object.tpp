#include "object.hpp"
#include <string>
#include "utility.hpp"

template<typename It>
mill::object mill::read_object(It begin, It end) {
#define INT(T) (read_integer<T>(begin, end, byte_order::little_endian))

    std::string magic(4, '\0');
    read_exactly(begin, end, magic.size(), magic.begin());
    if (magic != "\xDE\xAD\xBE\xEF") {
        throw "bad magic number";
    }

    auto version_major = INT(std::uint16_t);
    auto version_minor = INT(std::uint16_t);
    auto version_patch = INT(std::uint16_t);
    if (version_major != 0 && version_minor != 0 && version_patch != 1) {
        throw "bad version number";
    }

    auto string_count = INT(std::uint32_t);
    std::vector<std::string> strings(string_count);
    for (auto& string : strings) {
        auto string_length = INT(std::uint32_t);
        string.resize(string_length);
        read_exactly(begin, end, string_length, string.begin());
    }

    auto name = INT(std::uint32_t);

    auto dependencyCount = INT(std::uint32_t);
    std::vector<std::size_t> dependencies(dependencyCount);
    for (auto& dependency : dependencies) {
        dependency = INT(std::uint32_t);
    }

    auto subroutine_count = INT(std::uint32_t);
    std::vector<object::subroutine> subroutines(subroutine_count);
    for (auto& subroutine : subroutines) {
        subroutine.name = INT(std::uint32_t);
        subroutine.parameter_count = INT(std::uint32_t);
        auto body_length = INT(std::uint32_t);
        subroutine.body.resize(body_length);
        read_exactly(begin, end, body_length, subroutine.body.begin());
    }

    object obj;
    obj.strings = std::move(strings);
    obj.name = name;
    obj.dependencies = std::move(dependencies);
    obj.subroutines = std::move(subroutines);
    return obj;

#undef INT
}

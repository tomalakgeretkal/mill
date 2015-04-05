#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/optional.hpp>
#include "data.hpp"
#include "interpret.hpp"
#include <ios>
#include <iterator>
#include "object.hpp"
#include <stdexcept>
#include <string>
#include "tape.hpp"
#include <utility>
#include "utility.hpp"
#include <vector>

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

template<typename GlobalGetter, typename GlobalSetter>
void mill::load_object(
    object const& object,
    GlobalGetter&& get_global,
    GlobalSetter&& set_global
) {
    for (auto&& subroutine_description : object.subroutines) {
        set_global(
            object.strings[object.name] + "::" + object.strings[subroutine_description.name],
            handle(subroutine([=] (auto arguments_begin, auto arguments_end) {
                auto const& code = subroutine_description.body;
                tape<decltype(code.begin())> tape(code.begin(), code.end());
                return interpret(
                    tape,
                    arguments_begin,
                    arguments_end,
                    [&] (auto index) -> boost::optional<handle> {
                        try {
                            return get_global(object.strings.at(index));
                        } catch (std::out_of_range const&) {
                            return boost::none;
                        }
                    },
                    [&] (auto index) -> boost::optional<handle> {
                        try {
                            return handle(string(object.strings.at(index)));
                        } catch (std::out_of_range const&) {
                            return boost::none;
                        }
                    }
                );
            }))
        );
    }
}

template<typename GlobalGetter, typename GlobalSetter>
void mill::load_object(
    boost::filesystem::path const& path,
    GlobalGetter&& get_global,
    GlobalSetter&& set_global
) {
    boost::filesystem::ifstream stream;
    stream.exceptions(std::ios::failbit);
    stream.open(path, std::ios::in | std::ios::binary);
    std::vector<unsigned char> object_file(
        (std::istreambuf_iterator<char>(stream)),
        std::istreambuf_iterator<char>()
    );
    auto object = read_object(object_file.begin(), object_file.end());
    load_object(
        object,
        std::forward<GlobalGetter>(get_global),
        std::forward<GlobalSetter>(set_global)
    );
}

template<typename GlobalGetter, typename GlobalSetter>
void mill::load_object(
    std::string const& name,
    search_path const& search_path,
    GlobalGetter&& get_global,
    GlobalSetter&& set_global
) {
    using boost::algorithm::replace_all_copy;
    boost::filesystem::path relative_path(replace_all_copy(name, "::", "/"));
    for (auto&& directory_path : search_path) {
        auto path = (directory_path / relative_path) += ".millo";
        if (boost::filesystem::exists(path)) {
            load_object(
                path,
                std::forward<GlobalGetter>(get_global),
                std::forward<GlobalSetter>(set_global)
            );
            return;
        }
    }
    throw no_such_object(name);
}

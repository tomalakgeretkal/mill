#pragma once
#include <string>
#include <vector>

namespace mill {
    // A Mill object file.
    struct object {
        struct subroutine {
            std::size_t name;
            std::size_t parameter_count;
            std::vector<unsigned char> body;
        };

        std::vector<std::string> strings;
        std::size_t name;
        std::vector<std::size_t> dependencies;
        std::vector<subroutine> subroutines;
    };

    // Read an object in binary format into an object object.
    template<typename It>
    object read_object(It begin, It end);
}

#include "object.tpp"

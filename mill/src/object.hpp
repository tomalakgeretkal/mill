#pragma once
#include <boost/filesystem.hpp>
#include <deque>
#include <stdexcept>
#include <string>
#include <vector>

namespace mill {
    // Thrown when no object with the given name could be found.
    class no_such_object : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

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

    // A sequence of paths which are searched in order to find an
    // object given its name.
    using search_path = std::deque<boost::filesystem::path>;

    // Load an object from an object object.
    //
    // - get_global is a callable that takes a string and returns an optional
    //   handle.
    // - set_global is a callable that takes a string and a handle and returns
    //   void.
    template<typename GlobalGetter, typename GlobalSetter>
    void load_object(
        object const& object,
        GlobalGetter&& get_global,
        GlobalSetter&& set_global
    );

    // Load the object at the given path.
    //
    // See the first overload for more information.
    template<typename GlobalGetter, typename GlobalSetter>
    void load_object(
        boost::filesystem::path const& path,
        GlobalGetter&& get_global,
        GlobalSetter&& set_global
    );

    // Load the object with the given name in the given search path. Throw
    // no_such_object if the object could not be found.
    //
    // See the first overload for more information.
    template<typename GlobalGetter, typename GlobalSetter>
    void load_object(
        std::string const& name,
        search_path const& search_path,
        GlobalGetter&& get_global,
        GlobalSetter&& set_global
    );
}

#include "object.tpp"

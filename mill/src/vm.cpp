#include <baka/io/memory_stream.hpp>
#include "interpreter.hpp"
#include "object.hpp"
#include "vm.hpp"

void mill::VM::loadObject(Object const& object) {
    for (auto&& string : object.strings) {
        strings[&object].push_back(this->string(string));
    }

    for (auto&& subroutine : object.subroutines) {
        auto const& body =
            std::find_if(object.subroutines.begin(),
                         object.subroutines.end(),
                         [&] (auto const& s) { return object.strings.at(s.name) == object.strings[subroutine.name]; })
            ->body;
        globals[object.strings[object.name] + "::" + object.strings[subroutine.name]] =
            this->subroutine([&object, &body] (VM& vm, std::size_t, GCPtr*) {
                baka::io::memory_stream bodyReader;
                bodyReader.write((char*)body.data(), (char*)body.data() + body.size());
                bodyReader.seek_begin(0);
                return interpret(vm, object, bodyReader);
            });
    }
}

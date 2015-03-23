#include <baka/io/memory_stream.hpp>
#include <boost/intrusive_ptr.hpp>
#include "interpreter.hpp"
#include "object.hpp"
#include <string>
#include <utility>
#include "value.hpp"
#include "vm.hpp"

void mill::VM::loadObject(Object const& object) {
    for (auto&& string : object.strings) {
        strings[&object].emplace_back(make<String>(string));
    }

    for (auto&& subroutine : object.subroutines) {
        auto const& body =
            std::find_if(object.subroutines.begin(), object.subroutines.end(),
                         [&] (auto const& s) { return object.strings.at(s.name) == object.strings[subroutine.name]; })
            ->body;
        globals[object.strings[object.name] + "::" + object.strings[subroutine.name]] =
            make<Subroutine>([&object, &body] (VM& vm, std::size_t, boost::intrusive_ptr<Value>*) {
                baka::io::memory_stream bodyReader;
                bodyReader.write((char*)body.data(), (char*)body.data() + body.size());
                bodyReader.seek_begin(0);
                return interpret(vm, object, bodyReader);
            });
    }
}

void mill::VM::setGlobal(std::string const& name, boost::intrusive_ptr<Value> value) {
    globals.emplace(name, std::move(value));
}

boost::intrusive_ptr<mill::Value> mill::VM::global(std::string const& name) const {
    return globals.at(name);
}

boost::intrusive_ptr<mill::Value> mill::VM::global(Object const& object, std::size_t nameIndex) const {
    return global(static_cast<String const*>(strings.at(&object)[nameIndex].get())->value());
}

boost::intrusive_ptr<mill::Value> mill::VM::string(Object const& object, std::size_t index) const {
    return strings.at(&object)[index];
}

#include <atomic>
#include <baka/io/memory_stream.hpp>
#include <boost/intrusive_ptr.hpp>
#include <future>
#include "interpreter.hpp"
#include "jit.hpp"
#include <memory>
#include "object.hpp"
#include <string>
#include <utility>
#include "value.hpp"
#include "vm.hpp"

namespace {
    auto constexpr jitThreshold = 10;
}

void mill::VM::loadObject(Object const& object) {
    for (auto&& string : object.strings) {
        strings[&object].emplace_back(make<String>(string));
    }

    for (auto&& subroutine : object.subroutines) {
        auto const& body = subroutine.body;
        auto& subroutineGlobal = globals[object.strings[object.name] + "::" + object.strings[subroutine.name]];
        subroutineGlobal = make<Subroutine>([this, callCount = std::make_shared<std::atomic<long>>(0), &object, &body, &subroutineGlobal]
                                            (VM& vm, std::size_t argc, Value** argv) mutable {
            if ((*callCount)++ == jitThreshold) {
                baka::io::memory_stream bodyReader;
                bodyReader.write((char*)body.data(), (char*)body.data() + body.size());
                bodyReader.seek_begin(0);

                auto& subroutineValue = *static_cast<Subroutine*>(subroutineGlobal.get());
                subroutineValue.fast = jitCompile(vm, object, bodyReader);
                subroutineValue.fastAvailable = true;

                return subroutineValue.fast(vm, argc, argv);
            } else {
                baka::io::memory_stream bodyReader;
                bodyReader.write((char*)body.data(), (char*)body.data() + body.size());
                bodyReader.seek_begin(0);
                return interpret(vm, object, bodyReader, argv);
            }
        });
    }
}

void mill::VM::setGlobal(std::string const& name, boost::intrusive_ptr<Value> value) {
    if (!name.empty() && name[0] != '$' && name[0] != '%' && globals.count(name)) {
        throw std::runtime_error("cannot mutate immutable global '" + name + "'");
    }
    globals.emplace(name, std::move(value));
}

boost::intrusive_ptr<mill::Value> mill::VM::global(std::string const& name) const {
    return globals.at(name);
}

boost::intrusive_ptr<mill::Value> mill::VM::global(Object const& object, std::size_t nameIndex) const {
    return global(static_cast<String const*>(strings.at(&object)[nameIndex].get())->value);
}

boost::intrusive_ptr<mill::Value> mill::VM::string(Object const& object, std::size_t index) const {
    return strings.at(&object)[index];
}

std::future<boost::intrusive_ptr<mill::Value>> mill::VM::call(Value* value, std::size_t argc, Value** argv) {
    std::packaged_task<boost::intrusive_ptr<mill::Value>()> task([=] {
        return dynamic_cast<Subroutine&>(*value)(*this, argc, argv);
    });
    auto result = task.get_future();
    threadPool.post(std::move(task));
    return result;
}

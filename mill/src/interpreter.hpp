#pragma once
#include <algorithm>
#include <boost/intrusive_ptr.hpp>
#include <cstdint>
#include "instructions.hpp"
#include "object.hpp"
#include <stack>
#include "value.hpp"
#include <vector>
#include "vm.hpp"

namespace mill {
    namespace detail {
        template<typename ReaderSeeker>
        class Interpreter {
        public:
            Interpreter(VM& vm, Object const& object, ReaderSeeker& source)
                : vm(&vm), object(&object), source(&source) { }

            boost::intrusive_ptr<Value> operator()() {
                boost::intrusive_ptr<Value> result;
                while (!result) {
                    result = readInstruction(*source, *this);
                }
                return result;
            }

            boost::intrusive_ptr<Value> visitPushGlobal(std::uint32_t nameIndex) {
                stack.push(vm->global(*object, nameIndex));
                return nullptr;
            }

            boost::intrusive_ptr<Value> visitPushString(std::uint32_t index) {
                stack.push(vm->string(*object, index));
                return nullptr;
            }

            boost::intrusive_ptr<Value> visitPushBoolean(std::uint8_t value) {
                stack.push(make<Boolean>(!!value));
                return nullptr;
            }

            boost::intrusive_ptr<Value> visitPushUnit() {
                stack.push(make<Unit>());
                return nullptr;
            }

            boost::intrusive_ptr<Value> visitPop() {
                stack.pop();
                return nullptr;
            }

            boost::intrusive_ptr<Value> visitCall(std::uint32_t argc) {
                std::vector<boost::intrusive_ptr<Value>> argv(argc);
                for (decltype(argc) i = 0; i < argc; ++i) {
                    argv[i] = stack.top();
                    stack.pop();
                }
                std::reverse(argv.begin(), argv.end());

                auto callee = stack.top();
                stack.pop();

                auto result = dynamic_cast<Subroutine&>(*callee).value(*vm, argc, argv.data());
                stack.push(result);

                return nullptr;
            }

            boost::intrusive_ptr<Value> visitReturn() {
                return stack.top();
            }

        private:
            VM* vm;
            Object const* object;
            ReaderSeeker* source;
            std::stack<boost::intrusive_ptr<Value>> stack;
        };
    }

    template<typename ReaderSeeker>
    boost::intrusive_ptr<Value> interpret(VM& vm, Object const& object, ReaderSeeker& source) {
        detail::Interpreter<ReaderSeeker> interpreter(vm, object, source);
        return interpreter();
    }
}

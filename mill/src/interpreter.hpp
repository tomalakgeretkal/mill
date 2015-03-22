#pragma once
#include <cstdint>
#include "instructions.hpp"
#include "object.hpp"
#include <stack>
#include "value.hpp"
#include "vm.hpp"

namespace mill {
    namespace detail {
        template<typename ReaderSeeker>
        class Interpreter {
        public:
            Interpreter(VM& vm, Object const& object, ReaderSeeker& source)
                : vm(&vm), object(&object), source(&source) { }

            Value* operator()() {
                Value* result = nullptr;
                while (!result) {
                    result = readInstruction(*source, *this);
                }
                return result;
            }

            Value* visitPushModuleMember(std::uint32_t) {
                throw "not implemented";
            }

            Value* visitPushString(std::uint32_t index) {
                stack.push(vm->string(*object, index));
                return nullptr;
            }

            Value* visitPushBoolean(std::uint8_t value) {
                stack.push(value ? vm->true_() : vm->false_());
                return nullptr;
            }

            Value* visitPushUnit() {
                stack.push(vm->unit());
                return nullptr;
            }

            Value* visitPop() {
                stack.pop();
                return nullptr;
            }

            Value* visitCall(std::uint32_t) {
                throw "not implemented";
            }

            Value* visitReturn() {
                return stack.top();
            }

        private:
            VM* vm;
            Object const* object;
            ReaderSeeker* source;
            std::stack<Value*> stack;
        };
    }

    template<typename ReaderSeeker>
    Value* interpret(VM& vm, Object const& object, ReaderSeeker& source) {
        detail::Interpreter<ReaderSeeker> interpreter(vm, object, source);
        return interpreter();
    }
}

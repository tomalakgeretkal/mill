#pragma once
#include <algorithm>
#include <cstdint>
#include "gc.hpp"
#include "instructions.hpp"
#include <llvm/ADT/SmallVector.h>
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

            GCPtr operator()() {
                GCPtr result;
                while (!result) {
                    result = readInstruction(*source, *this);
                }
                return result;
            }

            GCPtr visitPushGlobal(std::uint32_t nameIndex) {
                stack.push(vm->global(*object, nameIndex));
                return nullptr;
            }

            GCPtr visitPushString(std::uint32_t index) {
                stack.push(vm->string(*object, index));
                return nullptr;
            }

            GCPtr visitPushBoolean(std::uint8_t value) {
                stack.push(value ? vm->true_() : vm->false_());
                return nullptr;
            }

            GCPtr visitPushUnit() {
                stack.push(vm->unit());
                return nullptr;
            }

            GCPtr visitPop() {
                stack.pop();
                return nullptr;
            }

            GCPtr visitCall(std::uint32_t argc) {
                llvm::SmallVector<GCPtr, 4> argv(argc);
                for (decltype(argc) i = 0; i < argc; ++i) {
                    argv[i] = stack.top();
                    stack.pop();
                }
                std::reverse(argv.begin(), argv.end());

                auto callee = stack.top();
                stack.pop();

                auto result =
                    PrimitiveType<VM::Subroutine>::instance().get(callee)
                    .implementation
                    ->operator()(*vm, argc, argv.data());
                stack.push(result);

                return nullptr;
            }

            GCPtr visitReturn() {
                return stack.top();
            }

        private:
            VM* vm;
            Object const* object;
            ReaderSeeker* source;
            std::stack<GCPtr> stack;
        };
    }

    template<typename ReaderSeeker>
    GCPtr interpret(VM& vm, Object const& object, ReaderSeeker& source) {
        detail::Interpreter<ReaderSeeker> interpreter(vm, object, source);
        return interpreter();
    }
}

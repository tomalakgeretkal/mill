#pragma once
#include <algorithm>
#include <boost/intrusive_ptr.hpp>
#include <cstdint>
#include "instructions.hpp"
#include "object.hpp"
#include "value.hpp"
#include <vector>
#include "vm.hpp"

namespace mill {
    namespace detail {
        template<typename ReaderSeeker>
        class Interpreter {
        public:
            Interpreter(VM& vm, Object const& object, ReaderSeeker& source, Value** params)
                : vm(&vm), object(&object), source(&source), params(params) { }

            boost::intrusive_ptr<Value> operator()() {
                boost::intrusive_ptr<Value> result;
                while (!result) {
                    result = readInstruction(*source, *this);
                }
                return result;
            }

            boost::intrusive_ptr<Value> visitPushGlobal(std::uint32_t nameIndex) {
                stack.push_back(vm->global(*object, nameIndex));
                return nullptr;
            }

            boost::intrusive_ptr<Value> visitPushString(std::uint32_t index) {
                stack.push_back(vm->string(*object, index));
                return nullptr;
            }

            boost::intrusive_ptr<Value> visitPushBoolean(std::uint8_t value) {
                stack.push_back(make<Boolean>(!!value));
                return nullptr;
            }

            boost::intrusive_ptr<Value> visitPushUnit() {
                stack.push_back(&Unit::instance());
                return nullptr;
            }

            boost::intrusive_ptr<Value> visitPushParameter(std::uint32_t index) {
                stack.push_back(params[index]);
                return nullptr;
            }

            boost::intrusive_ptr<Value> visitPop() {
                stack.pop_back();
                return nullptr;
            }

            boost::intrusive_ptr<Value> visitSwap() {
                using std::swap;
                swap(stack[stack.size() - 1], stack[stack.size() - 2]);
                return nullptr;
            }

            boost::intrusive_ptr<Value> visitCall(std::uint32_t argc) {
                std::vector<boost::intrusive_ptr<Value>> argvRefs(argc);
                std::vector<Value*> argv(argc);
                for (decltype(argc) i = 0; i < argc; ++i) {
                    argvRefs[i] = stack.back();
                    argv[i] = stack.back().get();
                    stack.pop_back();
                }
                std::reverse(argv.begin(), argv.end());

                auto callee = stack.back();
                stack.pop_back();

                auto result = dynamic_cast<Subroutine&>(*callee)(*vm, argc, argv.data());
                stack.push_back(result);

                return nullptr;
            }

            boost::intrusive_ptr<Value> visitReturn() {
                return stack.back();
            }

            boost::intrusive_ptr<Value> visitUnconditionalJump(std::uint32_t offset) {
                source->seek_begin(offset);
                return nullptr;
            }

            boost::intrusive_ptr<Value> visitConditionalJump(std::uint32_t offset) {
                auto condition = dynamic_cast<Boolean&>(*stack.back()).value;
                stack.pop_back();
                if (condition) {
                    source->seek_begin(offset);
                }
                return nullptr;
            }

        private:
            VM* vm;
            Object const* object;
            ReaderSeeker* source;
            Value** params;
            std::vector<boost::intrusive_ptr<Value>> stack;
        };
    }

    template<typename ReaderSeeker>
    boost::intrusive_ptr<Value> interpret(VM& vm, Object const& object, ReaderSeeker& source, Value** params) {
        detail::Interpreter<ReaderSeeker> interpreter(vm, object, source, params);
        return interpreter();
    }
}

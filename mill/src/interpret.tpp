#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <cassert>
#include "data.hpp"
#include "interpret.hpp"
#include <iterator>
#include "tape.hpp"
#include <utility>
#include <vector>
#include <wheels/adl/swap.h++>

namespace mill {
    namespace detail {
        template<
            typename CodeIt,
            typename ArgumentIt,
            typename GlobalGetter,
            typename StringGetter
        >
        class interpreter : public boost::static_visitor<boost::optional<handle>> {
        public:
            interpreter(
                tape<CodeIt> tape,
                ArgumentIt arguments_begin,
                ArgumentIt arguments_end,
                GlobalGetter& get_global,
                StringGetter& get_string
            )
                : tape(std::move(tape))
                , arguments_begin(std::move(arguments_begin))
                , arguments_end(std::move(arguments_end))
                , get_global(&get_global)
                , get_string(&get_string) { }

            handle operator()() {
                for (;;) {
                    auto instruction = tape.read();
                    auto result = boost::apply_visitor(*this, instruction);
                    if (result) {
                        return *result;
                    }
                }
            }

            boost::optional<handle> operator()(push_global_instruction const& instruction) {
                auto global = (*get_global)(instruction.op0);
                assert(global);
                push(std::move(*global));
                return boost::none;
            }

            boost::optional<handle> operator()(push_string_instruction const&) {
                throw "not implemented";
            }

            boost::optional<handle> operator()(push_boolean_instruction const& instruction) {
                push(handle(static_cast<bool>(instruction.op0)));
                return boost::none;
            }

            boost::optional<handle> operator()(push_unit_instruction const&) {
                push(handle(unit()));
                return boost::none;
            }

            boost::optional<handle> operator()(push_parameter_instruction const& instruction) {
                assert(instruction.op0 < std::distance(arguments_begin, arguments_end));
                push(arguments_begin[instruction.op0]);
                return boost::none;
            }

            boost::optional<handle> operator()(pop_instruction const&) {
                pop();
                return boost::none;
            }

            boost::optional<handle> operator()(swap_instruction const&) {
                assert(stack.size() >= 2);
                wheels::adl::swap(stack[stack.size() - 1], stack[stack.size() - 2]);
                return boost::none;
            }

            boost::optional<handle> operator()(call_instruction const&) {
                throw "not implemented";
            }

            boost::optional<handle> operator()(return_instruction const&) {
                return pop();
            }

            boost::optional<handle> operator()(conditional_jump_instruction const& instruction) {
                if (pop().template data<bool>()) {
                    tape.seek(instruction.op0);
                }
                return boost::none;
            }

            boost::optional<handle> operator()(unconditional_jump_instruction const& instruction) {
                tape.seek(instruction.op0);
                return boost::none;
            }

        private:
            void push(handle value) {
                stack.push_back(std::move(value));
            }

            handle pop() {
                assert(!stack.empty());
                auto result = std::move(stack.back());
                stack.pop_back();
                return result;
            }

            tape<CodeIt> tape;
            ArgumentIt arguments_begin, arguments_end;
            GlobalGetter* get_global;
            StringGetter* get_string;
            std::vector<handle> stack;
        };
    }
}

template<
    typename CodeIt,
    typename ArgumentIt,
    typename GlobalGetter,
    typename StringGetter
>
mill::handle mill::interpret(
    tape<CodeIt> tape,
    ArgumentIt arguments_begin,
    ArgumentIt arguments_end,
    GlobalGetter&& get_global,
    StringGetter&& get_string
) {
    detail::interpreter<CodeIt, ArgumentIt, GlobalGetter, StringGetter> interpreter(
        std::move(tape),
        arguments_begin,
        arguments_end,
        get_global,
        get_string
    );
    return interpreter();
}

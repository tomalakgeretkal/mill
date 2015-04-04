#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <cassert>
#include "data.hpp"
#include "interpret.hpp"
#include "tape.hpp"
#include <utility>
#include <vector>

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

            boost::optional<handle> operator()(push_global_instruction const&) {
                throw "not implemented";
            }

            boost::optional<handle> operator()(push_string_instruction const&) {
                throw "not implemented";
            }

            boost::optional<handle> operator()(push_boolean_instruction const& instruction) {
                stack.emplace_back(static_cast<bool>(instruction.op0));
                return boost::none;
            }

            boost::optional<handle> operator()(push_unit_instruction const&) {
                throw "not implemented";
            }

            boost::optional<handle> operator()(push_parameter_instruction const&) {
                throw "not implemented";
            }

            boost::optional<handle> operator()(pop_instruction const&) {
                throw "not implemented";
            }

            boost::optional<handle> operator()(swap_instruction const&) {
                throw "not implemented";
            }

            boost::optional<handle> operator()(call_instruction const&) {
                throw "not implemented";
            }

            boost::optional<handle> operator()(return_instruction const&) {
                return pop();
            }

            boost::optional<handle> operator()(conditional_jump_instruction const&) {
                throw "not implemented";
            }

            boost::optional<handle> operator()(unconditional_jump_instruction const&) {
                throw "not implemented";
            }

        private:
            handle pop() {
                assert(!stack.empty());
                auto result = stack.back();
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

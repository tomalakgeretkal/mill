#pragma once
#include <cstddef>
#include <cstdint>
#include <baka/io/io_error.hpp>
#include <baka/io/memory_stream.hpp>
#include "instructions.hpp"
#include "object.hpp"
#include <ostream>

namespace mill {
    namespace detail {
        class Disassembler {
        public:
            Disassembler(Object const& object, std::ostream& out)
                : object(&object), out(&out) { }

            void operator()() {
                for (auto&& subroutine : object->subroutines) {
                    *out << object->strings[subroutine.name] << ":\n";
                    baka::io::memory_stream bodyReader;
                    bodyReader.write((char*)subroutine.body.data(), (char*)subroutine.body.data() + subroutine.body.size());
                    bodyReader.seek_begin(0);
                    try {
                        for (;;) {
                            offset = bodyReader.tell();
                            readInstruction(bodyReader, *this);
                        }
                    } catch (baka::io::eof_error const&) { }
                }
            }

            void visitPushGlobal(std::uint32_t nameIndex) {
                *out << offset << "\tPushGlobal " << object->strings[nameIndex] << '\n';
            }

            void visitPushString(std::uint32_t index) {
                *out << offset << "\tPushString \"" << object->strings[index] << "\"\n";
            }

            void visitPushBoolean(std::uint8_t value) {
                *out << offset << "\tPushBoolean " << (value ? "true" : "false") << '\n';
            }

            void visitPushUnit() {
                *out << offset << "\tPushUnit\n";
            }

            void visitPushParameter(std::uint32_t index) {
                *out << offset << "\tPushParameter " << index << '\n';
            }

            void visitPop() {
                *out << offset << "\tPop\n";
            }

            void visitSwap() {
                *out << offset << "\tSwap\n";
            }

            void visitCall(std::uint32_t argc) {
                *out << offset << "\tCall " << argc << '\n';
            }

            void visitReturn() {
                *out << offset << "\tReturn\n";
            }

            void visitConditionalJump(std::uint32_t jumpOffset) {
                *out << offset << "\tConditionalJump " << jumpOffset << '\n';
            }

            void visitUnconditionalJump(std::uint32_t jumpOffset) {
                *out << offset << "\tUnconditionalJump " << jumpOffset << '\n';
            }

        private:
            Object const* object;
            std::ostream* out;
            std::size_t offset;
        };
    }

    inline
    void disassemble(Object const& object, std::ostream& out) {
        detail::Disassembler disassembler(object, out);
        disassembler();
    }
}

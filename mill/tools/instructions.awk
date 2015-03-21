#!/usr/bin/awk -f

BEGIN {
    print "#pragma once"
    print "#include <baka/binary.hpp>"
    print "#include <cstdint>"
    print "namespace mill {"
    print "template<typename Reader, typename Visitor>"
    print "auto readInstruction(Reader& reader, Visitor&& visitor) {"
    print "auto opcode = baka::read_integer<std::uint8_t>(reader, baka::byte_order::little_endian);";
    print "switch (opcode) {"
}

/^\s*(#.*)?$/ { next }

{
    print "#line " NR
    printf "case %d: {\n", $1

    for (i = 3; i <= NF; i++) {
        printf "auto op%d = baka::read_integer<std::%s_t>(reader, baka::byte_order::little_endian);\n", i, $i;
    }

    printf "return visitor.visit%s(", $2
    sep = ""
    for (i = 3; i <= NF; i++) {
        printf "%sop%d", sep, i
        sep = ", "
    }
    printf ");\n"
    print "} // case"
}

{
    mnemonics[length(mnemonics) + 1] = $2
}

END {
    print "default: throw std::runtime_error(\"bad opcode\");"
    print "} // switch (opcode)"
    print "} // readInstruction()"

    print "template<typename T>"
    print "struct DummyInstructionVisitor {"
    for (i in mnemonics) {
        printf "template<typename... Args>"
        printf "T visit%s(Args&&...) {\n", mnemonics[i]
        print "throw \"dummy instruction visitor called\";"
        print "} // visit" mnemonics[i]
    }
    print "}; // struct DummyInstructionVisitor"

    print "} // namespace mill"
}

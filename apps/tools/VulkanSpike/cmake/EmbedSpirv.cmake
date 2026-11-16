# Converts SPIR-V binaries into a C++ header with uint32_t arrays.
# Usage: cmake -DOUT=<header> -DNAMES=a;b -DFILES=a.spv;b.spv -P EmbedSpirv.cmake
set(content "#pragma once\n#include <cstdint>\n#include <cstddef>\n")
list(LENGTH NAMES n)
math(EXPR last "${n} - 1")
foreach(i RANGE ${last})
    list(GET NAMES ${i} name)
    list(GET FILES ${i} file)
    file(READ "${file}" hex HEX)
    string(LENGTH "${hex}" hexlen)
    math(EXPR words "${hexlen} / 8")
    string(APPEND content "inline const std::uint32_t ${name}[] = {\n")
    # Little-endian: bytes b0 b1 b2 b3 -> 0xb3b2b1b0
    string(REGEX REPLACE "(..)(..)(..)(..)" "0x\\4\\3\\2\\1,\n" body "${hex}")
    string(APPEND content "${body}};\n")
    string(APPEND content "inline constexpr std::size_t ${name}_words = ${words};\n")
endforeach()
file(WRITE "${OUT}" "${content}")

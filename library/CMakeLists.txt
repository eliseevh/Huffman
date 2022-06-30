cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 17)

add_library(huffman bit_sequence.cpp decoder.cpp encoder.cpp tree.cpp)

if (NOT MSVC)
    target_compile_options(huffman PRIVATE -Wall -Wno-sign-compare -pedantic)
endif()

option(USE_SANITIZERS "Enable to build with undefined,leak and address sanitizers" OFF)
if (USE_SANITIZERS)
    target_compile_options(huffman PUBLIC -fsanitize=address,undefined,leak -fno-sanitize-recover=all)
    target_link_options(huffman PUBLIC -fsanitize=address,undefined,leak)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(huffman PUBLIC -stdlib=libc++)
endif()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(huffman PUBLIC -D_GLIBCXX_DEBUG)
endif()

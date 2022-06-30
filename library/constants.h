#pragma once
#include <cstddef>

namespace huffman {
static constexpr size_t CHARS_COUNT = 256;
static constexpr size_t LOG_CHARS_COUNT = 8;
static constexpr size_t BYTE_SIZE = 8;
static constexpr size_t LOG_MAX_NODE_NUMBER = 9;
static constexpr size_t MAX_BUFFER_SIZE = 4096 * BYTE_SIZE;
static constexpr size_t TREE_SHORTCUT_SIZE = 4;
static constexpr size_t TREE_SHORTCUT_CHARS_COUNT = 64;
}

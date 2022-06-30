#pragma once

#include "bit_sequence.h"
#include "constants.h"
#include "tree.h"
#include <array>
#include <cstdint>
#include <istream>
#include <memory>
#include <ostream>
#include <vector>

namespace huffman {
struct decoder {
  decoder() = default;
  decoder(decoder const& other) = delete;
  decoder& operator=(decoder const& other) = delete;
  ~decoder() = default;
  std::pair<size_t, size_t> decode(std::istream& input, std::ostream& output);

private:
  static size_t get_header_size(uint8_t first_byte);
  void read_header(std::vector<uint8_t> const& header);
  size_t dump_buffer(std::ostream& output);
  std::unique_ptr<tree> tree_{nullptr};
  bit_sequence buffer;
  uint8_t end_padding{0};
};
} // namespace huffman

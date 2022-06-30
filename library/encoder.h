#pragma once

#include "bit_sequence.h"
#include "constants.h"
#include "tree.h"
#include <array>
#include <cstdint>
#include <istream>
#include <ostream>
#include <vector>

namespace huffman {
struct encoder {
  encoder();

  encoder(encoder const& other) = delete;

  encoder& operator=(encoder const& other) = delete;

  ~encoder() = default;

  void add_chars(std::istream& stream);

  void add_char(uint8_t ch);

  void encode(std::istream& input, std::ostream& output);
  // used only for tests
  bit_sequence encode(std::vector<uint8_t> const& input);

  bit_sequence header() const;

  // It is correct only after added all chars from input
  size_t get_input_size() const;

  // It is correct only after compile
  size_t get_output_size() const;

private:
  void compile();

  static void dump_buffer(bit_sequence& buffer, std::ostream& output);

  bool is_empty() const;
  uint8_t count_size_mod_8() const;
  bool is_compiled{false};
  std::array<bit_sequence, CHARS_COUNT> codes;
  std::array<size_t, CHARS_COUNT> counts{};
};
} // namespace huffman

#pragma once

#include "bit_sequence.h"
#include "constants.h"
#include <array>
#include <cstdint>
#include <ostream>
#include <tuple>
#include <utility>
#include <vector>

namespace huffman {

struct tree {
  tree() = delete;

  tree(tree const& other) = delete;

  tree& operator=(tree const& other) = delete;

  explicit tree(std::array<size_t, CHARS_COUNT> const& counts);

  explicit tree(std::vector<uint16_t> const& traversal);

  ~tree() = default;

  void get_codes(std::array<bit_sequence, CHARS_COUNT>& result) const;

  bit_sequence header() const;

  bool get_char(bit_sequence const& code, size_t& idx, uint8_t& result) const;

  std::pair<size_t, size_t> dump(bit_sequence const& buffer, size_t last_idx,
              std::ostream& output);

private:

  // first idx - node index, second - TREE_SHORTCUT_SIZE-bit number
  std::vector<std::vector<std::pair<std::vector<uint8_t>, size_t>>>
  get_shortcuts() const;
  bit_sequence traversal() const;
  bool get_char(bit_sequence const& code, size_t& idx, uint8_t& result,
                size_t start_node) const;

  bool calculated_shortcuts{false};
  std::vector<std::vector<std::pair<std::vector<uint8_t>, size_t>>> shortcuts;

  size_t root;
  std::vector<uint8_t> leafs;
  std::vector<std::pair<size_t, size_t>> children;
  std::vector<size_t> parents;
};
} // namespace huffman

#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

namespace huffman {
struct bit_sequence {
  bit_sequence() = default;

  bit_sequence(bit_sequence const& other) = default;

  bit_sequence& operator=(bit_sequence const& other) = default;

  ~bit_sequence() = default;

  bit_sequence& append(bool bit);

  bit_sequence& append(bit_sequence const& other);

  bit_sequence& append(uint64_t number, size_t size);

  uint64_t get_number(size_t size, size_t start_idx) const;

  void pop_back();

  size_t size() const;

  bool operator[](size_t i) const;

  void swap(bit_sequence& other);

private:
  static constexpr uint64_t ONES = static_cast<uint64_t>(-1);
  static constexpr uint64_t ELEMENT_SIZE = 64;
  static constexpr uint64_t LOG_ELEMENT_SIZE = 6;

  std::size_t size_{0};
  std::vector<uint64_t> data;
};
} // namespace huffman

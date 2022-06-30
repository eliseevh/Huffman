#include "bit_sequence.h"

namespace huffman {
bit_sequence& bit_sequence::append(bool bit) {
  return append(bit ? ONES : 0, 1);
}

bit_sequence& bit_sequence::append(bit_sequence const& other) {
  size_t other_size = other.size();
  size_t full_nums = other_size >> LOG_ELEMENT_SIZE;
  size_t last_size = other_size % ELEMENT_SIZE;

  for (size_t i = 0; i < full_nums; ++i) {
    append(other.data[i], ELEMENT_SIZE);
  }

  return append(other.data[full_nums], last_size);
}

bit_sequence& bit_sequence::append( // NOLINT(misc-no-recursion)
    uint64_t number, // NOLINT(bugprone-easily-swappable-parameters)
    size_t size) {
  // Ensures that neither size nor free_size will be zero if we go to true
  // branch, to there's no UB in shifts and this->data is not empty
  if (size == 0) {
    return *this;
  }
  size_t free_size =
      (ELEMENT_SIZE - this->size() % ELEMENT_SIZE) % ELEMENT_SIZE;

  if (free_size >= size) {
    // ONES >> (ELEMENT_SIZE - free_size) is number with higher free_size bits is one, others is zero
    // bitwise and creates number that is equal with data.back() on bits we want to set,
    // zero on lower (ELEMENT_SIZE - free_size) bits, other bits we don't care
    // we shift number by (ELEMENT_SIZE - free_size) so the bits we want to set becomes on positions we need
    // bitwise xor creates a number, that contains zeroes on lower (ELEMENT_SIZE - free_size) bits,
    // next size bits contains xor of bits that is now in data.back() and bits we want to be there,
    // other bits we don't care,
    // so when we do bitwise xor with data.back()
    // lower (ELEMENT_SIZE - free_size) bits do not change,
    // next size bits become what we want it to be, other bits we don't care
    uint64_t bitmask = ONES << (ELEMENT_SIZE - free_size);
    uint64_t needed_part = bitmask & data.back();
    uint64_t xor_mask = (number << (ELEMENT_SIZE - free_size)) ^ needed_part;

    data.back() ^= xor_mask;

    size_ += size;
  } else {
    // will go to true or size == 0 branch so no possible infinite recursion
    append(number, free_size);

    data.push_back(number >> free_size);

    size_ += size - free_size;
  }
  return *this;
}

uint64_t bit_sequence::get_number( // NOLINT(misc-no-recursion)
    size_t size, // NOLINT(bugprone-easily-swappable-parameters)
    size_t start_idx) const {
  // Ensures that size won't be zero so there's no UB in shifts
  if (size == 0) {
    return 0;
  }
  size_t taken_size = ELEMENT_SIZE - start_idx % ELEMENT_SIZE;
  if (taken_size >= size) {
    // We shift data so our number will take lower size bits
    // We shift ones so lower size bits will be one, other will be zero
    // Bitwise and creates a number with lower size bits that we need and other
    // are zero, so it is the number we want
    return (data[start_idx >> LOG_ELEMENT_SIZE] >> (start_idx % ELEMENT_SIZE)) &
           (ONES >> (ELEMENT_SIZE - size));
  } else {
    // both will go to true branch so no possible infinite recursion
    uint64_t lower = get_number(taken_size, start_idx);
    uint64_t higher = get_number(size - taken_size, start_idx + taken_size);
    return lower | (higher << taken_size);
  }
}
void bit_sequence::pop_back() {
  --size_;
  if (size_ % ELEMENT_SIZE == 0) {
    data.pop_back();
  }
}
size_t bit_sequence::size() const {
  return size_;
}

bool bit_sequence::operator[](size_t i) const {
  size_t idx = i >> LOG_ELEMENT_SIZE;
  return ((data[idx] >> (i % ELEMENT_SIZE)) & 1u) == 1u;
}

void bit_sequence::swap(bit_sequence& other) {
  std::swap(size_, other.size_);
  std::swap(data, other.data);
}
} // namespace huffman

#include "encoder.h"
#include <cassert>
#include <stdexcept>
#include <string>

namespace huffman {
encoder::encoder() {
  counts.fill(0);
}
void encoder::add_chars(std::istream& stream) {
  while (true) {
    uint8_t ch = stream.get();
    if (stream.fail()) {
      break;
    }
    add_char(ch);
  }
}
void encoder::add_char(uint8_t ch) {
  assert(!is_compiled);
  ++counts[ch];
}

bit_sequence encoder::encode(std::vector<uint8_t> const& input) {
  if (!is_compiled) {
    compile();
  }
  bit_sequence result;
  for (uint8_t ch : input) {
    if (codes[ch].size() == 0) {
      std::string message("Unexpected char to encode: ");
      message.append(1, static_cast<char>(ch));
      throw std::runtime_error(message);
    }
    result.append(codes[ch]);
  }
  return result;
}
void encoder::encode(std::istream& input, std::ostream& output) {
  if (is_empty()) {
    output.put(0);
    return;
  }

  if (!is_compiled) {
    compile();
  }
  bit_sequence buffer(header());
  while (true) {
    uint8_t ch = input.get();
    if (input.fail()) {
      break;
    }
    buffer.append(codes[ch]);

    if (buffer.size() > MAX_BUFFER_SIZE) {
      dump_buffer(buffer, output);
    }
  }
  while (buffer.size() % BYTE_SIZE != 0) {
    buffer.append(false);
  }
  dump_buffer(buffer, output);
  assert(buffer.size() == 0);
}

bit_sequence encoder::header() const {
  if (is_empty()) {
    bit_sequence result;
    result.append(0, BYTE_SIZE);
    return result;
  }
  bit_sequence result = tree(counts).header();

  // add padding
  uint8_t size_mod_8 = (result.size() + 3) % BYTE_SIZE;
  size_mod_8 += count_size_mod_8();
  size_mod_8 %= BYTE_SIZE;

  result.append((BYTE_SIZE - size_mod_8) % BYTE_SIZE, 3);

  return result;
}

void encoder::compile() {
  tree tree_(counts);
  tree_.get_codes(codes);
  is_compiled = true;
}

bool encoder::is_empty() const {
  for (size_t cnt : counts) {
    if (cnt != 0) {
      return false;
    }
  }
  return true;
}

uint8_t encoder::count_size_mod_8() const {
  uint8_t result = 0;
  for (size_t i = 0; i < CHARS_COUNT; ++i) {
    // to prevent size_t overflow, if counts[i] is >
    // std::numeric_limits<size_t>::max() / 8
    result += ((counts[i] % BYTE_SIZE) * codes[i].size()) % BYTE_SIZE;
    result %= BYTE_SIZE;
  }
  return result;
}

void encoder::dump_buffer(bit_sequence& buffer, std::ostream& output) {
  for (size_t i = 0; i < buffer.size() / BYTE_SIZE; ++i) {
    output.put(static_cast<char>(buffer.get_number(BYTE_SIZE, i * BYTE_SIZE)));
  }

  bit_sequence new_buffer;
  for (size_t i = buffer.size() / BYTE_SIZE * BYTE_SIZE; i < buffer.size();
       ++i) {
    new_buffer.append(buffer[i]);
  }

  buffer.swap(new_buffer);
}
size_t encoder::get_input_size() const {
  size_t result = 0;
  for (size_t val : counts) {
    result += val;
  }
  return result;
}
size_t encoder::get_output_size() const {
  size_t result = 0;
  size_t cur = header().size();

  for (size_t i = 0; i < CHARS_COUNT; ++i) {
    result += cur / BYTE_SIZE;
    cur %= BYTE_SIZE;
    cur += counts[i] * codes[i].size();
  }
  result += (cur + BYTE_SIZE - 1) / BYTE_SIZE;
  return result;
}
} // namespace huffman

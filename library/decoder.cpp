#include "decoder.h"
#include <cassert>
#include <memory>
#include <stdexcept>

namespace huffman {

size_t decoder::get_header_size(uint8_t first_byte) {
  // if first byte is 0 than it is only byte in encoded file,
  // and decoded file will be empty
  if (first_byte == 0) {
    return 1;
  }
  // first byte contains number of used characters (number of leafs in tree - 1)
  // each node takes 9 bits so (2 * first_byte + 1) * 9 is size of traversal
  // + 3 next bits contains padding, we take that,
  // divide by 8 with ceiling ((x + 7) / 8) and also this first byte is a part
  // of header, so we add 1 to the result
  return ((2 * first_byte + 1) * LOG_MAX_NODE_NUMBER + 3 + BYTE_SIZE - 1) /
             BYTE_SIZE +
         1;
}
void decoder::read_header(std::vector<uint8_t> const& header) {
  // decoder must be empty
  assert(tree_ == nullptr);
  assert(buffer.size() == 0);
  bit_sequence seq;
  for (uint8_t byte : header) {
    seq.append(byte, BYTE_SIZE);
  }
  size_t traversal_size = header[0] * 2 + 1;
  std::vector<uint16_t> traversal;
  traversal.reserve(traversal_size);
  for (size_t idx = BYTE_SIZE;
       idx < BYTE_SIZE + traversal_size * LOG_MAX_NODE_NUMBER;
       idx += LOG_MAX_NODE_NUMBER) {
    traversal.push_back(seq.get_number(LOG_MAX_NODE_NUMBER, idx));
  }

  std::array<bool, CHARS_COUNT> chars{false};
  for (uint16_t node : traversal) {
    if (node < CHARS_COUNT) {
      if (chars[node] && traversal.size() != 3) {
        throw std::runtime_error("Incorrect input");
      }
      chars[node] = true;
    } else if (node != CHARS_COUNT) {
      throw std::runtime_error("Incorrect input");
    }
  }

  tree_ = std::make_unique<tree>(traversal);
  end_padding =
      seq.get_number(3, BYTE_SIZE + LOG_MAX_NODE_NUMBER * traversal_size);
  for (size_t i = 3 + BYTE_SIZE + LOG_MAX_NODE_NUMBER * traversal_size;
       i < seq.size(); ++i) {
    buffer.append(seq[i]);
  }
}
std::pair<size_t, size_t> decoder::decode(std::istream& input, std::ostream& output) {
  uint8_t first_byte = input.get();
  size_t rest_header_size = get_header_size(first_byte) - 1;
  std::vector<uint8_t> header(rest_header_size + 1);
  header[0] = first_byte;
  for (size_t i = 0; i < rest_header_size; ++i) {
    header[i + 1] = input.get();
  }
  read_header(header);
  size_t input_size = rest_header_size + 1;
  size_t output_size = 0;
  while (true) {
    uint8_t ch = input.get();
    if (input.fail()) {
      break;
    }
    ++input_size;
    buffer.append(ch, BYTE_SIZE);
    if (buffer.size() > MAX_BUFFER_SIZE) {
      output_size += dump_buffer(output);
    }
  }
  output_size += dump_buffer(output);
  if (buffer.size() != end_padding) {
    throw std::runtime_error("Incorrect input");
  }
  return {input_size, output_size};
}

size_t decoder::dump_buffer(std::ostream& output) {
  auto [idx, write_size] = tree_->dump(buffer, buffer.size() - end_padding, output);

  bit_sequence new_buffer;
  for (size_t i = idx; i < buffer.size(); ++i) {
    new_buffer.append(buffer[i]);
  }
  buffer.swap(new_buffer);
  return write_size;
}
} // namespace huffman
